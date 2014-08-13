/*
  pool.c String pool routines.
  Firebird Bulletin Board System (The TIP Project version)
  BBSLIB library version 2.0
  Copyright (C) 1998 - 2000, Sh Yunchen
        - rock@bbs.isu.edu.tw, shirock@mail.educities.edu.tw

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/*
pool.c: 包含: pool, parray

!! NOT SAFE FOR THREAD.

作者: rock 遊手好閒的石頭成
日期: 87.9.25

實作目的:

提供一個動態長度字串的功能，在使用時，自動視情形調整字串長度，
並可以在最少的改變下，繼續使用原本的標準函數。
在 Apache 中，有提供一個功能更強的 pool 模組，但是沒辦法直接
抽離出來給 BBS 用，所以我寫了一個使用起來比較簡單的 pool 。

實作說明:

content 指向 pool 的真正內容(通常是一個字串)。
length 表示 content 目前的字串長度，即已使用的容量，但不包含
字串終結字元 '\0' 所佔的容量。
size 表示目前 content 可用的大小， length 的最大值等於 size-1 。

每次以 POOLBLKSIZ 為單位來配置一個記憶體區域，以 content 指
向該區域， size 表示該區域的大小( n * POOLBLKSIZ ， POOLBLKSIZ
的整數倍數)。
將字串內容儲存在該區域中，但是因為字串不會一次剛好用完整個含量，
故以 length 表示目前已使用的容量。
當要儲存新的字串內容時，會自動判斷目前配置的容量，是否仍足夠儲
存新的內容，如果不夠，會以 POOLBLKSIZ 為單位，再配置一塊足夠容
納所有內容(原有及新增)的記憶體區域，將所有內容給儲存到該新的區
域中後，才釋放掉舊的記憶體區域， content 會指向新的區域， size
會指派新的容量大小。

針對會影嚮儲存區域的常有動作，提供了:
pstrcpy(), pstrncpy(), pstrcat(), pstrncat()

四個對應函數，此四個函數的結果都跟原來的一模一樣。
至於不會影嚮儲存區域的動作，可以利用:
pcontent() 取得字串內容或 pcharat() 取得字元內容
再呼叫標準函數內容如 strcmp() 使用。

parray 為利用 pool 實作的動態陣列。
	
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
/*STRUCT: string_i.h	Pool need to include this file*/
#include "string_i.h"
#include "pool.h"
/*STRUCT END*/
#include "bbsdefs.h"

/*STRUCT: pool_t
不要將 struct pool 的變數定義為 static 或 全域性的變數!!
因為我寫的這個功能的相關函數，沒有針對 multi-thread 程式做
規劃或測試。
除非你自已用系統呼叫來進行變數的同步使用。
例如 PThread 的 pthread_mutex_* 系列。
DONT DEFINE struct pool variable as static or global!!
Struct pool and its functions are not safe for multi-thread program.
*/
#if 0
struct pool
{
	string_i striobj;	// 具有 string_i 所承諾的行為的物件;
	char *content;        // pool 儲存區，Zero-end string;
	size_t length;        // Length of string, less terminating null character;
	size_t size;          // The (current) available size of content;
	size_t increment;     // 每次容量增加量，預設為 POOLBLKSIZ 
	enum {
		POOL_MALLOC,
		POOL_AUTO
	} type;
};
typedef struct pool pool_t;

// _SC_PAGESIZE not part of POSIX.1
#if defined( _SC_PAGESIZE )
#define POOLBLKSIZ  sysconf(_SC_PAGESIZE)   // per block size
#else
#define POOLBLKSIZ 1024
#endif
/*
設為 1024 是因為大多數的 32bit OS 是以 1024 bytes 為 1 page 。
1 page 是 OS 在配置記憶體時的最小單位。
PS. 我目前已確知的 OS ，其 page 大小
	OS/2 : 4 KB = 1 page.
	Linux: 4 KB = 1 page.
	DOS  : 16 bytes = 1 para.
	DOS 是以 para 為配置單位，但其意義跟 page 很像。
	其他的 i386 OS ，也都是以 1 KB 到 4 KB 為 1 page ，大於或小於
	都太浪費了。
*/
#endif


