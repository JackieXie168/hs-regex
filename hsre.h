#ifndef HSRE_H
#define HSRE_H

#include <sys/types.h> /* size_t */
#include <stdlib.h> /* malloc, etc. */
#include <stdio.h> /* FILE */
#include <string.h> /* memcpy */
#include <wctype.h>

#define _ANSI_ARGS_(x) x

#define CONST const
#define INLINE inline
#define VOID void
#define __REG_VOID_T void
#define UCHAR(c) ((unsigned char)c)

/* omg hax */
#define ckalloc(n) malloc(n)
#define ckfree(p) free(p)
#define ckrealloc(p,n) realloc(p,n)

/* unicode-safe char funcs raped from Tcl */
typedef wint_t Tcl_UniChar; 

/* from tclUtf.c:
 *
 * Unicode characters less than this value are represented by themselves in
 * UTF-8 strings.
 */
#define UNICODE_SELF 0x80
#define TCL_UTF_MAX 3
#define Tcl_UniCharIsAlnum(ch) iswalnum(ch)
#define Tcl_UniCharIsAlpha(ch) iswalpha(ch)
#define Tcl_UniCharIsDigit(ch) iswdigit(ch)
#define Tcl_UniCharIsSpace(ch) iswspace(ch)
#define Tcl_UniCharToLower(ch) towlower(ch)
#define Tcl_UniCharToUpper(ch) towupper(ch)
#define Tcl_UniCharToTitle(ch) towupper(ch)

INLINE int Tcl_UniCharToUtf(int ch, char *buf);

/* emulated Tcl_DString.. see tclUtil.c for more about the real, 
   non-hax implementation.. see hsre.c for ours */

#define TCL_DSTRING_STATIC_SIZE 200
typedef struct Tcl_DString {
    char *string;		/* Points to beginning of string: either
				 * staticSpace below or a malloced array. */
    int length;			/* Number of non-NULL characters in the
				 * string. */
    int spaceAvl;		/* Total number of bytes available for the
				 * string and its terminating NULL char. */
    char staticSpace[TCL_DSTRING_STATIC_SIZE];
				/* Space to use in common case where string is
				 * small. */
} Tcl_DString;

#define Tcl_DStringLength(dsPtr) ((dsPtr)->length)
#define Tcl_DStringValue(dsPtr) ((dsPtr)->string)
#define Tcl_DStringTrunc Tcl_DStringSetLength

void Tcl_DStringInit(Tcl_DString *dsPtr);
void Tcl_DStringFree(Tcl_DString *dsPtr);
void Tcl_DStringSetLength(Tcl_DString *dsPtr, int length);
char *Tcl_UniCharToUtfDString(CONST Tcl_UniChar *uniStr, int uniLength, Tcl_DString *dsPtr);

#include "regex.h"
#include "regguts.h"

#endif
