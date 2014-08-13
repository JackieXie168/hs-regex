/*
	ci_strcmp.h Header file of string compare routines.
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
#if !defined( __CI_STRCMP_H )
#define __CI_STRCMP_H
#if defined(__cplusplus)
extern "C" {
#endif

/*NAME: ci_strcmp()
	同strcmp()，但忽略大小寫。
	
	RC: 相等(0)、不同(非0 值, s1 < s2: 負, s1 > s2: 正)
	See aslo: strcmp(), ci_strncmp()
*/
int ci_strcmp(const char*s1, const char*s2);

/*NAME: ci_strncmp()
	同strncmp()，但忽略大小寫。
	
	RC: 相同(0)、不同(非0 值)
	See also: strncmp(), ci_strcmp()
*/
int ci_strncmp(const char*s1, const char*s2, int n);

/*NAME: strcmp_match()		Write by Apache Group, describe by rock
	同 strcmp() ，但包含萬用字元(*, ?) 的字串比對函數，
	含有萬用字元的字串，需放在第二個參數 exp 的位置。
	當萬用字元的使用意義不明時，比對失敗。
	
	RC: 相同(0)、不同(1)、失敗(-1)
	See also: strcasecmp_match(), is_matchexp()
*/
int strcmp_match(const char *str, const char *exp) ;

/*NAME: strcasecmp_match()	Write by Apache Group, describe by rock
	同 strcmp_match() ，但是忽略字母大小寫的差異。
	含有萬用字元的字串，需放在第二個參數 exp 的位置。
	當萬用字元的使用意義不明時，比對失敗。
	
	RC: 相同(0)、不同(1)、失敗(-1)
	See also: strcmp_match(), is_matchexp()
*/
int strcasecmp_match(const char *str, const char *exp) ;

/*NAME: is_matchexp()		Write by Apache Group, describe by rock
	指定的字串是否含有萬用字元(*, ?)。
	
	RC: 含萬用字元(1)、不含(0)
	See also: strcmp_match(), strcasecmp_match()
*/
int is_matchexp(const char *str) ;

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
int strcmp_regex1(const char*str, const char*regex, int cflags);

/*NAME: strcmp_regex()		write by rock
	正規運算式字串比對函數。
	
	呼叫 strcmp_regex1() ，設定比對旗標: REG_EXTENDED ，使用 
	POSIX 擴充運算規則。
	
	RC: 符合運算式(0)、不符(1)、失敗(-1)
	See also: strcmp_regex1(), strcasecmp_regex()
*/
int strcmp_regex(const char*str, const char*regex);

/*NAME: strcasecmp_regex()	write by rock
	正規運算式字串比對函數，忽略字母大小寫差異。
	
	呼叫 strcmp_regex1() ，設定比對旗標: REG_EXTENDED 及 REG_ICASE
	，使用 POSIX 擴充運算規則，且忽視字母大小寫。
	
	RC: 符合運算式(0)、不符(1)、失敗(-1)
	See also: strcmp_regex1(), strcmp_regex()
*/
int strcasecmp_regex(const char*str, const char*regex);

#if defined(__cplusplus)
}
#endif
#endif
