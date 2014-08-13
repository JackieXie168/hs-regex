/*
    rowconv.h   row converter.
    Firebird Bulletin Board System (The TIP Project version)
    BBSLIB library version 2.0
    Copyright (C) 2001, Sh Yunchen <shirock@mail.educities.edu.tw>

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
#if !defined( __ROWCONV_H )
#define __ROWCONV_H
#if defined(__cplusplus)
extern "C" {
#endif
#include "string_i.h"
#include "pool.h"

#define MAX_FIELDNAME_LEN	256

/* General entities translate */
char*entity_trans(const char*src, string_i*dst);

/*<key>data</key> => hash */
phash_t* row2hash(phash_t*ph, const char*rows);

/* hash => <key>data</key> */
pool_t* hash2row(pool_t*rows, phash_t*ph);

/*
rowstr = <a>A</a><b>b</b>...
You should construct and init content.
It will cat(not copy) field value to content if the field found else
do nothing for content.

return content.text or NULL
*/
char* row_getfield(const char*rowstr, const char*field_name, string_i*content);

#if defined(__cplusplus)
}
#endif
#endif
