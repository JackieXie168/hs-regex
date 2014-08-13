/*
    rowconv.c   row converter.
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
#include <stdlib.h>
#include <string.h>
#include "rowconv.h"

#define CHAR_LT	'<'
#define CHAR_GT	'>'
#define CHAR_SLASH	'/'
#define CHAR_AMP	'&'

/* XML 內建的通用實體(general entities) */
static char entities_index[] = "<>&\"";
static struct {
	char*symb;
	int len;
} entities[]={
	"&lt;", 4,
	"&gt;", 4,
	"&amp;", 5,
	"&quot;", 6,
	NULL, 0
};

/* General entities translate */
char*entity_trans(const char*src, string_i*dst) {
	int i;
	const char *ptext1, *ptext2, *ps;

	if(!src || !dst)
		return NULL;
	ps = ptext1 = ptext2 = src;

	while(*ps != '\0') {
		if( *ps != CHAR_AMP ) {
			ptext2++;
		}
		else {
			for(i=0; entities[i].symb !=NULL; i++) {
				if(strncmp(ps, entities[i].symb, entities[i].len) ==0) {
					dst->strncat(dst, ptext1, ptext2 - ptext1);
					dst->strncat(dst, entities_index + i, 1);
					ps = ps + entities[i].len -1;
					break;
				}
			}
			/*若有單獨的 & 字元存在,仍將其視為一般字元,
			  嚴格說來不會有單獨的 & 字元存在，此為容錯. */
			if( *ps == CHAR_AMP )
				ptext2++;
			else
				ptext1 = ptext2 = ps+1;
		}
		ps++;
	}
	return dst->strncat(dst, ptext1, ptext2 - ptext1);
}

/*<key>data</key> => hash */
phash_t* row2hash(phash_t*ph, const char*rows) {
	int i;
	char key[MAX_FIELDNAME_LEN];
	pool_t text;
	paElement pa_text;
	const char *pkey1, *pkey2, *ptext1, *ptext2, *ps;

	ps = rows;
	if(ph == NULL)
		ph = phashconstruct(NULL);
	pconstruct(&text);

	while(*ps != '\0') {
	/*------------------------------------------------------*/
		while(*ps != CHAR_LT && *ps != '\0')
			ps++;
		if(*ps == '\0') 	break;
		else 	ps++;

		//pkey=key;
		pkey1 = pkey2 = ps;
		while(*ps != CHAR_GT && *ps != ' ') {
			//*pkey = *ps; pkey++;
			ps++;
			pkey2++;
		}
		while(*ps != CHAR_GT)
			ps++;
		ps++;
		//*pkey = '\0';
		strncpy(key, pkey1, pkey2 - pkey1);
		key[pkey2 - pkey1] = '\0';
	/* ------------------------------------------------------------*/
		//ptext = text;
		//text[0] = '\0';
		pclear(&text);
		ptext1 = ptext2 = ps;
		while(*ps !=CHAR_LT && *ps != '\0') {
			if( *ps != CHAR_AMP ) {
				//*ptext = *ps;
				//ptext++;
				ptext2++;
			}
			else {
				for(i=0; entities[i].symb !=NULL; i++) {
					if(strncmp(ps, entities[i].symb, entities[i].len) ==0) {
						pstrncat(&text, ptext1, ptext2 - ptext1);
						//*ptext = spec_symbolic[i].symb[0];
						pcharcat(&text, entities_index[i]);
						//ptext++;
						ps = ps + entities[i].len -1;
						break;
					}
				}
				if( *ps == CHAR_AMP ) {
					/*若有單獨的 & 字元存在,仍將其視為一般字元,
					  嚴格說來不會有單獨的 & 字元存在，此為容錯. */
					//*ptext = *ps;
					//ptext++;
					ptext2++;
				}
				else {
					ptext1 = ptext2 = ps+1;
				}
			}
			ps++;
		}
		//*ptext = '\0';
		pstrncat(&text, ptext1, ptext2 - ptext1);
	/*-------------------------------------------------------------*/
		/*不檢查關閉標籤的內容。
		  目前允許省略關閉標籤,但嚴格說來是必須有關閉標籤的,
		  故此點僅為容錯,不是常規. */
		if(*ps != '\0' && ps[1] == CHAR_SLASH) {
			while(*ps !=CHAR_GT)
				ps++;
			ps++;
		}

		//printf("[%s]%s\n", key, text);
		pa_text.content = pcontent(&text);
		pa_text.size = plength(&text)+1;
		phashPut(ph, key, &pa_text);
	}
	pdestroy(&text);
	return ph;
}

