/*
    pool.h Header file of string pool routines.
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
#if !defined( __POOL_H )
#define __POOL_H
#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <unistd.h>
/*STRUCT: string_i.h	Pool need to include this file*/
#include "string_i.h"
/*STRUCT END*/

typedef struct {
	void* content;
	size_t size;
} paElement;

/*STRUCT: pool_t
  不要將 struct pool 的變數定義為 static 或 全域性的變數!!
  因為我寫的這個功能的相關函數，沒有針對 multi-thread 程式做
  規劃或測試。
  除非你自已用系統呼叫來進行變數的同步使用。
  例如 PThread 的 pthread_mutex_* 系列。
  DONT DEFINE struct pool variable as static or global!!
  Struct pool and its functions are not safe for multi-thread program.
*/
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
  Linux: 1 KB = 1 page.
  DOS  : 16 bytes = 1 para.
  DOS 是以 para 為配置單位，但其意義跟 page 很像。
  其他的 i386 OS ，也都是以 1 KB 到 4 KB 為 1 page ，大於或小於
  都太浪費了。
*/
/*STRUCT END*/

/*NAME: plength()
  取得 pool 的字串長度。
  你也可以直接讀取 pool.length ，但是基於資料封裝的理由，
  不建議這麼做。
  如果是用 C++ ，可以用 online 。
  
  RC: pool 現已使用的容量長度
  See also: pool_t, pconstruct2()
*/
//#define plength(p)  (size_t)p->length
C_INLINE size_t plength(const pool_t*p) {
	return p->length;
}

/*NAME: pcharat()
  讀取 pool 第 n 個索引的字元值。
  n 從 0 開始。
  可以直接使用 pool.content[n] ，但同樣基於資料封裝的理由，
  不建議這麼做。
  
  RC: 字元
  See also: plength(), pcontent()
*/
//#define pchatat(p,n)  (n >= p->length ? '\0' : p->content[(size_t)n])
C_INLINE char pcharat(const pool_t *p,size_t n) {
	if( n >= plength(p) )
		return '\0';
	return p->content[n];
}


/*NAME: pcharcat()
  若參數 ch 等於字元 '\0' ，將不會被加入。
  
  See also: pool_t, pconstruct(), pcontent(), plength()
*/
char* pcharcat(pool_t *pdest, char ch);

/*NAME: pcontent()
  取得 pool 儲存內容的指標。
  建議多利用這個函數來取得儲存內容，不要直接用 pool.content 。
  
  RC: pool 儲存區的指標
  See also: pool_t, pconstruct2(), plength()
*/
//#define pcontent(p)  (char*)p->content
C_INLINE char *pcontent(const pool_t *p) {
	return p->content;
}

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
char* pdownsize(pool_t *p);

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
char* pupsize(pool_t *p,size_t upsize);

/*NAME: pstrcpy()
  參考 strcpy()
  
  See also: pool_t, pstrncpy(), pconstruct(), pcontent(), plength()
*/
char* pstrcpy(pool_t *pdest, const char*ssrc);

/*NAME: pstrncpy()
  參考 strncpy()
  
  See also: pool_t, pstrcpy(), pconstruct(), pcontent(), plength()
*/
char* pstrncpy(pool_t *pdest, const char* ssrc, int maxlen);

/*NAME: pstrcat()
  參考 strcat()
  
  See also: pool_t, pstrncat(), pconstruct(), pcontent(), plength()
*/
char* pstrcat(pool_t *pdest, const char* ssrc);

/*NAME: pstrncat()
  參考 strncat()
  
  See also: pool_t, pstrcat(), pconstruct(), pcontent(), plength()
*/
char* pstrncat(pool_t *pdest, const char* ssrc, int maxlen);

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
void* pmemcpy(pool_t *pdest, const void* ssrc, size_t nbytes);

/*NAME: pmemcat()
  參考 pmemcpy()
  
  See also: pool_t, pmemcpy(), pconstruct()
*/
void* pmemcat(pool_t *pdest, const void* ssrc, size_t nbytes);

/*NAME: pclear()
  clear all content of pool.
  content set to zero.
  length set to zero.
  
  RC: void
  See also: pool_t
*/
void pclear( pool_t*p );

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
pool_t *pconstruct2(pool_t *p, int initsiz, int incrsiz);

/*NAME: pconstruct1()
  建構一個 pool 變數。
  除了不指定每次容量增加量(increment)外，餘同 pconstruct2()。
  
  RC: 傳回一個 pool_t 指標
  See also: pconstruct2()
*/
C_INLINE pool_t *pconstruct1(pool_t *p, int initsiz)
{
	return pconstruct2( p, initsiz, POOLBLKSIZ );
}

/*NAME: pconstruct()
  建構一個 pool 變數。
  不指定每次容量增加量及初始容量。
  
  RC: 傳回一個 pool_t 指標
  See also: pconstruct2()
*/
C_INLINE pool_t *pconstruct(pool_t *p)
{
	return pconstruct2( p, 0, 0 );
}

