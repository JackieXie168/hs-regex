/*
  parse.c String parse routines.
  Firebird Bulletin Board System (The TIP Project version)
  BBSLIB version 2.0 alpha
    Copyright (C) 1998, Sh Yunchen, rock@bbs.isu.edu.tw

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
  parse
    rock
    1998.10
    
    為了在實作 POP3, NNTP 等協定時，方便解析收到的命令，所以
    我寫了這個玩意，當然啦，它能分析的字串，不只是 POP3 式的
    簡單字串。
    
    如何 parse:
    基本:
    1.以空白及TAB 字元分離 token
    2.可以使用 "" 或 '' 括住字串，被 "" 或 '' 括住的字串視為一個 token
    例:
    USER rock 
    => token 0 = USER, token 1 = rock
    SEND "I'm rock" NORMAL
    => token 0= SEND, token 1=I'm rock, token 2=NORMAL
    
    進階
    3.以參數 spesc 設定將特定的字元視為 token
    例: spesc = "=,;"
    FINDPATH = . ,/usr/doc,/usr/local/doc
    => token 0: FINDPATH 
       token 1: =
       token 2: .
       token 3: ,
       token 4: /usr/doc
       token 5: ,
       token 6: /usr/local/doc
       
    4.以參數 ignores 設定忽視不用的 token
    參數 ignores 必須配合參數 specs ，如果 specs 沒有設定的話，那麼
    ignores 就不會發生作用。
    再以上例來說，雖然 , 這個字元被視為一個 token，但這只是為了分離
    token 而已，在實際使用時，這個 token 對我們來說並沒有作用，所以
    我們可以設定 ignores = ","
    則上例的結果變成:
    FINDPATH = . ,/usr/doc,/usr/local/doc
    => token 0: FINDPATH 
       token 1: =
       token 2: .
       token 3: /usr/doc
       token 4: /usr/local/doc
*/
#include <stdlib.h>
#include <string.h>
#include "bbsdefs.h"
#include "pool.h"
#include "parse.h"

/* 2001.3.16  rock (石頭成)
為了改善解析動作上的彈性，增加一組函數 parsing:
parsing_start(), parsig_get(), pasing_end().

parse_token() 是一次解析全部的 token 後才回傳，適合
單行指令或字串的解析。
而這組 parsing 函數，是一次抓一個 token ，較適合用在
整份文件的解析。

加上對 multi-thread safe 的修改:
strtok() => strtok_r()
*/


/*
  剖析 token
  parsestr: 
    待剖析字串
  spesc: 
    視為單獨 token 的字元
  ignores:
    忽視不用的 token 的字元
*/
parsing_buffer* parsing_start (const char*parsestr, const char*specs, const char*ignores) {
	parsing_buffer* pb;
	size_t pbsiz;

	if(!parsestr)
		return NULL;
	pbsiz = sizeof(parsing_buffer) +
		strlen(parsestr)+1;
	/* 有指定 specs 時，ignores 的內容才有作用 */
	if(specs) {
		pbsiz += strlen(specs)+1;
		if(ignores)
			pbsiz += strlen(ignores)+1;
	}
	if( !(pb = (parsing_buffer*) malloc(pbsiz)))
		return NULL;

	memset(pb, 0, sizeof(parsing_buffer));
	pb->buf = pb->p = sizeof(parsing_buffer) + (char*)pb;
	strcpy(pb->buf, parsestr);
	/* 有指定 specs 時，ignores 的內容才有作用 */
	if(specs) {
		pb->specs = pb->buf + strlen(parsestr)+1;
		strcpy(pb->specs, specs);
		if(ignores) {
			pb->ignores = pb->specs + strlen(specs)+1;
			strcpy(pb->ignores, ignores);
		}
	}
	return pb;
}

/*
若初設的字串已經解析完了，但還有更多的字串要處理時，
呼叫此函數，以處理更多的字串。
*/
char* parsing_more(parsing_buffer*pb, const char*parsestr) {
	size_t	siz;
	if(!pb || !parsestr)
		return NULL;
	siz = strlen(parsestr)+1;
	if(pb->more_siz > 0 && pb->more_siz < siz) {
		/* current space is too small, 
		free it then allocate new one. */
		free(pb->buf);
		pb->more_siz = 0;
	}
	if(pb->more_siz == 0) {
		pb->buf = (char*)malloc(siz);
		memcpy(pb->buf, parsestr, siz);
		pb->more_siz = siz;
	}
	else {
		memcpy(pb->buf, parsestr, siz);
	}
	return (pb->p = pb->buf);
}