/*NAME: plength()
取得 pool 的字串長度。
你也可以直接讀取 pool.length ，但是基於資料封裝的理由，
不建議這麼做。
如果是用 C++ ，可以用 online 。
	
RC: pool 現已使用的容量長度
See also: pool_t, pconstruct2()
*/
#if 0
// C_INLINE
size_t plength(const pool_t *p) {
	return p->length;
}
#endif 


/*NAME: pcharat()
讀取 pool 第 n 個索引的字元值。
n 從 0 開始。
可以直接使用 pool.content[n] ，但同樣基於資料封裝的理由，
不建議這麼做。
	
RC: 字元
See also: plength(), pcontent()
*/
#if 0
// C_INLINE
char pcharat(const pool_t *p,size_t n) {
	if( n >= plength(p) )
		return '\0';
	return p->content[n];
}
#endif

/*NAME: pcharcat()
若參數 ch 等於字元 '\0' ，將不會被加入。
	
See also: pool_t, pconstruct(), pcontent(), plength()
*/
char* pcharcat(pool_t *pdest, char ch) {
	if( ch == '\0' )
		return pdest->content;
	if( pdest->length +1 >= pdest->size ) {
		size_t newsize=pdest->size + pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else    
		char *t1, *t2;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);
		strcpy(t1,pdest->content);
		t1[ pdest->length ] = ch;
		t2=pdest->content;
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif
		pdest->content[ pdest->length ] = ch;
	pdest->length++;
	return pdest->content;
}


/*NAME: pcontent()
取得 pool 儲存內容的指標。
建議多利用這個函數來取得儲存內容，不要直接用 pool.content 。

RC: pool 儲存區的指標
See also: pool_t, pconstruct2(), plength()
*/
#if 0
// C_INLINE
char *pcontent(const pool_t *p) {
	return p->content;
}
#endif


/*NAME: pdownsize()
縮減 pool 容量至最小值。
pool 在使用時，只會自動增加容量，不會自動縮減容量，若覺得
有些動作，在操作完畢後，所增加的容量不會再用到的話，可以
使用此函數，強迫縮減容量。
因為不一定會縮減成功，也可能縮減容量後，馬上又碰到需要更大
容量的操作動作，所以沒必要儘量不要使用。

RC: 傳回縮減後的 pool content 指標
See also: pool_t, pupsize(), pcontent()
*/
char* pdownsize(pool_t *p) {
	size_t newsize;
	newsize=((p->length / POOLBLKSIZ)+1)*POOLBLKSIZ;
	// 因為是要縮減大小，而不是要增加，所以不理會 pool.increment 的值。
	if( newsize < p->size ) {
#ifdef realloc
		p->content=realloc(p->content, newsize);
		p->size=newsize;
#else
		char *t1,*t2 = NULL;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);
		if( p->content != NULL ) {
			memcpy(t1,p->content,p->length);
			t2=p->content;
		}
		p->size=newsize;
		p->content=t1;
		free(t2);
#endif
	}
	return p->content;
}


/*NAME: pupsize()
增加 pool 容量大小。
一般情形下， pool 會自動增加容量大小，但若覺得有些連續操作，可能
會不斷的需要增加容量，為了避免頻繁的配置動作折損效率，可以先呼叫
此函數，強迫 pool 先配置到一個大於或等於 upsize 的容量，減少再配
置的機會。
pool 的容量增加，是以 pool.increment 的整數倍增加的，如果你設定的
upsize 不是 pool.increment 的整數倍，此函數會自動調整到大於 upsize
的整數倍值。

RC: 傳回擴增後的 pool content 指標。
See also: pool_t, pdownsize(), pcontent()
*/
char* pupsize(pool_t *p,size_t upsize) {
	int newsize;
	newsize=((upsize / p->increment)+1)* p->increment;
	if( newsize > p->size ) {
#ifdef realloc  
		p->content=realloc(p->content, newsize);
		p->size=newsize;
#else
		char *t1,*t2 = NULL;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);
		if( p->content != NULL ) {
			memcpy(t1,p->content,p->length);
			t2=p->content;
		}
		p->content=t1;
		p->size=newsize;
		free(t2);
#endif    
	}
	return p->content;
}


