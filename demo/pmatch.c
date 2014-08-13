/* 
	定義一組容納子樣式字串指標偏移值的陣列 pmatch，其型態為 regmatch_t。並定義 nmatch 以指示陣列大小。
	pmatch 之中儲存的並不是字串指標，而是子樣式字串在來源字串的偏移位址的起始與終止值。
*/

// gcc -o pmatch pmatch.c -lregex -L.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
 
int main()
{
	regex_t preg;
	regmatch_t pmatch[50];
	size_t nmatch = 50;
	int cflags = REG_EXTENDED | REG_ICASE;
	int i, len, rc;
	char buf[16384], reg[4096], str[1024];
 
	while(1) {
		printf("Input exp: ");
		fgets(reg, 4096, stdin);
		if(reg[0] == '\n') break;
		strtok(reg,"\n");
 
		printf("Input str: ");
		fgets(str,1024,stdin);
		if(str[0] == '\n') break;
		strtok(str,"\n");
 
		if( regcomp(&preg, reg, cflags) != 0 ) {
			puts("regex compile error!\n");
			return 1;
		}
 
		rc = regexec(&preg, str, nmatch, pmatch, 0);
		regfree(&preg);
 
		if (rc != 0) {
			printf("no match\n");
			continue;
		}
 
		for (i = 0; i < nmatch && pmatch[i].rm_so >= 0; ++i) {
			len = pmatch[i].rm_eo - pmatch[i].rm_so;
			strncpy(buf, str + pmatch[i].rm_so, len);
			buf[len] = '\0';
			printf("sub pattern %d is %s\n", i, buf);
		}
	}
 
	return 0;
}
