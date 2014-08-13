/** 
	 @file			regex_example.c
	 @author		Mitch Richling <http://www.mitchr.me/>
	 @Copyright Copyright 1994 by Mitch Richling.	All rights reserved.
	 @brief		 UNIX regex tools@EOL
	 @Keywords	UNIX regular expressions regex
	 @Std			 ISOC POSIX.2 (IEEE Std 103.2) BSD4.3

							This is an example program intended to illustrate
							very basic use of regular expressions.
	
							Grumpy programmer note: IEEE Std 1003.2, generally
							referred to as 'POSIX.2' is a bit vague regarding
							several details like how back references work.	It also
							has a couple of errors (like how a single ')' is treated
							in a regular expression.	Because of this, most actual
							implementations of the standard will have several minor
							inconsistencies that one must watch out for.	My best
							advice is to "read the man page" on the platforms you
							wish to run on and to avoid exotic things.	For example,
							avoid things like the BSD REG_NOSPEC and REG_PEND
							options.	Another option is to simply carry your
							favorite regular expression library with you.	For
							example, the BOOST library has a very nice regex class
							for C++ programmers.	PCRE is probably the most popular
							alternative, FOSS regular expression library available.

	 @Tested		
							- Solaris 2.8
							- MacOS X.2
							- Linux (RH 7.3)
 */

#include <sys/types.h>					/* UNIX types			POSIX */
#include <regex.h>							/* Regular Exp		 POSIX */
#include <stdio.h>							/* I/O lib				 ISOC	*/
#include <string.h>						 /* Strings				 ISOC	*/
#include <stdlib.h>						 /* Standard Lib		ISOC	*/

#define MAX_SUB_EXPR_CNT 256
#define MAX_SUB_EXPR_LEN 256
#define MAX_ERR_STR_LEN	256

int main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
	int i;																/* Loop variable.													*/
	char p[MAX_SUB_EXPR_LEN];						 /* For string manipulation								 */
	regex_t aCmpRegex;										/* Pointer to our compiled regex					 */
	char *aStrRegex;											/* Pointer to the string holding the regex */
	regmatch_t pMatch[MAX_SUB_EXPR_CNT];	/* Hold partial matches.									 */
	char **aLineToMatch;									/* Holds each line that we wish to match	 */
	int result;													 /* Return from regcomp() and regexec()		 */
	char outMsgBuf[MAX_ERR_STR_LEN];			/* Holds error messages from regerror()		*/
	char *testStrings[] = { "This should match... hello",
													"This could match... hello!",
													"More than one hello.. hello",
													"No chance of a match...",
													NULL};

	/* use aStrRegex for readability. */
	aStrRegex = "(.*)(hello)+";
	printf("Regex to use: %s\n", aStrRegex);

	/* Compile the regex */
	if( (result = regcomp(&aCmpRegex, aStrRegex, REG_EXTENDED)) ) {
		printf("Error compiling regex(%d).\n", result);
		regerror(result, &aCmpRegex, outMsgBuf, sizeof(outMsgBuf));
		printf("Error msg: %s\n", outMsgBuf);
		exit(1);
	} /* end if */

	/*	Possible last argument to regcomp (||'ed together):
				REG_EXTENDED	Use extended regular expressions
				REG_BASIC		 Use basic regular expressions
				REG_NOSPEC		Special character support off (Not POSIX.2)
				REG_ICASE		 Ignore upper/lower case distinctions
				REG_NOSUB		 No sub-strings (just check for match/no match)
				REG_NEWLINE	 Compile for newline-sensitive matching
				REG_PEND			Specify alternate string ending (Not POSIX.2) */


	/* Apply our regular expression to some strings. */
	for(aLineToMatch=testStrings; *aLineToMatch != NULL; aLineToMatch++) {
		printf("String: %s\n", *aLineToMatch);
		printf("				%s\n", "0123456789012345678901234567890123456789");
		printf("				%s\n", "0				 1				 2				 3");
		/* compare and check result (MAX_SUB_EXPR_CNT max sub-expressions).*/
		if( !(result = regexec(&aCmpRegex, *aLineToMatch, MAX_SUB_EXPR_CNT, pMatch, 0)) ) {
			/* Last argument to regexec (||'ed together):
				 REG_NOTBOL		Start of the string is NOT the start of a line
				 REG_NOTEOL		$ shouldn't match end of string (gotta have a newline)
				 REG_STARTEND	Not POSIX.2 */
			printf("Result: We have a match!\n");
			for(i=0;i<=(int)aCmpRegex.re_nsub;i++) {
				printf("Match(%2d/%2d): (%2d,%2d): ", 
							 i, 
							 (int)(aCmpRegex.re_nsub), 
							 (int)(pMatch[i].rm_so), 
							 (int)(pMatch[i].rm_eo));

					if( (pMatch[i].rm_so >= 0) && (pMatch[i].rm_eo >= 1) && 
							(pMatch[i].rm_so != pMatch[i].rm_eo) ) {
						strncpy(p, &((*aLineToMatch)[pMatch[i].rm_so]), pMatch[i].rm_eo-pMatch[i].rm_so);
						p[pMatch[i].rm_eo-pMatch[i].rm_so] = '\0';
						printf("'%s'", p);
					} /* end if */
					printf("\n");
			} /* end for */
			printf("\n");
		} else {
			switch(result) {
				case REG_NOMATCH	 	: printf("String did not match the pattern\n");				break;
				//Some typical return codes:
				case REG_BADPAT			: printf("invalid regular expression\n");							break;
				case REG_ECOLLATE		: printf("invalid collating element\n");							break;
				case REG_ECTYPE			: printf("invalid character class\n");								break;
				case REG_EESCAPE	 	: printf("`\' applied to unescapable character\n");			break;
				case REG_ESUBREG	 	: printf("invalid backreference number\n");					break;
				case REG_EBRACK			: printf("brackets `[ ]' not balanced\n");						break;
				case REG_EPAREN			: printf("parentheses `( )' not balanced\n");					break;
				case REG_EBRACE			: printf("braces `{ }' not balanced\n");							break;
				case REG_BADBR		 	: printf("invalid repetition count(s) in `{ }'\n");				break;
				case REG_ERANGE			: printf("invalid character range in `[ ]'\n");					break;
				case REG_ESPACE			: printf("Ran out of memory\n");									break;
				case REG_BADRPT			: printf("`?', `*', or `+' operand invalid\n");					break;
				case REG_EMPTY		 	: printf("empty (sub)expression\n");								break;
				case REG_ASSERT			: printf("can't happen - you found a bug\n");				break;
				case REG_INVARG			: printf("A bad option was passed\n");							break;
				//case REG_ILLSEQ			: printf("illegal byte sequence\n");									break;
				default								: printf("Unknown error\n");											break;
			} /* end switch */
			regerror(result, &aCmpRegex, outMsgBuf, sizeof(outMsgBuf));
			printf("Result: Error msg: %s\n\n", outMsgBuf);
		} /* end if/else */
	} /* end for */
	
	/* Free up resources for the regular expression */
	regfree(&aCmpRegex);

	exit(0);
} /* end func main */
