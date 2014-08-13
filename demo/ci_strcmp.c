/*
	ci_strcmp.c String compare routines.
	Firebird Bulletin Board System (The TIP Project version)
	BBSLIB version 2.0
		Copyright (C) 1999, Sh Yunchen, shirock@mail.educities.edu.tw
	BBSLIB version 2.0 alpha
		Copyright (C) 1998, Sh Yunchen, rock@bbs.isu.edu.tw
	BBSLIB version 1.0
		Copyright (C) 1997, Sh Yunchen, rock@bbs.touc.edu.tw

	Firebird Bulletin Board System
	Copyright (C) 1995, ?@cs.ccu.edu.tw

	Eagles Bulletin Board System
	Copyright (C) 1992, Raymond Rocker, rocker@rock.b11.ingr.com
											Guy Vega, gtvega@seabass.st.usm.edu
											Dominic Tynes, dbtynes@seabass.st.usm.edu

	Pirate Bulletin Board System
	Copyright (C) 1990, Edward Luke, lush@Athena.EE.MsState.EDU

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 1, or (at your option)
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// gcc -DMAIN -o ci_strcmp ci_strcmp.c -lregex -L.

#include <string.h>
#include <sys/types.h>
#include <regex.h>

/*NAME: ci_strcmp()
	同strcmp()，但忽略大小寫。
	
	RC: 相等(0)、不同(非0 值, s1 < s2: 負, s1 > s2: 正)
	See aslo: strcmp(), ci_strncmp()
*/
int ci_strcmp(const char*s1, const char*s2)
{
		char c1, c2;
		while( 1 ) {
				c1 = *s1++;
				c2 = *s2++;
				if( c1 >= 'a' && c1 <= 'z' )
						c1 += 'A' - 'a';
				if( c2 >= 'a' && c2 <= 'z' )
						c2 += 'A' - 'a';
				if( c1 != c2 )
						return (c1 - c2);
				if( c1 == 0 )
						return 0;
		}
}

/*NAME: ci_strncmp()
	同strncmp()，但忽略大小寫。
	
	RC: 相同(0)、不同(非0 值)
	See also: strncmp(), ci_strcmp()
*/
int ci_strncmp(const char*s1, const char*s2, int n)
{
		char				c1, c2;
		while( n-- > 0 ) {
				c1 = *s1++;
				c2 = *s2++;
				if( c1 >= 'a' && c1 <= 'z' )
						c1 += 'A' - 'a';
				if( c2 >= 'a' && c2 <= 'z' )
						c2 += 'A' - 'a';
				if( c1 != c2 )
						return (c1 - c2);
				if( c1 == 0 )
						return 0;
		}
		return 0;
}

/*===================================================================
 * util.c: string utility things
 * 
 * 3/21/93 Rob McCool
 * 1995-96 Many changes by the Apache Group
 * 
 */

/* Match = 0, NoMatch = 1, Abort = -1 */
/* Based loosely on sections of wildmat.c by Rich Salz
 * Hmmm... shouldn't this really go component by component?
 */
/*NAME: strcmp_match()		Write by Apache Group, describe by rock
	同 strcmp() ，但包含萬用字元(*, ?) 的字串比對函數，
	含有萬用字元的字串，需放在第二個參數 exp 的位置。
	當萬用字元的使用意義不明時，比對失敗。
	
	RC: 相同(0)、不同(1)、失敗(-1)
	See also: strcasecmp_match(), is_matchexp()
*/
int strcmp_match(const char *str, const char *exp) 
{
		int x,y;

		for(x=0,y=0;exp[y];++y,++x) {
				if((!str[x]) && (exp[y] != '*'))
						return -1;
				if(exp[y] == '*') {
						while(exp[++y] == '*');
						if(!exp[y])
								return 0;
						while(str[x]) {
								int ret;
								if((ret = strcmp_match(&str[x++],&exp[y])) != 1)
										return ret;
						}
						return -1;
				} else 
						if((exp[y] != '?') && (str[x] != exp[y]))
								return 1;
		}
		return (str[x] != '\0');
}

/*NAME: strcasecmp_match()	Write by Apache Group, describe by rock
	同 strcmp_match() ，但是忽略字母大小寫的差異。
	含有萬用字元的字串，需放在第二個參數 exp 的位置。
	當萬用字元的使用意義不明時，比對失敗。
	
	RC: 相同(0)、不同(1)、失敗(-1)
	See also: strcmp_match(), is_matchexp()
*/
int strcasecmp_match(const char *str, const char *exp) 
{
		int x,y;

		for(x=0,y=0;exp[y];++y,++x) {
				if((!str[x]) && (exp[y] != '*'))
						return -1;
				if(exp[y] == '*') {
						while(exp[++y] == '*');
						if(!exp[y])
								return 0;
						while(str[x]) {
								int ret;
								if((ret = strcasecmp_match(&str[x++],&exp[y])) != 1)
										return ret;
						}
						return -1;
				} else 
						if((exp[y] != '?') && (tolower(str[x]) != tolower(exp[y])))
								return 1;
		}
		return (str[x] != '\0');
}

