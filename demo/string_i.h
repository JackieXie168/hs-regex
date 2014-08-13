/*
  string_i.h Header file of string interface.
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
#if !defined( __STRING_I_H )
#define __STRING_I_H

#include "strexp.h"

struct stringinterface 
{
  void* striobj;
  size_t (*length)(const void*);
  char* (*text)(const void*);
  char* (*strcpy)(void*, const char*);
  char* (*strncpy)(void*, const char*, int);
  char* (*strcat)(void*, const char*);
  char* (*strncat)(void*, const char*, int);
};
typedef struct stringinterface string_i;

#include "pool.h"
#include "buffer.h"

#endif	// defined( __STRING_I_H )