/*
return the position as pointer.
*/
/*char* parsing_getpos(parsing_buffer*pb) {
	return (pb ? pb->p : NULL);
}*/

/*
if(ch is not null)
	seek to any space(" \t\r\n").
if(ch not found)
	return NULL
seek to ch and return the new position as pointer.
*/
char* parsing_seek(parsing_buffer*pb, char ch) {
	char* np;
	if(!pb)
		return NULL;
	if(ch) {
		np = strchr(pb->p, ch);
	}
	else {
		np = pb->p;
		while(*np != '\0' && *np != ' ' && *np != '\t' &&
		  *np != '\n' && *np != '\r')
			np++;
		if(*np == '\0')
			np = NULL;
	}
	if(np)
		pb->p = np;
	return np;
}

char* parsing_get (parsing_buffer*pb, string_i*token) {
	enum {NONE, DOUBLE = '"', SINGLE = '\''} quotation; // 括號種類
	char *ptoken;
	int n;

	if(pb==NULL || token==NULL)
		return NULL;

	//for( n=0; /*n < siz*/; n++) {
	while(1) {
		if( pb->p == NULL )
			break;

		// 跳過空白及 TAB 字元
		while(*pb->p==' ' || *pb->p=='\t' || *pb->p=='\n' || *pb->p=='\r')
			pb->p++;
		if( *pb->p == '\0' )        // 如果字串已結束，則結束分析
			break;

		if( *pb->p == '"' )         // 設定括號種類
			quotation = DOUBLE;
		else if( *pb->p == '\'' )
			quotation = SINGLE;
		else
			quotation = NONE;

		if( quotation != NONE ) {
			pb->p++;           // 跳過括號字元
			ptoken = pb->p;
			while( *pb->p != quotation && *pb->p != '\0')
				pb->p++;         // 找尋配對的括號
			if( *pb->p != '\0' ) {
				*pb->p = '\0';   // 消掉括號字元
				pb->p++;         // 指向下一個字元
				//strcpy(token[n], ptoken);
				//tokens_add(token, ptoken);
				return token->strcpy(token, ptoken);
			}
		}
		else {
			if( pb->specs == NULL ) {
				#if !defined(MTHREAD)
				ptoken = strtok(pb->p, " \t\r\n");
				#else
				ptoken = strtok_r(pb->p, " \t\r\n", &pb->lasts);
				#endif
				//strcpy(token[n], ptoken);
				//tokens_add(token, ptoken);
				ptoken = token->strcpy(token, ptoken);
				#if !defined(MTHREAD) 
				pb->p = strtok(NULL, "\0");
				#else
				pb->p = strtok_r(NULL, "\0", &pb->lasts);
				#endif
				return ptoken;
			}
			else {
				ptoken = pb->p;
				while( (*pb->p != ' ' && *pb->p != '\t') && strchr(pb->specs,*pb->p) == NULL )
					pb->p++;
				if( pb->p == ptoken ) {
					n = NA;
					if( pb->ignores == NULL ) {
						//tokens_add_nbytes(token, pch, 1);
						/*因為strncpy()不會在尾端加上'\0'(當來源字串長度大於len時),
						所以要求多copy一個字元，再將多出的那個字元設為'\0'*/
						ptoken = token->strncpy(token, pb->p, 2);
						ptoken[1] = '\0';
						n = YEA;
						//return ptoken;
					}
					else if( strchr(pb->ignores,*pb->p) == NULL ) {
						//token[n][0] = *pch;
						//token[n][1] = '\0';
						//tokens_add_nbytes(token, pb->p, 1);
						ptoken = token->strncpy(token, pb->p, 2);
						ptoken[1] = '\0';
						n = YEA;
						//return ptoken;
					}
					/*else
						n--;*/
					pb->p++;
					if(n)	return ptoken;
				}
				else {
					pb->p--;
					while( *pb->p == ' ' || *pb->p == '\t' || *pb->p=='\r' || *pb->p=='\n')
						pb->p--;
					pb->p++;
					//strncpy(token[n], ptoken, pch - ptoken);
					//token[n][ pch - ptoken ] = '\0';
					//tokens_add_nbytes(token, ptoken, pch - ptoken);
					/*因為strncpy()不會在尾端加上'\0'(當來源字串長度大於len時),
					所以要求多copy一個字元，再將多出的那個字元設為'\0'*/
					n = pb->p - ptoken;
					ptoken = token->strncpy(token, ptoken, n+1);
					ptoken[n] = '\0';
					return ptoken;
				}
			}
		}
	}
	return NULL/*token->strcpy(token, ptoken)*/;
}