/*NAME: pdestroy()
  解構一個 pool 變數。
  會自動將 content 指向的記憶體區域釋放掉，
  接著，若 pool.type = POOL_MALLOC ，則將 pool 整個釋放掉，
  若 pool.type = POOL_AUTO ，則將其資料全部重設(清除為0)。
  RC: void
  See also: pconstruct2()
*/
void pdestroy(pool_t *p);

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
                          size_t incrsiz, size_t persiz);

/*NAME: parrayconstruct1()
  建構一個 parray 變數。
  
  RC: 傳回一個 parray 的指標
  See also: parray_t, parrayconstruct()
*/  
C_INLINE parray_t* parrayconstruct1(parray_t* pa, size_t persiz)
{
  return parrayconstruct( pa, 0, 0, persiz);
}


/*NAME: parraydestroy()
  解構一個 parray 變數。
  
  RC: void
  See also: parray_t, parrayconstruct()
*/  
void parraydestroy( parray_t* pa );

/*NAME: paClear()
  清除一個 parray 變數中的元素內容。
  elementCount = 0;
  all element set to zero;
  
  RC: void
  See also: parray_t, parrayconstruct()
*/  
void paClear( parray_t* pa );

/*NAME: paElementCount()
  傳回陣列中現有的元素個數。
  
  RC: 元素個數
  See also: parray_t, parrayconstruct(), parraydestroy()
*/
C_INLINE int paElementCount(parray_t* pa)
{
	return pa->elementCount;
}

/*NAME: paContent()
  傳回陣列內容的指標。
  
  RC: 傳回陣列內容的指標
  See also: parray_t, pconstruct(), paElementCount()
*/
C_INLINE void* paContent(const parray_t* pa)
{
	return (void*) pa->p.content;
}


/*NAME: paAddElement()
  增加一個陣列元素到陣列中，並傳回陣列元素被加入的指標。
  必須注意的是，因為 parray 是一個動態擴充大小的陣列，其陣列內容(content)
  的指標會因為陣列的擴充而改變，所以陣列元素的指標也會改變。
  
  RC: 傳回被加入的陣列元素所在指標
  See also: parray_t, paElementCount(), paContent()
*/
C_INLINE void* paAddElement(parray_t* pa, const void*elem)
{
	void* t;
	pmemcat( (pool_t*) pa, elem, pa->sizeOfElement);
	t = &(pa->p.content[ pa->elementCount * pa->sizeOfElement ]);
	pa->elementCount ++;
	return t;
}

/*NAME: paGetElement()
  取得一個指定陣列元素所在的指標。
  如果無此元素，則傳回 NULL 。
  
  RC: 陣列元素的指標
  See also: parray_t, paAddElement()
*/  
C_INLINE void* paGetElement(const parray_t* pa, int n, void*elem)
{
	void* t;
	if( n >= pa->elementCount )
		return NULL;
	t = &(pa->p.content[ n * pa->sizeOfElement ]);
	if( elem != NULL )
		memcpy(elem,t,pa->sizeOfElement);
	return t;
}

/*NAME: paSetElement()
  設定指定的陣列元素的內容。
  如果指定的陣列元素不存在，則傳回 NULL ，否則傳回指定元素的目前指標。
  
  RC: 傳回指定元素的指標
  See also: parray_t, paGetElement()
*/
C_INLINE void* paSetElement(const parray_t* pa, int n, const void*elem)
{
	void* t;
	if( n >= pa->elementCount )
		return NULL;
	t = &(pa->p.content[ n * pa->sizeOfElement ]);
	memcpy(t,elem,pa->sizeOfElement);
	return t;
}

/*NAME: parraycpy()
  設定 ntiems 筆資料(elmes)，到陣列中。
  從陣列開頭處開始複製。
  
  RC: 傳回陣列內容
  See also: parray_t, parraycat()
*/
void* parraycpy(parray_t* pa, int nitems, void*elems);

/*NAME: parraycat()
  增加 ntiems 筆資料(elmes)到陣列元素中。
  RC: 傳回陣列內容
  See also: parray_t, parraycpy()
*/  
void* parraycat(parray_t* pa, int nitems, void*elems);

/*------------ phash - associative array ------------------*/
/* 2000.12.18 by rock */
struct phash {
	pool_t hashpool;
	int keyCount;
	enum {
		POOLHASH_MALLOC,
		POOLHASH_AUTO
	} type;
};
typedef struct phash phash_t;

phash_t* phashconstruct(phash_t* ph);
void phashdestroy( phash_t* ph );

C_INLINE int phashCount( phash_t* ph) {
	return ph->keyCount;
}

void phashClear( phash_t* ph );

void* phashPut( phash_t* ph, const char*key, const paElement*data );
paElement phashGet( phash_t* ph, const char*key);

/*
  return the element in phash by index.
  0 <= index value < key count -1
  key stores the key value. (count be NULL)
*/
paElement phashGetN( phash_t* ph, int idx, char *key);

int phashApply( phash_t* ph, int (*func)(const char*key, paElement*, void*), void*farg);

#if defined(__cplusplus)
}
#endif
#endif