/*NAME: is_matchexp()		Write by Apache Group, describe by rock
	指定的字串是否含有萬用字元(*, ?)。
	
	RC: 含萬用字元(1)、不含(0)
	See also: strcmp_match(), strcasecmp_match()
*/
int is_matchexp(const char *str) 
{
		register int x;

		for(x=0;str[x];x++)
				if((str[x] == '*') || (str[x] == '?'))
						return 1;
		return 0;
}

/*
	strcmp_match(), strcasecmp_match(), is_matchexp()
	是從 Apache 的 util.c 中擷取出來的。
	=====================================================================*/

/*
	POSIX regex functions: regcomp(), regexec(), regfree()

	strcmp_regex1() 是我利用 POSIX 正規運算式regex (regular expresstion) 
	函數所實作的正規運算式比對函數。
	
	1998	Write by rock
	
	以下是一些可用的運算規則:
	------------------------
	^		定位規則，文字開頭
	$		定位規則，文字尾端
	.		單一規則，任何單一字元
	[chars]	單一規則，有 chars 裡其中一個字元
	[^chars]	單一規則，沒有 chars 裡其中一個字元
	?		倍數規則， 0 或 1 個的前導字元的字元
	*		倍數規則， 0 或多個的前導字元的字元
	+		倍數規則， 1 或多個的前導字元的字元
	\char		跳脫規則，將 char 視為一般字元，而非運算規則字元
	(string)	記憶規則，將 string 記憶起來，歸於一組。
			稍後可利用 \n 的方式，將第 n 組 string 提出。
	------------------------
	運算例:
	------------------------
	運算式	可符合的例子
	^11.*		112 , 113 , 114, 115424fdsa
	以 11 為文字開端，第三個以後，有 0 或多個任何字元的文字。
	
	.*abc$	1abc , 89u9u9uabc
	前面為 0 或多個任何字元，但尾端是 abc 的文字。
	
	([a-z])([a-z])\2\1	otto , cddc
	以 otto 為例， o 有 [a-z] 的其中一個字元，將 o 記憶起來，歸於第 1 組，
	t 有 [a-z] 的其中一個字元，將 t 記憶起來，歸於第 2 組。
	\2 將 t 拿出來， \1 將 o 拿出來。
	------------------------
*/

/*NAME: strcmp_regex1()		write by rock
	基本的正規運算式字串比對函數。
	
	運算式字串要放在第二個參數 regex 。
	
	cflags 是比對時的一些旗標，可用的旗標值有:
	REG_ICASE, REG_NOSUB, REG_EXTENDED 等，定義於 <regex.h> 中。
	在 strcmp_regex1() 中，旗標 REG_NOSUB 一定會被設定。
	當使用的運算式規則不明或不支援時，回傳 -1 。
	
	RC: 符合運算式(0)、不符(1)、失敗(-1)
	See also: regcomp(), regexec(), strcmp_regex(), strcasecmp_regex()
*/
int strcmp_regex1(const char*str, const char*regex, int cflags)
{
	regex_t preg;
	int rc;
	if( regcomp(&preg, regex, cflags | REG_NOSUB) != 0 )
		return -1;
	rc = regexec(&preg, str, 0, NULL, 0);
	regfree(&preg);
	return rc;
}

/*NAME: strcmp_regex()		write by rock
	正規運算式字串比對函數。
	
	呼叫 strcmp_regex1() ，設定比對旗標: REG_EXTENDED ，使用 
	POSIX 擴充運算規則。
	
	RC: 符合運算式(0)、不符(1)、失敗(-1)
	See also: strcmp_regex1(), strcasecmp_regex()
*/
int strcmp_regex(const char*str, const char*regex)
{
	return strcmp_regex1(str, regex, REG_NOSUB | REG_EXTENDED);
}

/*NAME: strcasecmp_regex()	write by rock
	正規運算式字串比對函數，忽略字母大小寫差異。
	
	呼叫 strcmp_regex1() ，設定比對旗標: REG_EXTENDED 及 REG_ICASE
	，使用 POSIX 擴充運算規則，且忽視字母大小寫。
	
	RC: 符合運算式(0)、不符(1)、失敗(-1)
	See also: strcmp_regex1(), strcmp_regex()
*/
int strcasecmp_regex(const char*str, const char*regex)
{
	return strcmp_regex1(str, regex, REG_ICASE | REG_NOSUB | REG_EXTENDED);
}


#ifdef MAIN

#include <stdio.h>
#include <stdlib.h>
int main()
{
	char reg[4096], str[4096];
	int rc;
	while(1)
	{
		printf("Input exp: ");
		fgets(reg,4096,stdin);
		if(reg[0] == '\n')
			break;
		strtok(reg,"\n");
		printf("Input str: ");
		fgets(str,4096,stdin);
		if(str[0] == '\n')
			break;
		strtok(str,"\n");
		
		rc = ci_strcmp(str,reg);
		printf("ci_strcmp(%s,%s) = %d\n", str,reg,rc);
		rc = strcmp_match(str,reg);
		printf("strcmp_match(%s,%s) = %d\n", str,reg,rc);
		rc = strcmp_regex(str,reg);
		printf("strcmp_regex(%s,%s) = %d\n", str,reg,rc);
	}
	return 0;
}
#endif