/*void parsing_end(parsing_buffer*pb) {
	if(pb)
		free(pb);
}*/


/*
  由於要分析的字串中，所含有的 token 數目不定，所以我
  又利用了 pool 及 parray 來實作一個功能，具有:
  1.可存放不定數目的 token 
  2.可存放不限長度的 token 內容
  我將所有分出的 token 內容，存入 pool 中，再利用 parray
  來存放這些 token 被存入 pool 的位置。
  注意，因為 pool 在必要時會重配置記憶體，導致其指標會改
  變，因此，不能在 parray 中，直接存放 token 的指標，而
  要存偏移值，將此偏移值與 pool 目前的指標相加，就可以
  得到 token 的指標了。
*/
/*struct tokens {
  parray_t index;
  pool_t   content;
};
typedef struct tokens tokens_t;*/

/*NAME: tokens_init()
  初始一個 tokens 物件，供 parse_toke() 存放分析出來的 token
*/
/*void tokens_init(tokens_t*tokens)
{
  parrayconstruct(&tokens->index, sizeof(char*)*10, sizeof(char*)*10, sizeof(char*));
  pconstruct(&tokens->content);
}*/


/*NAME: tokens_free()
  釋放 tokens 中所配置的資源。
*/
/*void tokens_free(tokens_t*tokens)
{
  parraydestroy(&tokens->index);
  pdestroy(&tokens->content);
}*/


/*NAME: tokens_clear()
  清除(重置) tokens 中的內容。
*/
/*char* tokens_clear(tokens_t*tokens)
{
  paClear(&tokens->index);
  pclear(&tokens->content);
}*/


/*NAME: gettoken(tokens,n)
  取得 token 的指標 char* 。
  tokens.index 是存的是 token 在 tokens.content 的偏移值，所以
  要再加上 tokens.content 的位址，才是 token 的指標。
*/
/*char* gettoken(tokens_t*tokens, int n)
{
  char *elem;
  if( paGetElement(&tokens->index, n, &elem) != NULL)
    return pcontent(&tokens->content) + (int)elem;
  else
    return NULL;
}*/


/*
  pool
    +-----+-+----------+-+-------+-+-------
    |str0 |0| str1     |0|str2   |0|...
    +-----+-+----------+-+-------+-+-------
  pmemcat(pool, str, strlen+1 ) -> include \0
  
  parray
    +--------------+--------------+--------------+----
    |offset to str0|offset to str1|offset to str2|...
    +--------------+--------------+--------------+----
  offset to next str = base + length of pool -> include \0
  point to str = base + offset
  paAddElement(parray, &point to next)    
*/
C_INLINE void tokens_add(tokens_t*token, const char*str)
{
  char*p;
  p = (char*) plength(&token->content);
  pmemcat(&token->content, str, strlen(str)+1);	/* include \0 */
  paAddElement(&token->index, &p);
}

C_INLINE void tokens_add_nbytes(tokens_t*token, const char*str, int nbytes)
{
  char *p, zero='\0';
  p = (char*) plength(&token->content);
  pmemcat(&token->content, str, nbytes);
  pmemcat(&token->content, &zero, 1);	/* cat '\0' */
  paAddElement(&token->index, &p);
}

