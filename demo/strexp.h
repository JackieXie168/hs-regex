/*
  strexp.h String expression routines.
  Firebird Bulletin Board System (The TIP Project version)
  BBSLIB version 2.0
    Copyright (C) 1999, Sh Yunchen, shirock@mail.educities.edu.tw

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
#if !defined( __STREXP_H )
#define __STREXP_H
#if defined(__cplusplus)
extern "C" {
#endif

#include <string.h>
#include "bbsdefs.h"
#include "ci_strcmp.h"

#define ISEMPTYSTR(str)  (str==NULL || str[0]=='\0' ? 1 : 0)

C_INLINE char*lcase(char*s) {
	char*c;
	if(s) {
		c=s;
		while(*c) {
			if( *c >= 'A' && *c <= 'Z')
				*c += '\x20' /* 'A' - 'a' */;
			c++;
		}
	}
	return s;
}

C_INLINE char*ucase(char*s) {
	char*c;
	if(s) {
		c=s;
		while(*c) {
			if( *c >= 'a' && *c <= 'z')
				*c -= '\x20';
			c++;
		}
	}
	return s;
}

#if defined(__cplusplus)
}
#endif
#endif
