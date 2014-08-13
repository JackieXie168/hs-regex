/*
  bbsdefs.h
  Firebird Bulletin Board System (The TIP Project version)
  BBSLIB version 2.0
    Copyright (C) 1999, Sh Yunchen, shirock@mail.educities.edu.tw
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
//=================================================
//      BBS 各項常數及型態定義
//	影嚮範圍: 全部
//=================================================

#ifndef _BBS_DEFS_H
#define _BBS_DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//==================================================
//      一般用途
//==================================================
#ifndef YEA
#define YEA	(1)	/* Booleans  (for true and false) */
#define NA	(0)
#endif
#ifndef TRUE
#define TRUE	(1)
#define FALSE	(0)
#endif
#ifndef ENABLE
#define ENABLE	(1)
#define DISABLE	(0)
#endif

#define RC_SUCCESS	 	(int)0
#define RC_FAULT		(int)-1

#if !defined(NAME_MAX)			// NAME_MAX is POSIX definition.
#if defined(FILENAME_MAX)
#define NAME_MAX	FILENAME_MAX	// FILENAME_MAX is BSD definition.
#else
#define NAME_MAX	240	// It's a conservative value for most system.
#endif
#endif

#if !defined(C_INLINE)
#define C_INLINE	static __inline__ 
#endif

#define QUIT 	0x666	/* 中止遞回函數的特定傳回值 */

//概略性字串長度，沒有算入終端字元0 的長度。
#define SSTRLEN	40	/* Same as STRLEN but shorter (short) */
#define STRLEN	80	/* 一般字串資料的長度 (normal) */
#define LSTRLEN	256	/* 長字串資料的長度 (long) */
#define KSTRLEN	1024	/* 最長字串資料的長度 (kilo) */

//以下所定義的 LEN 皆為包含終端字元0 的長度。

//#define IDLEN	14	/* userid 的長度 (含結尾字元) */
#define IDLEN	80

//#define NAMELEN	20	/* 名稱字串的長度 (Username/realname) */

#define BOARDNAMELEN	80	/* 討論區名稱 (Boardname)字串長度 */

#define ULIST	".UTMP"		/* 上線使用者的動態，上線動態資訊儲存在共享記憶體中，此檔為共享記憶體之key */

#define BOARDS	".BOARDS"	/* 討論區資料檔(討論區列表) */

#define DOT_DIR		".DIR"		/* Name of Directory file info */
#define DIGEST_DIR	".DIGEST"	/* Name of Directory file info */
#define THREAD_DIR	".THREAD"

#define DIR_NORMAL	0	/* 一般模式 */
#define DIR_DIGEST	1	/* 文摘模式 */
#define DIR_THREAD	2	/* 主題模式 */

#define MODE_FILE	0640
#define MODE_DIR	0750

#if 0
#define NAMEFILE "BoardName"    //?? /* File containing site name of bbs */
#define USERIDSIZE (16)		//??
#define USERNAMESZ (24)		//??
#define TERMTYPESZ (10)		//??
#define ZAPPED  0x1         //??  /* For boards...tells if board is Zapped */
#endif

//=================================================
//      macro define: 常用巨集定義
//=================================================
#if !defined( bell )
#define bell()		write(STDOUT_FILENO, "\a", 1);
#endif
#if !defined( beep )
#define beep()		write(STDOUT_FILENO, "\a", 1);
#endif

#if !defined( chartoupper )
#define chartoupper(c)  ((c >= 'a' && c <= 'z') ? c+'A'-'a' : c)
#endif

#if !defined( STRNCPY )
#define STRNCPY(d,s,n)	{strncpy(d,s,n); d[n-1]='\0';}
#endif

#if !defined( BIT_SET )
#define BIT_SET( n, x)	(n |= (x))
#endif
#if !defined( BIT_UNSET )
#define BIT_UNSET( n, x)	(n &= ~(x))
#endif
#if !defined( BIT_ISSET )
#define BIT_ISSET( n, x)	(n & (x) ? 1: 0)
#endif


#if !defined(stat_t)
typedef	struct stat	stat_t;
#endif
//#include "struct.h"

#endif // _BBS_DEFS_H