/*NAME: parse_token()
  剖析 token
  parsestr: 
    待剖析字串
  spesc: 
    視為單獨 token 的字元
  ignores:
    忽視不用的 token 的字元
  tokens:
    存放分析出來的 token 的物件。
*/
int parse_token(const char*parsestr, const char*specs, const char*ignores, tokens_t* token) {
#if 1
	int n = 0;
	char *s;
	pool_t t;
	parsing_buffer*pb;

	pconstruct(&t);
	pb = parsing_start(parsestr, specs, ignores);
	while( (s=parsing_get(pb, (string_i*)&t)) ) {
		tokens_add(token, s);
		n++;
	}
	parsing_end(pb);
	pdestroy(&t);
	return n;
#else
    enum {NONE, DOUBLE = '"', SINGLE = '\''} quotation; // 括號種類
    char parsebuf[LSTRLEN];	// 不用在意此buf的容量，若不夠，會自動配置足夠的空間。
    char *pch;		// 指向待分析的字串
    char *ptoken;
    int n, needfree;	// 已解出 token 的計數

    if( parsestr == NULL)
      return 0;

    if( strlen(parsestr) >= sizeof(parsebuf) ) {
      pch = (char*) malloc(strlen(parsestr)+1);
      needfree=1;
    } else {
      pch = parsebuf;
      needfree=0;
    }
    strcpy(pch, parsestr);
    
    for( n=0; /*n < siz*/; n++)
    {
      if( pch == NULL )
        break;

      // 跳過空白及 TAB 字元
      while( *pch==' ' || *pch=='\t' || *pch=='\n' || *pch=='\r')
        pch++;
      if( *pch == '\0' )        // 如果字串已結束，則結束分析
        break;

      if( *pch == '"' )         // 設定括號種類
        quotation = DOUBLE;
      else if( *pch == '\'' )
        quotation = SINGLE;
      else
        quotation = NONE;

      if( quotation != NONE ) {
        pch++;           // 跳過括號字元
        ptoken = pch;
        while( *pch != quotation && *pch != '\0')
          pch++;         // 找尋配對的括號
        if( *pch != '\0' ) {
          *pch = '\0';   // 消掉括號字元
          pch++;         // 指向下一個字元
          //strcpy(token[n], ptoken);
          tokens_add(token, ptoken);
        }
      }
      else {
        if( specs == NULL ) {
          ptoken = strtok(pch, " \t\r\n");
          //strcpy(token[n], ptoken);
          tokens_add(token, ptoken);
          pch = strtok(NULL, "\0");
        }
        else {
          ptoken = pch;
          while( (*pch != ' ' && *pch != '\t') && strchr(specs,*pch) == NULL )
            pch++;
          if( pch == ptoken ) {
            if( ignores == NULL )
              tokens_add_nbytes(token, pch, 1);
            else if( strchr(ignores,*pch) == NULL )
              //token[n][0] = *pch;
              //token[n][1] = '\0';
              tokens_add_nbytes(token, pch, 1);
            else
              n--;
            pch++;
          }
          else {
            pch--;
            while( *pch == ' ' || *pch == '\t' || *pch=='\r' || *pch=='\n')
              pch--;
            pch++;
            //strncpy(token[n], ptoken, pch - ptoken);
            //token[n][ pch - ptoken ] = '\0';
            tokens_add_nbytes(token, ptoken, pch - ptoken);
          }
        }
      }
    }
    if( needfree )
      free(pch);
    return n;
#endif
}


#ifdef MAIN
#include <stdio.h>
void main()
{
  int argc, i;
  char inbuf[512], *p;
  tokens_t token;
  
  parrayconstruct(&token.index, sizeof(char*)*10, sizeof(char*)*10, sizeof(char*));
  pconstruct(&token.content);

  while(1)
  {
    fputs("> ", stdout);
    fgets(inbuf,sizeof(inbuf), stdin);

    //cmdstr = strtok(inbuf," \n\r\t");
    //argstr = strtok(NULL,"\n\r");
    paClear(&token.index);
    argc = parse_token(inbuf, "=,;", "=,;", &token);

    printf("argc=%d, element=%d\n", argc, paElementCount(&token.index));    
    
    if( argc )
      for( i=0; i < argc; i++ ) {
        paGetElement(&token.index, i, &p);
        printf("token[%d] = %s.\n", i, p);
      }
    
  }
}
#endif
