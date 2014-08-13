/* ex_regex.c */
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
 
int main(int argc, char ** argv)
{
	if (argc != 3)
	{
		printf("Usage: %s RegexString Text\n", argv[0]);
		return 1;
	}
 
	const char * pRegexStr = argv[1];
	const char * pText = argv[2];
 
	regex_t oRegex;
	int nErrCode = 0;
	char szErrMsg[1024] = {0};
	size_t unErrMsgLen = 0;
 
	if ((nErrCode = regcomp(&oRegex, pRegexStr, 0)) == 0)
	{
		if ((nErrCode = regexec(&oRegex, pText, 0, NULL, 0)) == 0)
		{
			printf("%s matches %s\n", pText, pRegexStr);
			regfree(&oRegex);
			return 0;
		}
	}
 
	unErrMsgLen = regerror(nErrCode, &oRegex, szErrMsg, sizeof(szErrMsg));
	unErrMsgLen = unErrMsgLen < sizeof(szErrMsg) ? unErrMsgLen : sizeof(szErrMsg) - 1;
	szErrMsg[unErrMsgLen] = '\0';
	printf("ErrMsg: %s\n", szErrMsg);
 
	regfree(&oRegex);
	return 1;
}