/*NAME: pstrcpy()
參考 strcpy()

See also: pool_t, pstrncpy(), pconstruct(), pcontent(), plength()
*/
char* pstrcpy(pool_t *pdest, const char*ssrc)
{
	size_t ssrclen;

	ssrclen=strlen(ssrc);
	if( ssrclen >= pdest->size ) {
		size_t newsize=((ssrclen / pdest->increment)+1)* pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else  
		char*t1,*t2;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);       // memset() 的動作其實可以省下來...
		strcpy(t1,ssrc);
		t2=pdest->content;          // swap 的次序有意義，不要亂調。
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif  
		strcpy(pdest->content,ssrc);
	pdest->length = ssrclen;
	return pdest->content;
}


/*NAME: pstrncpy()
參考 strncpy()

See also: pool_t, pstrcpy(), pconstruct(), pcontent(), plength()
*/
char* pstrncpy(pool_t *pdest, const char* ssrc, int maxlen)
{
	size_t ssrclen;
	ssrclen=strlen(ssrc);
	if( ssrclen > maxlen )
		ssrclen=maxlen;
	if( ssrclen >= pdest->size ) {
		size_t newsize=((ssrclen / pdest->increment)+1)* pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else    
		char*t1,*t2;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);
		strncpy(t1,ssrc,ssrclen);
		t2=pdest->content;
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif  
		strncpy(pdest->content,ssrc,ssrclen);
	if( ssrclen > pdest->length )
		pdest->length = ssrclen;
	return pdest->content;
}


/*NAME: pstrcat()
參考 strcat()

See also: pool_t, pstrncat(), pconstruct(), pcontent(), plength()
*/
char* pstrcat(pool_t *pdest, const char* ssrc)
{
	size_t ssrclen;
	if( ISEMPTYSTR(ssrc) )
		return pdest->content;
	ssrclen=strlen(ssrc);
	if( ssrclen + pdest->length >= pdest->size ) {
		size_t newsize=(((ssrclen+pdest->length) / pdest->increment)+1)* pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else    
		char *t1, *t2;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);
		if(pdest->content)
			strcpy(t1,pdest->content);
		strcat(t1,ssrc);
		t2=pdest->content;
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif  
		strcat(pdest->content,ssrc);
	pdest->length = strlen(pdest->content);
	//pdest->length=pdest->length+ssrclen;
	return pdest->content;
}


