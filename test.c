#include <stdlib.h>
#include "hsre.h"

#define N_MATCHES 50

void die(const char* msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

int main(void) {
	const char pat[] = "a(.+)c";
	const char str[] = "a hello c";
	int flags = REG_ADVANCED;
	regex_t re;
	size_t matches_sz;
	regmatch_t* matches;
	int err;
	char errbuf[200];
	int i;

	err = regcomp(&re, pat, flags);
	if (err != REG_OKAY) {
		regerror(err, &re, errbuf, sizeof(errbuf));
		die(errbuf);
	} else {
		printf("Compilation successful\n");
	}
	
	matches_sz = sizeof(regmatch_t) * N_MATCHES;
	matches = malloc(matches_sz);
	memset(matches, 0, matches_sz);
	
	err = regexec(&re, str, N_MATCHES, matches, flags);
	if (err != REG_OKAY) {
		regerror(err, &re, errbuf, sizeof(errbuf));
		die(errbuf);
	} else {
		printf("Execution successful\n");	
		for (i = 0; i < N_MATCHES; i++) {
			if (matches[i].rm_so != -1 && matches[i].rm_eo != 1) 
				printf("%d. {start: %ld, end: %ld}\n", 
					i, matches[i].rm_so, matches[i].rm_eo);
		}
	}
	
	regfree(&re);
	
	exit(EXIT_SUCCESS);
}

