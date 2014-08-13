/* $Id: rex_util.h,v 1.10 2009/09/09 22:38:13 alex Exp $ */
/*******************************************************************************

    rex_util.h

    Regular Expression Utility Definitions.

*******************************************************************************/

#ifndef  REX_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  REX_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Compiled Regular Expressions (Client View).
*******************************************************************************/

typedef  struct  _CompiledRE  *CompiledRE ;


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  rex_util_debug  OCD ("rex_util") ;
					/* Message to pinpoint error during parse. */
extern  char  *rex_error_text  OCD ("rex_util") ;


/*******************************************************************************
    Public functions.
*******************************************************************************/

extern  int  rex_compile P_((const char *expression,
                             CompiledRE *pattern))
    OCD ("rex_util") ;

extern  int  rex_delete P_((CompiledRE pattern))
    OCD ("rex_util") ;

extern  int  rex_dump P_((FILE *outfile, CompiledRE pattern))
    OCD ("rex_util") ;

extern  int  rex_match P_((const char *target,
                           CompiledRE pattern,
                           char **matchStart,
                           int *matchLength,
                           int numSubExps,
                           ...))
    OCD ("rex_util") ;

extern  int  rex_nosex P_((CompiledRE pattern))
    OCD ("rex_util") ;

extern  int  rex_replace P_((const char *source,
                             CompiledRE pattern,
                             const char *replacement,
                             int maxSubstitutions,
                             char **result,
                             int *numSubstitutions))
    OCD ("rex_util") ;

extern  const  char  *rex_wild P_((const char *wildcard))
    OCD ("rex_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