/*NAME: pstrncat()
參考 strncat()

See also: pool_t, pstrcat(), pconstruct(), pcontent(), plength()
*/
char* pstrncat(pool_t *pdest, const char* ssrc, int maxlen)
{
	size_t ssrclen;
	if( ISEMPTYSTR(ssrc) )
		return pdest->content;
	ssrclen=strlen(ssrc);
	if( ssrclen > maxlen )
		ssrclen=maxlen;
	if( ssrclen + pdest->length >= pdest->size ) {
		size_t newsize=(((ssrclen+pdest->length) / pdest->increment)+1)* pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else  
		char *t1, *t2;
		t1=(char*)malloc(newsize);
		memset(t1,0,newsize);
		if(pdest->content)
			strcpy(t1,pdest->content);
		strncat(t1,ssrc,ssrclen);
		t2=pdest->content;
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif
		strncat(pdest->content,ssrc,ssrclen);
	pdest->length = strlen(pdest->content);
	//pdest->length=pdest->length+ssrclen;
	return pdest->content;
}


/*NAME: pmemcpy()
參考 memcpy() & memmove
With memcpy, if ssrc and pdest overlap, the behavior is undefined.
With memmove, even when the ssrc and pdest blocks overlap, bytes
in the ovarlapping locations are copied correctly.
Because of that, I implement pmemcpy by memmove, not memcpy.
Be careful! The pmemcpy and pmemcat are not care about the
terminating null character, so the pool.length might equal pool.size.

See also: pool_t, pmemcat(), pconstruct()
*/
void* pmemcpy(pool_t *pdest, const void* ssrc, size_t nbytes)
{
	if( nbytes > pdest->size ) {
		size_t newsize=((nbytes / pdest->increment)+1)* pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else  
		void*t1,*t2;
		t1=(void*)malloc(newsize);
		memset(t1,0,newsize);
		memmove(t1,ssrc,nbytes);
		t2=pdest->content;
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif  
		memmove(pdest->content,ssrc,nbytes);
	if( nbytes > pdest->length )
		pdest->length = nbytes;
	return pdest->content;
}


/*NAME: pmemcat()
參考 pmemcpy()

See also: pool_t, pmemcpy(), pconstruct()
*/
void* pmemcat(pool_t *pdest, const void* ssrc, size_t nbytes)
{
	if( nbytes + pdest->length > pdest->size ) {
		size_t newsize=(((nbytes+pdest->length) / pdest->increment)+1)* pdest->increment;
#ifdef realloc
		pdest->content=realloc(pdest->content, newsize);
		pdest->size=newsize;
	}
#else  
		void *t1, *t2;
		t1=(void*)malloc(newsize);
		memset(t1,0,newsize);
		if(pdest->content)
			memmove(t1,pdest->content,pdest->length);
		memmove((void*)((size_t)t1+pdest->length),ssrc,nbytes);
		t2=pdest->content;
		pdest->content=t1;
		pdest->size=newsize;
		free(t2);
	} else
#endif  
		memmove(pdest->content + pdest->length,ssrc,nbytes);
	pdest->length=pdest->length+nbytes;
	return pdest->content;
}


/*NAME: pclear()
clear all content of pool.
content set to zero.
length set to zero.

RC: void
See also: pool_t
*/
void pclear( pool_t*p )
{
	if( p == NULL || p->content == NULL)
		return;
	memset( p->content, 0, p->length );
	p->length = 0;
}


/*NAME: pconstruct2()
建構一個 pool 變數。
如果參數 p 傳 NULL 的話，會自動配置一個 pool 變數，並初始化
內容，設 pool.type 為 POOL_MALLOC 。
如果參數 p 不為 NULL 的話，表示此為自動變數(動態的區域變數)，
只做初始化動作，並設 pool.type 為 POOL_AUTO 。
pool.type 會影嚮 destroy 的動作。
initsiz 是 pool 儲存區的初始大小。
incrsiz 則是每次容量的增加量。

pconstruct2() 較少使用，pconstruct() 的使用率較高。

RC: 傳回一個 pool_t 指標。  
See also: pool_t, pconstruct(), pconstruct(), pdestroy(),
	plength(), pcontent(), pstrcpy(), pstrncpy(),
	pstrcat(), pstrncat(), pmemcpy(), pmemcat(),
	pdownsize(), pupsize(), pcharat(), pcharcat()
*/
pool_t *pconstruct2(pool_t *p, int initsiz, int incrsiz)
{
	pool_t *tmp;
	if( p==NULL )
		tmp=(pool_t *) malloc(sizeof(pool_t));
	else
		tmp=p;
	memset(tmp,0,sizeof(pool_t));
	// tmp->content=NULL;
	// tmp->length=0;
	// tmp->size=0;
	// all of the elements set to clear (null).
	tmp->striobj.striobj = tmp;
	tmp->striobj.length  = (void*)plength;
	tmp->striobj.text    = (void*)pcontent;
	tmp->striobj.strcpy  = (void*)pstrcpy;
	tmp->striobj.strncpy = (void*)pstrncpy;
	tmp->striobj.strcat  = (void*)pstrcat;
	tmp->striobj.strncat = (void*)pstrncat;
	
	if( initsiz > 0 ) {
		tmp->content = (char*) malloc( ((initsiz / POOLBLKSIZ)+1)*POOLBLKSIZ );
		tmp->size = initsiz;
	}
	if( incrsiz > POOLBLKSIZ )  // incrsiz > 0 && incrsiz > POOLBLKSIZ
		tmp->increment = ((incrsiz / POOLBLKSIZ)+1)*POOLBLKSIZ;
	else
		tmp->increment = POOLBLKSIZ;
	if( p==NULL )
		tmp->type = POOL_MALLOC;
	else
		tmp->type = POOL_AUTO;
	return tmp;
}


/*NAME: pconstruct1()
建構一個 pool 變數。
除了不指定每次容量增加量(increment)外，餘同 pconstruct2()。

RC: 傳回一個 pool_t 指標
See also: pconstruct2()
*/
#if 0
// C_INLINE
pool_t *pconstruct1(pool_t *p, int initsiz)
{
	return pconstruct2( p, initsiz, POOLBLKSIZ );
}
#endif


/*NAME: pconstruct()
建構一個 pool 變數。
不指定每次容量增加量及初始容量。

RC: 傳回一個 pool_t 指標
See also: pconstruct2()
*/
#if 0
// C_INLINE
pool_t *pconstruct(pool_t *p)
{
	return pconstruct2( p, 0, 0 );
}
#endif


/*NAME: pdestroy()
解構一個 pool 變數。
會自動將 content 指向的記憶體區域釋放掉，
接著，若 pool.type = POOL_MALLOC ，則將 pool 整個釋放掉，
若 pool.type = POOL_AUTO ，則將其資料全部重設(清除為0)。
RC: void
See also: pconstruct2()
*/
void pdestroy(pool_t *p)
{
	if( p != NULL )
	{
		if( p->content != NULL )
			free(p->content);
		if( p->type == POOL_MALLOC )
			free(p);
		else
			memset(p,0,sizeof(pool_t));
	}
}

#if 0
/*STRUCT: parray
parray: 利用 pool 實作的動態陣列。
*/
struct parray
{
	pool_t p;
	size_t sizeOfElement;         // 陣列元素的單一大小
	int    elementCount;          // 陣列元素的數量
	enum {
		PARRAY_MALLOC,
		PARRAY_AUTO
	} type;
};
typedef struct parray parray_t;
/*STRUCT END*/
#endif

/*NAME: parrayconstruct()
建構一個 parray 變數。
initsiz 設定陣列初始容量。
incrsiz 設定陣列每次增加容量。
persiz 設定單一陣列元素的大小，此值不可為 0 。

RC: 傳回一個 parray 的指標
See also: parray_t, pool_t, pconstruct1(), parraydestroy(),
	paElementCount(), paContent(), paAddElement(),
	paGetElement(), paSetElement(), parraycpy(), parraycat()
*/
parray_t* parrayconstruct(parray_t* pa, size_t initsiz,
				size_t incrsiz, size_t persiz)
{
	parray_t* tmp;
	if( persiz == 0 )
		return NULL;

	if( pa == NULL )
		tmp = (parray_t*) malloc(sizeof(parray_t));
	else
		tmp = pa;

	pconstruct2( (pool_t*)tmp, initsiz, incrsiz );
	// 呼叫上層建構函數 pconstruct2()
	/*
	if( initsiz > 0 )
		pupsize( (pool_t*)tmp, initsiz );

	if( incrsiz > 0 )
		tmp->capacityIncrement = incrsiz;
	else
		tmp->capacityIncrement = POOLBLKSIZ;
	*/
	tmp->sizeOfElement = persiz;

	tmp->elementCount = 0;

	if( pa==NULL )
		tmp->type = PARRAY_MALLOC;
	else
		tmp->type = PARRAY_AUTO;

	return tmp;
}


/*NAME: parrayconstruct1()
建構一個 parray 變數。

RC: 傳回一個 parray 的指標
See also: parray_t, parrayconstruct()
*/  
#if 0
// C_INLINE
parray_t* parrayconstruct1(parray_t* pa, size_t persiz)
{
	return parrayconstruct( pa, 0, 0, persiz);
}
#endif


/*NAME: parraydestroy()
解構一個 parray 變數。

RC: void
See also: parray_t, parrayconstruct()
*/  
void parraydestroy( parray_t* pa )
{
	if( pa == NULL )
		return;
	pdestroy( (pool_t*) pa);
	if( pa->type == PARRAY_MALLOC )
		free( pa );
	else
		memset(pa,0,sizeof(parray_t));
}


/*NAME: paClear()
清除一個 parray 變數中的元素內容。
elementCount = 0;
all element set to zero;

RC: void
See also: parray_t, parrayconstruct()
*/  
void paClear( parray_t* pa )
{
	if( pa == NULL )
		return;
	memset(pcontent(&pa->p), 0, plength(&pa->p));
	pa->p.length = 0;
	pa->elementCount = 0;
}


/*NAME: paElementCount()
傳回陣列中現有的元素個數。
	
RC: 元素個數
See also: parray_t, parrayconstruct(), parraydestroy()
*/
#if 0
// C_INLINE
int paElementCount(parray_t* pa)
{
	return pa->elementCount;
}
#endif

/*NAME: paContent()
傳回陣列內容的指標。

RC: 傳回陣列內容的指標
See also: parray_t, pconstruct(), paElementCount()
*/
#if 0
// C_INLINE
void* paContent(const parray_t* pa)
{
	return (void*) pa->p.content;
}
#endif


/*NAME: paAddElement()
增加一個陣列元素到陣列中，並傳回陣列元素被加入的指標。
必須注意的是，因為 parray 是一個動態擴充大小的陣列，其陣列內容(content)
的指標會因為陣列的擴充而改變，所以陣列元素的指標也會改變。

RC: 傳回被加入的陣列元素所在指標
See also: parray_t, paElementCount(), paContent()
*/
#if 0
// C_INLINE
void* paAddElement(parray_t* pa, const void*elem)
{
	void* t;
	pmemcat( (pool_t*) pa, elem, pa->sizeOfElement);
	t = &(pa->p.content[ pa->elementCount * pa->sizeOfElement ]);
	pa->elementCount ++;
	return t;
}
#endif


/*NAME: paGetElement()
取得一個指定陣列元素所在的指標。
如果無此元素，則傳回 NULL 。

RC: 陣列元素的指標
See also: parray_t, paAddElement()
*/  
#if 0
// C_INLINE
void* paGetElement(const parray_t* pa, int n, void*elem)
{
	void* t;
	if( n >= pa->elementCount )
		return NULL;
	t = &(pa->p.content[ n * pa->sizeOfElement ]);
	if( elem != NULL )
		memcpy(elem,t,pa->sizeOfElement);
	return t;
}
#endif


/*NAME: paSetElement()
設定指定的陣列元素的內容。
如果指定的陣列元素不存在，則傳回 NULL ，否則傳回指定元素的目前指標。

RC: 傳回指定元素的指標
See also: parray_t, paGetElement()
*/
#if 0
// C_INLINE
void* paSetElement(const parray_t* pa, int n, const void*elem)
{
	void* t;
	if( n >= pa->elementCount )
		return NULL;
	t = &(pa->p.content[ n * pa->sizeOfElement ]);
	memcpy(t,elem,pa->sizeOfElement);
	return t;
}
#endif


/*NAME: parraycpy()
設定 ntiems 筆資料(elmes)，到陣列中。
從陣列開頭處開始複製。

RC: 傳回陣列內容
See also: parray_t, parraycat()
*/
void* parraycpy(parray_t* pa, int nitems, void*elems)
{
	void* t;
	if( nitems < 0)
		return NULL;
	t = pmemcpy( (pool_t*) pa, elems, nitems * pa->sizeOfElement );
	if( nitems > pa->elementCount )
		pa->elementCount = nitems;
	return t;
}


/*NAME: parraycat()
增加 ntiems 筆資料(elmes)到陣列元素中。
RC: 傳回陣列內容
See also: parray_t, parraycpy()
*/  
void* parraycat(parray_t* pa, int nitems, void*elems)
{
	void* t;
	if( nitems < 0 )
		return NULL;
	t = pmemcat( (pool_t*) pa, elems, nitems * pa->sizeOfElement );
	pa->elementCount += nitems;
	return t;
}

/* hash: associative array */
/* 89年12月18日 by rock (遊手好閒的石頭成) */
#if 0
struct phash {
	pool_t hashpool;
	int keyCount;
	enum {
		POOLHASH_MALLOC,
		POOLHASH_AUTO
	} type;
};
typedef struct phash phash_t;
#endif

typedef struct {
	off_t key;
	off_t nextkey;
	paElement data;
	size_t blksize;
} phash_key;

phash_t* phashconstruct(phash_t* ph) {
	phash_t* tmp;
	phash_key empty;

	if( ph == NULL ) {
		tmp = (phash_t*) malloc(sizeof(phash_t));
		tmp->type = POOLHASH_MALLOC;
	}
	else {
		tmp = ph;
		tmp->type = POOLHASH_AUTO;
	}
	pconstruct( &tmp->hashpool );
	tmp->keyCount = 0;
	memset(&empty, 0, sizeof(empty));
	pmemcat(&tmp->hashpool, &empty, sizeof(empty));
	return tmp;
}

void phashdestroy( phash_t* ph ) {
	if( ph == NULL )
		return;
	pdestroy( &ph->hashpool );
	if( ph->type == POOLHASH_MALLOC )
		free( ph );
	else
		memset(ph, 0, sizeof(phash_t));
}

#if 0
// C_INLINE
int phashCount( phash_t* ph) {
	return ph->keyCount;
}
#endif

void phashClear( phash_t* ph ) {
	phash_key empty;

	if( ph == NULL )
		return;
	pclear( &ph->hashpool );
	ph->keyCount = 0;
	memset(&empty, 0, sizeof(empty));
	pmemcat(&ph->hashpool, &empty, sizeof(empty));
}

C_INLINE phash_key* _phash_seek( phash_t* ph, const char* key) {
	phash_key* ckey;
	char *base;

	base = pcontent(&ph->hashpool);
	ckey = (phash_key*) base;
	while(ci_strcmp(key, base + ckey->key )) {
		ckey = (phash_key*)(base + ckey->nextkey);
		if( !ckey->key && !ckey->nextkey)
			break;
	}
	return ckey;
}

void* phashPut( phash_t* ph, const char*key, const paElement*data ) {
	phash_key *ckey, empty;
	void *base;

	ckey = _phash_seek(ph, key);
	base = pcontent(&ph->hashpool);

	/* 新的 key/data */
	if( !ckey->key ) {
		ckey->key = plength(&ph->hashpool);
		pmemcat(&ph->hashpool, key, strlen(key)+1);
		ph->keyCount ++;
		ckey->data.content = (void*) plength(&ph->hashpool);
		pmemcat(&ph->hashpool, data->content, data->size);
		ckey->blksize = ckey->data.size = data->size;

		if( !ckey->nextkey) { /*加入新的空鍵 - 表示結束 */
			memset(&empty, 0, sizeof(empty));
			ckey->nextkey = plength(&ph->hashpool);
			pmemcat(&ph->hashpool, &empty, sizeof(empty));
		}
	}
	/* key 已存在，改變 data */
	else if( ckey->blksize >= data->size ) { /* replace data */

		memcpy(base + (off_t)ckey->data.content, data->content, data->size);
		ckey->data.size = data->size;
	}
	else { /* append data */
		ckey->data.content = (void*)plength(&ph->hashpool);
		pmemcat(&ph->hashpool, data->content, data->size);
		ckey->blksize = ckey->data.size = data->size;
	}
	return base + (off_t)ckey->data.content;
}

paElement phashGet( phash_t* ph, const char*key) {
	phash_key *ckey;
	paElement elem;

	ckey = _phash_seek(ph, key);
	if(ckey->key) {
		elem.content = pcontent(&ph->hashpool) + (off_t)ckey->data.content;
		elem.size = ckey->data.size;
	}
	else
		memset(&elem, 0, sizeof(elem));
	return elem;
}

/*
  return the element in phash by index.
  0 <= index value < key count -1
*/
paElement phashGetN( phash_t* ph, int idx, char *key) {
	phash_key* ckey;
	paElement elem;
	char *base;
	int i;

	if(idx >= ph->keyCount || idx < 0) {
		elem.content = NULL;
		elem.size = 0;
		return elem;
	}

	base = pcontent(&ph->hashpool);
	ckey = (phash_key*)base;
	i = 0;
	while(i < idx) {
		if(ckey->key)
			i++;
		ckey=(phash_key*)(base + ckey->nextkey);
	}
	if(key)
		strcpy(key, base + ckey->key);
	elem.content = base + (off_t)ckey->data.content;
	elem.size = ckey->data.size;
	return elem;
}

int phashApply( phash_t* ph, int (*func)(const char*key, paElement*, void*), void*farg) {
	phash_key* ckey;
	paElement elem;
	char *base;
	int rc;
	if(!func) {
		errno = EINVAL;
		return -1;
	}
	base = pcontent(&ph->hashpool);
	ckey = (phash_key*)base;
	while(1) {
		if(!ckey->key && !ckey->nextkey)
			break;
		elem.content = base + (off_t)ckey->data.content;
		elem.size = ckey->data.size;
		if( (rc=(*func)(base + ckey->key, &elem, farg)) )
			break;
		ckey=(phash_key*)(base + ckey->nextkey);
	}
	return rc;
}

/*int phashRemove( phash_t* ph, const char*key) {
}*/


#if 0
/*
以下為測試程式。
測試的結果跟使用 strcpy() 等函數的結果一樣，但是
容量不足時會自動增加。

我在 DOS 下測試時， POOLBLKSIZ 設為 32 ，仍然沒有出問題。
*/

typedef struct _testnode {
	char name[28];
	int value;
	char empty[480];
} testnode;

void docat(string_i* striobj,const char*s)
{
	(*striobj->strcat)(striobj->striobj,s);
}

void main(int argc, char**argv)
{
	pool_t p;
	int i;
	
	pconstruct(&p);

	printf("Go.\n");  
	pstrcpy(&p,"GO: ");
	for(i=1; i< argc; i++) {
		docat(&p.striobj, argv[i]);
	}
	printf("%s\n",pcontent(&p));
	pdestroy(&p);

#if 0
	parray_t pa;
	testnode n, *np;
	int i;

	printf("Go.\n");  
	parrayconstruct(&pa, 8000, 2000, sizeof(testnode));
	printf("element count: %d\n",paElementCount(&pa));

	memset(&n,0,sizeof(testnode));  
	for( i=0; i < 100; i++ ) {
		n.name[0] = 'a';
		n.value = i;
		np = paAddElement(&pa, &n);
		if( np == &n )
			printf("np equal n\n");
		printf("element count: %d\n",paElementCount(&pa));
	}
	printf("Total: %d, size: %d, length: %d\n", pa.elementCount, pa.p.size, pa.p.length);

	getchar();
	
	for( i=0; i < paElementCount(&pa); i++) {
		np = paGetElement(&pa, i, NULL);
		printf("%d. %s: %d\n",i, np->name, np->value);
	}

	getchar();  
	for( i=50; i < (paElementCount(&pa)-20); i++) {
		n.name[1] = (char)i;
		n.value += 1000;
		np = paSetElement(&pa, i, &n);
		printf("%d. %s: %d\n",i, np->name, np->value);
	}
		
	for( i=0; i < paElementCount(&pa); i++) {
		np = paGetElement(&pa, i, NULL);
		printf("%d. %s: %d\n",i, np->name, np->value);
	}
	getchar();
	
	parraydestroy(&pa);
#endif
}
#endif
#if 0
int main() {
	phash_t hash;
	paElement elem;
	char key[80], data[80], *p;

	phashconstruct(&hash);

	printf("Go.\n");  
	while(1) {
		fgets(key, sizeof(key), stdin);
		p=(char*)strchr(key, '\n'); *p='\0';
		if(*key == '\0') 	break;
		fgets(data, sizeof(data), stdin);
		p=(char*)strchr(data, '\n'); *p='\0';
		elem.content = data;
		elem.size = strlen(data)+1;

		printf("Put - Key: %s, Data:%s\n", key, data);
		phashPut(&hash, key, &elem);

		printf("Total: %d\n", phashCount(&hash));
	}
	while(1) {
		fgets(key, sizeof(key), stdin);
		p=(char*)strchr(key, '\n'); *p='\0';
		if(*key == '\0') 	break;

		elem = phashGet(&hash, key);
		if(elem.content)
			printf("%s: %s\n", key, elem.content);
		else
			printf("Not found\n");
	}
	phashdestroy(&hash);
	printf("End\n");
	return 1;
}
#endif