/*
rowstr = <a>A</a><b>b</b>...
You should construct and init content.
It will cat(not copy) field value to content if the field found else
do nothing for content.

return content.text or NULL
*/
char* row_getfield(const char*rowstr, const char*field_name, string_i*content) {
	int i, field_name_len;
	const char *pkey1, *pkey2, *ptext1, *ptext2, *ps;

	if( !field_name || !content)
		return NULL;
	field_name_len = strlen(field_name);
	ps = rowstr;

	while(*ps != '\0') {
		while(*ps != CHAR_LT && *ps != '\0')
			ps++;
		if(*ps == '\0') 	break;
		else 	ps++;

		pkey1 = pkey2 = ps;
		while(*ps != CHAR_GT && *ps != ' ') {
			ps++;
			pkey2++;
		}
		while(*ps != CHAR_GT)
			ps++;
		ps++;

		if( field_name_len == (pkey2 - pkey1) &&
		    strncmp(field_name, pkey1, field_name_len) == 0) {
			ptext1 = ptext2 = ps;
			while(*ps !=CHAR_LT && *ps != '\0') {
				if( *ps != CHAR_AMP ) {
					ptext2++;
				}
				else {
					for(i=0; entities[i].symb !=NULL; i++) {
						if(strncmp(ps, entities[i].symb, entities[i].len) ==0) {
							content->strncat(content, ptext1, ptext2 - ptext1);
							content->strncat(content, entities_index + i, 1);
							ps = ps + entities[i].len -1;
							break;
						}
					}
					/*若有單獨的 & 字元存在,仍將其視為一般字元,
					  嚴格說來不會有單獨的 & 字元存在，此為容錯. */
					if( *ps == CHAR_AMP )
						ptext2++;
					else
						ptext1 = ptext2 = ps+1;
				}
				ps++;
			}
			content->strncat(content, ptext1, ptext2 - ptext1);
			return content->text(content);
		}

		/*跳過關閉標籤 */
		if(*ps != '\0' && ps[1] == CHAR_SLASH) {
			while(*ps !=CHAR_GT)
				ps++;
			ps++;
		}
	}
	return NULL;
}


static int _apply_hash2row(const char*key, paElement*elm, string_i*row) {
	char keybuf[MAX_FIELDNAME_LEN];
	char *pi, *pt, *ptext1, *ptext2;

	sprintf(keybuf, "<%s>", key);
	row->strcat(row, keybuf);

	ptext1 = ptext2 = pt = elm->content;
	while(*pt) {
		if( (pi = strchr(entities_index, *pt)) !=NULL ) {
			row->strncat(row, ptext1, ptext2 - ptext1);
			row->strcat(row, entities[pi-entities_index].symb);
			ptext1 = ptext2+1;
		}
		ptext2++;
		pt++;
	}
	row->strcat(row, ptext1);

	sprintf(keybuf, "</%s>", key);
	row->strcat(row, keybuf);
	return 0;
}

/*string_i* hash2row(phash_t*ph) {
	pool_t* row;
	row = pconstruct(NULL);
	return (string_i*)row;
}*/
pool_t* hash2row(pool_t*rows, phash_t*ph) {
	if(rows==NULL)
		rows = pconstruct(NULL);
	phashApply(ph, (void*)_apply_hash2row, rows);
	return rows;
}

#if 0
int print_hash(const char*key, paElement*elm, void*none) {
	printf("[%s]%s\n", key, elm->content);
	return 0;
}
int main(int argc, char**argv) {
	phash_t hash;
	pool_t rows;
	if(argc <2)
		return 1;
	printf("%s\n", argv[1]);
	phashconstruct(&hash);

	printf("Convert\n");	
	row2hash(&hash, argv[1]);
	printf("Print\n");
	phashApply(&hash, (void*)print_hash, NULL);

	pconstruct(&rows);
	hash2row(&rows, &hash);
	printf("Row str: %s\n", pcontent(&rows));
	pdestroy(&rows);

	phashdestroy(&hash);
	return 0;
}
#endif
#if 0
int main(int argc, char**argv) {
	char buf[256];
	buffer_t value;
	if(argc <3)
		return 1;
	printf("%s\n", argv[1]);

	buffer_construct(&value, buf);
	buf[0] = '\0';
	printf("Get field: %s\n", argv[2]);	
	printf("value = %s\n", row_getfield(argv[1], argv[2], (string_i*)&value));
	buffer_destroy(&value);
	return 0;
}
#endif