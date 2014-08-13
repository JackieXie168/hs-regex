/*
  parse.h Header file of string parse routines.
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
#if !defined( __PARSE_H )
#define __PARSE_H
#if defined(__cplusplus)
extern "C" {
#endif
#include "pool.h"

/*
parsing_buffer 
+-------------------+-------------------------------------+
|buf,p,specs,ignores|parsestr text|specs text|ignores text|
+-------------------+-------------------------------------+
*/
typedef struct {
	char*	buf;
	char*	p;
	char*	specs;
	char*	ignores;
	size_t	more_siz;
#if defined(MTHREAD)
	char*	lasts;
#endif
} parsing_buffer;

parsing_buffer* parsing_start (const char*parsestr, const char*specs, const char*ignores);

/*
若初設的字串已經解析完了，但還有更多的字串要處理時，
呼叫此函數，以處理更多的字串。
*/
char* parsing_more(parsing_buffer*pb, const char*parsestr);

/*
return the position as pointer.
*/
C_INLINE char* parsing_getpos(parsing_buffer*pb) {
	return (pb ? pb->p : NULL);
}

/*
if(ch is not null)
	seek to any space(" \t\r\n").
if(ch not found)
	return NULL
seek to ch and return the new position as pointer.
*/
char* parsing_seek(parsing_buffer*pb, char ch);

char* parsing_get (parsing_buffer*pb, string_i*token);

C_INLINE void parsing_end(parsing_buffer*pb) {
	if(pb) {
		if(pb->more_siz >0)
			free(pb->buf);
		free(pb);
	}
}


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
struct tokens {
  parray_t index;
  pool_t   content;
};
typedef struct tokens tokens_t;


/*NAME: tokens_init()
  初始一個 tokens 物件，供 parse_toke() 存放分析出來的 token
*/
C_INLINE void tokens_init(tokens_t*tokens)
{
  parrayconstruct(&tokens->index, sizeof(char*)*10, sizeof(char*)*10, sizeof(char*));
  pconstruct(&tokens->content);
}

/*NAME: tokens_free()
  釋放 tokens 中所配置的資源。
*/
C_INLINE void tokens_free(tokens_t*tokens)
{
  parraydestroy(&tokens->index);
  pdestroy(&tokens->content);
}

/*NAME: tokens_clear()
  清除(重置) tokens 中的內容。
*/
C_INLINE char* tokens_clear(tokens_t*tokens)
{
  paClear(&tokens->index);
  pclear(&tokens->content);
}

/*NAME: gettoken(tokens,n)
  取得 token 的指標 charr* 。
*/
C_INLINE char* gettoken(tokens_t*tokens, int n)
{
  char *elem;
  if( paGetElement(&tokens->index, n, &elem) != NULL)
    return pcontent(&tokens->content) + (int)elem;
  else
    return NULL;
}

//void tokens_add(tokens_t*token, const char*str);
//void tokens_add_nbytes(tokens_t*token, const char*str, int nbytes);

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
int parse_token(const char*parsestr, const char*specs, const char*ignores, tokens_t* token);

#if defined(__cplusplus)
}
#endif
#endif
