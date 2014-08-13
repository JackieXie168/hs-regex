%{
/* $Id: rex_util_y.y,v 1.13 2011/07/18 17:48:17 alex Exp $ */
/*******************************************************************************

Procedure:

    yyparse ()


Author:    Alex Measday


Purpose:

    Function YYPARSE is a YACC-generated routine that parses a regular
    expression (RE) and builds a non-deterministic finite state machine
    that will recognize the RE.  The grammar for regular expressions
    is derived from the grammar presented by Robert Sedgewick in his
    book, ALGORITHMS (Chapter 21: Parsing).


    Invocation:

        status = yyparse () ;

    where

        <status>
            returns the status of parsing the input, zero if no errors occurred
            and a non-zero value otherwise.


    Public Variables:

        REX_UTIL_ERROR_TEXT - is a (CHAR *) pointer to a short error message
            explaining why YYPARSE failed, if it does.


Development History:
                                               Description
Author         Change ID       Build/Date      of Change
-------------  --------------  --------------  ---------------------
A. Measday     Port from TPOCC Release 6.1 to XSAR Release 0
    Added conditional inclusion of VMS-specific header files.  Eliminated
    external function declarations, since the required function prototypes
    are available in the "#include"d system and TPOCC header files.

A. Measday     Port from TPOCC Release 7 to XSAR Pre-Release 1
    Converted function declarations to ANSI C and added function prototypes
    for static, internal functions.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <ctype.h>			/* Character classification macros. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "rex_util.h"			/* Regular expression definitions. */
#include  "rex_internals.h"		/* Internal definitions. */

#define  NEW_STATE \
  { if (allocate_state ()) { \
        rex_error_text = "error increasing size of state list" ; \
        return (errno) ; \
    } \
  }

#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  rex_util_debug

#define  display_production(left, right) \
    LGI "%s ::== %s\n", left, right) ;

#define  display_token(name, text) \
    { LGI "-- Token %s = \"%c\"\n", name, text) ; }

#define  beginning_of_input()  (rtx->scan == rtx->expression)
#define  input()  ((*rtx->scan == '\0') ? 0 : *rtx->scan++)
#define  unput(c)  (((rtx->scan == rtx->expression) || (c == '\0')) ? -1 : (rtx->scan--, 0))

/*******************************************************************************

    Internal Non-Local Variables.  These flags give the parser (YYPARSE) some
        control over how the lexer (YYLEX) interprets characters in the input
        string.  The flags are automatically reset when YYLEX is positioned at
        the beginning of the input string.

        YYPARSE_AFTER_BRACKET - controls the interpretation of characters
            within a character class specification (i.e., "[...]").  A value
            of zero indicates that the parser is not in the midst of a bracket
            expression.  Once the left bracket is encountered, YYPARSE sets
            this variable to 1; as each character in the bracket is scanned,
            YYPARSE_AFTER_BRACKET is incremented.

        YYPARSE_EXPECTING_ASSIGNMENT - controls the interpretation of a '$'
            that follows a right parenthesis: "...)$...".  YYPARSE sets this
            variable to 1 after reading the right parenthesis.  If YYLEX then
            encounters a '$' followed by a digit ('0' - '9'), it returns the
            '$' as an ASSIGNMENT token; otherwise, the '$' is returned as an
            EOL_ANCHOR token.

        YYPARSE_HYPHEN_AS_LITERAL - controls the interpretation of hyphens
            within a bracketed character class expression.

*******************************************************************************/

static  int  yyparse_after_bracket  OCD ("rex_util")  = 0 ;
static  int  yyparse_expecting_assignment  OCD ("rex_util")  = 0 ;
static  int  yyparse_hyphen_as_literal  OCD ("rex_util")  = 0 ;


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  int  yyerror (
#    if PROTOTYPES
        char  *s
#    endif
    )  OCD ("rex_util") ;

static  int  yylex (
#    if PROTOTYPES && !defined(__cplusplus)
        void
#    endif
    )  OCD ("rex_util") ;

static  int  allocate_state (
#    if PROTOTYPES && !defined(__cplusplus)
        void
#    endif
    )  OCD ("rex_util") ;

static  int  first_char_of (
#    if PROTOTYPES
        int  state,
        cs_set  *first_set
#    endif
    )  OCD ("rex_util") ;

static  int  last_of (
#    if PROTOTYPES
        int  state
#    endif
    )  OCD ("rex_util") ;

static  int  longest_path (
#    if PROTOTYPES
        int  state
#    endif
    )  OCD ("rex_util") ;

static  int  shortest_path (
#    if PROTOTYPES
        int  state
#    endif
    )  OCD ("rex_util") ;
%}


/* Token definitions. */

%token  ANY_CHARACTER
%token  ASSIGNMENT
%token  BOL_ANCHOR
%token  CARAT
%token  COMMA
%token  CTYPE
%token  DIGIT
%token  EOL_ANCHOR
%token  _ERROR
%token  HYPHEN
%token  LEFT_BRACE
%token  LEFT_BRACKET
%token  LEFT_PAREN
%token  ONE_OR_MORE
%token  OR
%token  RIGHT_BRACE
%token  RIGHT_BRACKET
%token  RIGHT_PAREN
%token  SINGLE_CHARACTER
%token  ZERO_OR_MORE
%token  ZERO_OR_ONE

/* Operator precedence. */

%left  OR
%right  ONE_OR_MORE  ZERO_OR_MORE  ZERO_OR_ONE

/* Start symbol of the regular expression grammar. */

%start  complete_re

%{
/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

    static  char  buffer[8] ;
    static  cs_set  char_class ;
    static  int  i ;

/* Character classification names.  The ordering of these names is important
   to YYPARSE; see the "character_class => CTYPE" rule before making any
   changes. */

    static  char  *class_name[]  OCD ("rex_util")  = {
        "alnum",
        "alpha",
        "cntrl",
        "digit",
        "graph",
        "lower",
        "print",
        "punct",
        "space",
        "upper",
        "xdigit",
        NULL
    } ;

%}

%%

/*******************************************************************************
  Regular Expression Grammar
*******************************************************************************/


complete_re :
        regular_expression
          { display_production ("complete_re", "regular_expression") ;
            /* The initial state equals the first state in RE.  Add a final
               state and link the last state in RE to the final state. */
            rtx->start_state = $1 ;
            NEW_STATE ;
            rtx->state_list[last_of ($1)].next1 = rtx->num_states ;
            rtx->state_list[rtx->num_states].type = final ;
            rtx->state_list[rtx->num_states].x.match_char = ' ' ;
            rtx->num_states++ ;
            /* Estimate the length of the longest path through the complete
               RE.  The length is used to size the stack in the iterative
               version of REX_SEARCH. */
            for (i = 0 ;  i < rtx->num_states ;  i++)
                rtx->state_list[i].z.visited = 0 ;
            rtx->longest_path = longest_path ($1) ;
            /* Compute the set of first characters that may appear at the
               beginning of a target string matched by this RE.  This set
               allows REX_MATCH to avoid calling REX_SEARCH when the first
               character of the target string is not in the set of first
               characters. */
            CS_ZERO (&rtx->first_set) ;
            for (i = 0 ;  i < rtx->num_states ;  i++)
                rtx->state_list[i].z.visited = 0 ;
            first_char_of (rtx->start_state, &rtx->first_set) ;
          }
    ;


regular_expression :
        /* empty */
          { display_production ("regular_expression", "<empty>") ;
            /* Add an EMPTY state. */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = empty ;
            rtx->state_list[rtx->num_states].x.match_char = ' ' ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   term
          { display_production ("regular_expression", "term") ;  $$ = $1 ; }
    |   regular_expression  OR  regular_expression
          { display_production ("regular_expression", "regular_expression | regular_expression") ;
            /* Add an alternation state E1 and an empty state E2.  Link E1 to
               the first state in RE1 and the first state in RE2.  Link the
               last state in RE1 to E2; do the same for the last state in RE2.
               (The first state in "RE|RE" is E1.) */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = alternation ;
            rtx->state_list[rtx->num_states].x.match_char = ' ' ;
            rtx->state_list[rtx->num_states].next1 = $1 ;
            rtx->state_list[rtx->num_states].next2 = $3 ;
            $$ = rtx->num_states++ ;
            NEW_STATE ;
            rtx->state_list[last_of ($1)].next1 = rtx->num_states ;
            rtx->state_list[last_of ($3)].next1 = rtx->num_states ;
            rtx->state_list[rtx->num_states].type = empty ;
            rtx->state_list[rtx->num_states].x.match_char = ' ' ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            rtx->num_states++ ;
          }
    ;


term :
        factor
          { display_production ("term", "factor") ;  $$ = $1 ; }
    |   factor  term
          { display_production ("term", "factor term") ;
            /* Link the last state in RE1 to the first state in RE2.  (The
               first state in "RE1 RE2" is the first state in RE1.) */
            rtx->state_list[last_of ($1)].next1 = $2 ;
            $$ = $1 ;
          }
    ;


factor :
        BOL_ANCHOR
          { display_production ("factor", "^") ;
            /* Add an anchor state and set its match character to "^". */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = anchor ;
            rtx->state_list[rtx->num_states].x.match_char = '^' ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   EOL_ANCHOR
          { display_production ("factor", "$") ;
            /* Add an anchor state and set its match character to "$". */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = anchor ;
            rtx->state_list[rtx->num_states].x.match_char = '$' ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   factor  LEFT_BRACE  number  upper_bound  RIGHT_BRACE
          { display_production ("factor", "factor{[min][,[max]]}") ;
            /* Check for nested closures that might cause the matching
               algorithm to endlessly loop without consuming any input
               from the target string; e.g., "(a*){0,10}".  Actually,
               the maximum field puts a cap on the number of loops, but
               we'll disallow it anyway. */
            for (i = 0 ;  i < rtx->num_states ;  i++)
                rtx->state_list[i].z.visited = 0 ;
            i = shortest_path ($1) ;
            LGI "(yyparse) Shortest path from state %d = %d\n", $1, i) ;
            if (i <= 0) {
                SET_ERRNO (EINVAL) ;  rex_error_text = "nested empty closure" ;
                return (errno) ;
            }
            /* Add a closure state and set its minimum and maximum fields.
               Link the last state in RE to the closure state; link the closure
               state backwards to the first state in RE.  (The first state in
               "RE{min,max}" is the closure state.) */
            NEW_STATE ;
            rtx->state_list[last_of ($1)].next1 = rtx->num_states ;
            rtx->state_list[rtx->num_states].type = closure ;
            if ($3 < 0)  $3 = 0 ;
            rtx->state_list[rtx->num_states].x.min_closure = $3 ;
            if ($4 < 0)  $4 = $3 ;
            rtx->state_list[rtx->num_states].y.max_closure = $4 ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = $1 ;
            $$ = rtx->num_states++ ;
          }
    |   LEFT_PAREN  regular_expression  RIGHT_PAREN
          { yyparse_expecting_assignment = -1 ; }
        assignment
          { display_production ("factor", "(regular_expression) <assignment>") ;
            /* If an assignment clause ("$<digit>") was specified, add two
               parenthesis states, P1 and P2.  Link P1 to the first state
               in RE; link the last state in RE to P2.  Store the argument
               index (<digit>) in P1 and store the index of P1 in P2.  If an
               assignment clause was NOT specified, nothing needs to be done.
               (The first state in "(RE)" is the first state in RE; the first
                state in "(RE)$n" is P1.) */
            if ($5 < 0) {				/* "(RE)" */
                $$ = $2 ;
            } else {					/* "(RE)$n" */
                NEW_STATE ;
                rtx->state_list[rtx->num_states].type = left_paren ;	/* P1 */
                rtx->state_list[rtx->num_states].x.subexp_index = $5 ;
                rtx->state_list[rtx->num_states].y.subexp_state =
                    rtx->num_states + 1 ;
                rtx->state_list[rtx->num_states].next1 = $2 ;
                rtx->state_list[rtx->num_states].next2 = -1 ;
                if (rtx->num_args < ($5+1))  rtx->num_args = $5 + 1 ;
                $$ = rtx->num_states++ ;
                NEW_STATE ;
                rtx->state_list[last_of ($2)].next1 = rtx->num_states ;
                rtx->state_list[rtx->num_states].type = right_paren ;	/* P2 */
                rtx->state_list[rtx->num_states].x.subexp_index = $5 ;
                rtx->state_list[rtx->num_states].y.subexp_state = $$ ;
                rtx->state_list[rtx->num_states].next1 = -1 ;
                rtx->state_list[rtx->num_states].next2 = -1 ;
                rtx->num_states++ ;
            }
          }
    |   factor  ZERO_OR_MORE
          { display_production ("factor", "factor*") ;
            /* Check for nested closures that might cause the matching
               algorithm to endlessly loop without consuming any input
               from the target string; e.g., "(a*)*". */
            for (i = 0 ;  i < rtx->num_states ;  i++)
                rtx->state_list[i].z.visited = 0 ;
            i = shortest_path ($1) ;
            LGI "(yyparse) Shortest path from state %d = %d\n", $1, i) ;
            if (i <= 0) {
                SET_ERRNO (EINVAL) ;  rex_error_text = "nested empty closure" ;
                return (errno) ;
            }
            /* Add a closure state.  Link the last state in RE to the closure
               state; link the closure state backwards to the first state in
               RE.  (The first state in "RE*" is the closure state.) */
            NEW_STATE ;
            rtx->state_list[last_of ($1)].next1 = rtx->num_states ;
            rtx->state_list[rtx->num_states].type = closure ;
            rtx->state_list[rtx->num_states].x.min_closure = 0 ;
            rtx->state_list[rtx->num_states].y.max_closure = INT_MAX ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = $1 ;
            $$ = rtx->num_states++ ;
          }
    |   factor  ONE_OR_MORE
          { display_production ("factor", "factor+") ;
            /* Check for nested closures that might cause the matching
               algorithm to endlessly loop without consuming any input
               from the target string; e.g., "(a*)+". */
            for (i = 0 ;  i < rtx->num_states ;  i++)
                rtx->state_list[i].z.visited = 0 ;
            i = shortest_path ($1) ;
            LGI "(yyparse) Shortest path from state %d = %d\n", $1, i) ;
            if (i <= 0) {
                SET_ERRNO (EINVAL) ;  rex_error_text = "nested empty closure" ;
                return (errno) ;
            }
            /* Add a closure state.  Link the last state in RE to the closure
               state; link the closure state backwards to the first state in
               RE.  (The first state in "RE+" is the closure state.) */
            NEW_STATE ;
            rtx->state_list[last_of ($1)].next1 = rtx->num_states ;
            rtx->state_list[rtx->num_states].type = closure ;
            rtx->state_list[rtx->num_states].x.min_closure = 1 ;
            rtx->state_list[rtx->num_states].y.max_closure = INT_MAX ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = $1 ;
            $$ = rtx->num_states++ ;
          }
    |   factor  ZERO_OR_ONE
          { display_production ("factor", "factor?") ;
            /* Check for nested closures.  Even though nested closures are
               not a major threat in a zero-or-one closure, we check anyway,
               just to be consistent with the other forms of closure. */
            for (i = 0 ;  i < rtx->num_states ;  i++)
                rtx->state_list[i].z.visited = 0 ;
            i = shortest_path ($1) ;
            LGI "(yyparse) Shortest path from state %d = %d\n", $1, i) ;
            if (i <= 0) {
                SET_ERRNO (EINVAL) ;  rex_error_text = "nested empty closure" ;
                return (errno) ;
            }
            /* Add a closure state.  Link the last state in RE to the closure
               state; link the closure state backwards to the first state in
               RE.  (The first state in "RE?" is the closure state.) */
            NEW_STATE ;
            rtx->state_list[last_of ($1)].next1 = rtx->num_states ;
            rtx->state_list[rtx->num_states].type = closure ;
            rtx->state_list[rtx->num_states].x.min_closure = 0 ;
            rtx->state_list[rtx->num_states].y.max_closure = 1 ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = $1 ;
            $$ = rtx->num_states++ ;
          }
    |   LEFT_BRACKET
          { CS_ZERO (&char_class) ;
            yyparse_after_bracket = 1 ;
            yyparse_hyphen_as_literal = 0 ;
          }
        complement  character_classes  RIGHT_BRACKET
          { display_production ("factor", "[character_classes]") ;
            yyparse_after_bracket = 0 ;
            if ($3) {
                for (i = 0 ;  i < CS_SETSIZE ;  i++) {
                    if (CS_ISSET (i, &char_class))
                        CS_CLR (i, &char_class) ;
                    else
                        CS_SET (i, &char_class) ;
                }
            }
            LGI "Character Class:\n") ;
            if (I_DEFAULT_GUARD) {
                rex_dump_class (stdout, "    Matches: ", &char_class) ;
            }
            /* Add a multi-character state for the character class. */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = match ;
            rtx->state_list[rtx->num_states].x.match_char = 0 ;
            rtx->state_list[rtx->num_states].y.match_charset =
                (cs_set *) malloc (sizeof (cs_set)) ;
            if (rtx->state_list[rtx->num_states].y.match_charset == NULL) {
                LGE "(yyparse) Error allocating character class set.\nmalloc: ") ;
                rex_error_text = "error allocating character class set" ;
                return (errno) ;
            }
            CS_ZERO (rtx->state_list[rtx->num_states].y.match_charset) ;
            for (i = 0 ;  i < CS_SETSIZE ;  i++)
                if (CS_ISSET (i, &char_class))
                    CS_SET (i, rtx->state_list[rtx->num_states].y.match_charset) ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   SINGLE_CHARACTER
          { display_production ("factor", "SINGLE_CHARACTER") ;
            /* Add a one-character state for the character. */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = match ;
            rtx->state_list[rtx->num_states].x.match_char = $1 ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   COMMA
          { display_production ("factor", "COMMA") ;
            /* Add a one-character state for ",". */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = match ;
            rtx->state_list[rtx->num_states].x.match_char = $1 ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   DIGIT
          { display_production ("factor", "DIGIT") ;
            /* Add a one-character state for the digit. */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = match ;
            rtx->state_list[rtx->num_states].x.match_char = $1 ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    |   ANY_CHARACTER
          { display_production ("factor", "ANY_CHARACTER") ;
            /* Add a one-character state for any-character-matches. */
            NEW_STATE ;
            rtx->state_list[rtx->num_states].type = match ;
            rtx->state_list[rtx->num_states].x.match_char = -1 ;
            rtx->state_list[rtx->num_states].next1 = -1 ;
            rtx->state_list[rtx->num_states].next2 = -1 ;
            $$ = rtx->num_states++ ;
          }
    ;


assignment :
        /* empty */
          { display_production ("assignment", "<empty>") ;  $$ = -1 ; }
    |   ASSIGNMENT  DIGIT
          { display_production ("assignment", "$<0-9>") ;
            sprintf (buffer, "%c", $2) ;  $$ = atoi (buffer) ;
          }
    ;


complement :
        /* empty */
          { display_production ("complement", "<empty>") ;  $$ = 0 ; }
    |   CARAT
          { display_production ("complement", "^") ;  $$ = -1 ; }
    ;


number :
        /* empty */
          { display_production ("number", "<empty>") ;  $$ = -1 ; }
    |   digits
          { display_production ("number", "digits") ;  $$ = $1 ; }
    ;


upper_bound :
        /* empty */
          { display_production ("upper_bound", "<empty>") ;
            $$ = -1 ;			/* Upper bound = lower bound. */
          }
    |   COMMA  number
          { display_production ("lower_bound", "digits") ;
            if ($2 < 0)
                $$ = INT_MAX ;		/* Upper bound = infinity. */
            else
                $$ = $2 ;		/* Normal upper bound. */
          }
    ;


digits :
        DIGIT
          { display_production ("digits", "DIGIT") ;
            sprintf (buffer, "%c", $1) ;  $$ = atoi (buffer) ;
          }
    |   digits  DIGIT
          { display_production ("digits", "digits DIGIT") ;
            sprintf (buffer, "%c", $2) ;  $$ = ($1 * 10) + atoi (buffer) ;
          }
    ;


character_classes :
        character_class
          { display_production ("character_classes", "character_class") ; }
    |   character_classes  character_class
          { display_production ("character_classes", "character_classes character_class") ; }
    ;


character_class :
        character
          { display_production ("character_class", "character") ;
            CS_SET ($1, &char_class) ;
          }
    |   character  HYPHEN
          { yyparse_hyphen_as_literal = 1 ; }
        character
          { display_production ("character_class", "character-character") ;
            yyparse_hyphen_as_literal = 0 ;
            if ($1 > $4) {
                LGI "(yyparse) Start character exceeds end character in character class range \"%c-%c\".\n", $1, $4) ;
                SET_ERRNO (EINVAL) ;
                rex_error_text = "inverted character class range" ;
                return (errno) ;
            }
            for (i = $1 ;  i <= $4 ;  i++)
                CS_SET (i, &char_class) ;
          }
    |   CTYPE
          { display_production ("character_class", ":<class>:") ;
            for (i = 0 ;  i < CS_SETSIZE ;  i++) {
                switch ($1) {
                case  0:  if (isalnum (i))  CS_SET (i, &char_class) ;  break ;
                case  1:  if (isalpha (i))  CS_SET (i, &char_class) ;  break ;
                case  2:  if (iscntrl (i))  CS_SET (i, &char_class) ;  break ;
                case  3:  if (isdigit (i))  CS_SET (i, &char_class) ;  break ;
                case  4:  if (isgraph (i))  CS_SET (i, &char_class) ;  break ;
                case  5:  if (islower (i))  CS_SET (i, &char_class) ;  break ;
                case  6:  if (isprint (i))  CS_SET (i, &char_class) ;  break ;
                case  7:  if (ispunct (i))  CS_SET (i, &char_class) ;  break ;
                case  8:  if (isspace (i))  CS_SET (i, &char_class) ;  break ;
                case  9:  if (isupper (i))  CS_SET (i, &char_class) ;  break ;
                case 10:  if (isxdigit (i)) CS_SET (i, &char_class) ;  break ;
                }
            }
          }
    ;


character :
        SINGLE_CHARACTER
          { display_production ("character", "SINGLE_CHARACTER") ;  $$ = $1 ; }
    ;


%%

/*!*****************************************************************************
    Function YYERROR is invoked automatically by YYPARSE when an error is
    detected.  YYERROR simply prints out the error message passed to it by
    YYPARSE.
*******************************************************************************/

static  int  yyerror (

#    if PROTOTYPES
        char  *s)
#    else
        s)

        char  *s ;
#    endif

{

    LGI "(yyparse) %s\n", s) ;
    SET_ERRNO (EINVAL) ;  rex_error_text = s ;
    return (errno) ;

}

/*!*****************************************************************************
    Function YYLEX returns the next token of input.  This function is normally
    generated by LEX, but, for this parser, it's simple enough to do by hand.
*******************************************************************************/


static  int  yylex (

#    if PROTOTYPES && !defined(__cplusplus)
        void)
#    else
        )
#    endif

{    /* Local variables. */
    char  buffer[8] ;
    unsigned  char  c ;
    unsigned  int  i ;




    if (beginning_of_input ()) {		/* Reset lexical flags. */
        yyparse_after_bracket = 0 ;
        yyparse_expecting_assignment = 0 ;
        yyparse_hyphen_as_literal = 0 ;
    }

    c = (unsigned char) input () ;  yylval = c ;

/* At the end of the string, return an end marker. */

    if (c == '\0') {
        display_token ("END_MARKER", '0') ;  return (0) ;
    }

/* Within a bracket expression ("[...]"), most characters are interpreted
   literally.  When the left bracket is encountered, YYPARSE sets AFTER_BRACKET
   to 1.  A "^" immediately after the left bracket is returned as the character
   class complement indicator and AFTER_BRACKET is set to -1.  A right bracket
   or hyphen immediately after the left bracket (AFTER_BRACKET == 1) or the
   complement character (AFTER_BRACKET == -1) is returned as a literal
   character.  A hyphen following a hyphen ("[a--]") or preceding the right
   bracket ("[a-]") is returned as a literal character.  A colon (":")
   immediately after the left bracket or the complement character is the
   start of a character class name specification (":<class>:"); the class
   name is extracted, looked up in the class name table, and its index is
   returned to YYPARSE.  "\<c>" constructs are handled by the non-bracket
   expression SWITCH statement further down below. */

    if (yyparse_after_bracket) {
        switch (c) {
        case '^':
            if (yyparse_after_bracket == 1) {
                yyparse_after_bracket = -1 ;
                display_token ("CARAT", c) ;  return (CARAT) ;
            }
            if (yyparse_after_bracket == -1)  yyparse_after_bracket = 1 ;
            yyparse_after_bracket++ ;
            display_token ("SINGLE_CHARACTER", c) ;  return (SINGLE_CHARACTER) ;
        case ']':
            if (yyparse_after_bracket++ > 1) {
                display_token ("RIGHT_BRACKET", c) ;  return (RIGHT_BRACKET) ;
            }
            yyparse_after_bracket = 2 ;
            display_token ("SINGLE_CHARACTER", c) ;  return (SINGLE_CHARACTER) ;
        case '-':				/* Is "-" followed by a "]"? */
            c = (unsigned char) input () ;  unput (c) ;
            if ((yyparse_after_bracket++ > 1) &&
                (!yyparse_hyphen_as_literal) && (c != ']')) {
                display_token ("HYPHEN", c) ;  return (HYPHEN) ;
            }
            yyparse_after_bracket = 2 ;
            display_token ("SINGLE_CHARACTER", '-') ;
            return (SINGLE_CHARACTER) ;
        case ':':
            if (yyparse_after_bracket++ > 1) {
                display_token ("SINGLE_CHARACTER", c) ;  return (SINGLE_CHARACTER) ;
            }
            yyparse_after_bracket = 2 ;
				/* Extract class name from ":<class>:". */
            for (i = 0 ;  (c = (unsigned char) input ()) && (c != ':') ;  )
                if (i < (sizeof buffer - 1))  buffer[i++] = c ;
            buffer[i] = '\0' ;  strToLower (buffer, -1) ;
				/* Lookup name in class name table. */
            for (i = 0 ;  class_name[i] != NULL ;  i++)
                if (strcmp (buffer, class_name[i]) == 0)  break ;
            yylval = i ;  return (CTYPE) ;
        case '\\':
            if (yyparse_after_bracket == -1)  yyparse_after_bracket = 1 ;
            yyparse_after_bracket++ ;  yyparse_after_bracket++ ;
            break ;
        default:
            if (yyparse_after_bracket == -1)  yyparse_after_bracket = 1 ;
            yyparse_after_bracket++ ;
            display_token ("SINGLE_CHARACTER", c) ;  return (SINGLE_CHARACTER) ;
        }
    }


/* Outside of a bracket expression, characters receive the standard regular
   expression interpretation. */

    switch (c) {
    case '.':
        display_token ("ANY_CHARACTER", c) ;  return (ANY_CHARACTER) ;
    case '^':
        display_token ("BOL_ANCHOR", c) ;  return (BOL_ANCHOR) ;
    case ',':
        display_token ("COMMA", c) ;  return (COMMA) ;
    case '$':
        c = (unsigned char) input () ;  unput (c) ;
        if (yyparse_expecting_assignment && isdigit (c)) {
            yyparse_expecting_assignment = 0 ;
            display_token ("ASSIGNMENT", '$') ;  return (ASSIGNMENT) ;
        } else {
            yyparse_expecting_assignment = 0 ;
            display_token ("EOL_ANCHOR", '$') ;  return (EOL_ANCHOR) ;
        }
    case '|':
        display_token ("OR", c) ;  return (OR) ;
    case '*':
        display_token ("ZERO_OR_MORE", c) ;  return (ZERO_OR_MORE) ;
    case '+':
        display_token ("ONE_OR_MORE", c) ;  return (ONE_OR_MORE) ;
    case '?':
        display_token ("ZERO_OR_ONE", c) ;  return (ZERO_OR_ONE) ;
    case '(':
        display_token ("LEFT_PAREN", c) ;  return (LEFT_PAREN) ;
    case ')':
        display_token ("RIGHT_PAREN", c) ;  return (RIGHT_PAREN) ;
    case '{':
        display_token ("LEFT_BRACE", c) ;  return (LEFT_BRACE) ;
    case '}':
        display_token ("RIGHT_BRACE", c) ;  return (RIGHT_BRACE) ;
    case '[':
        display_token ("LEFT_BRACKET", c) ;  return (LEFT_BRACKET) ;
    case '\\':
        c = input () ;  yylval = c ;
        switch (c) {
        case 'n':  yylval = '\n' ;  break ;
        case 'r':  yylval = '\n' ;  break ;
        case 't':  yylval = '\t' ;  break ;
        default:   break ;
        }
        if (c == '\0') {
            unput (c) ;  display_token ("_ERROR", '0') ;  return (_ERROR) ;
        } else {
            display_token ("SINGLE_CHARACTER", c) ;  return (SINGLE_CHARACTER) ;
        }
    default:
        if (isdigit (c)) {
            display_token ("DIGIT", c) ;  return (DIGIT) ;
        } else {
            display_token ("SINGLE_CHARACTER", c) ;  return (SINGLE_CHARACTER) ;
        }
    }

}

/*!*****************************************************************************
    Function ALLOCATE_STATE simply checks if there is enough room to add a new
    state to the state list.  If there is enough room, nothing is done; the
    calling routine is responsible for incrementing the NUM_STATES pointer.
    If there is not enough room, the size of the state list is increased.
    Zero is returned if the calling routine can now add a state; ERRNO is
    returned if ALLOCATE_STATE could not resize the state list.
*******************************************************************************/

#define  INCREMENT  4

static  int  allocate_state (

#    if PROTOTYPES && !defined(__cplusplus)
        void)
#    else
        )
#    endif

{    /* Local variables. */
    char  *s ;
    int  size ;



    if (rtx->num_states >= rtx->max_states) {
        size = (rtx->max_states + INCREMENT) * sizeof (StateNode) ;
        if (rtx->state_list == NULL)
            s = (char *) malloc (size) ;
        else
            s = (char *) realloc ((char *) rtx->state_list, size) ;
        if (s == NULL) {
            LGE "(allocate_state) Error reallocating the state list.\nrealloc: ") ;
            return (errno) ;
        }
        rtx->state_list = (StateNode *) s ;
        rtx->max_states = rtx->max_states + INCREMENT ;
    }

    return (0) ;

}

/*!*****************************************************************************
    Function FIRST_CHAR_OF computes the set of characters that may appear at
    the beginning of a string that could be matched by a regular expression.
    For example, the set of first characters for "abc" is simply "a"; the set
    for "[A-Za-z][A-Za-z0-9]*" contains "A" through "Z" and "a" through "z".
    The set of first characters for "a*" is the set of all characters, since
    the regular expression could still match a zero-length string if the
    target string begins with any character other than "a".  FIRST_CHAR_OF
    returns a function value of zero if the computed set does not contain
    all characters; a non-zero value is returned if the computed set does
    contain all characters.
*******************************************************************************/


static  int  first_char_of (

#    if PROTOTYPES
        int  state,
        cs_set  *first_set)
#    else
        state, first_set)

        int  state ;
        cs_set  *first_set ;
#    endif

{    /* Local variables. */
    int  i ;



    LGI "(first_char_of) State = %d\n", state) ;
    if ((state < 0) || (rtx->state_list[state].z.visited))  return (0) ;

/* Mark the state as visited during the traversal.  The VISITED flags keep
   the program from looping endlessly on cycles in the RE's graph (e.g.,
   closure states). */

    rtx->state_list[state].z.visited = 1 ;

/* Compute the set of first characters, starting with this state and proceeding
   to the end of the RE's graph. */

    switch (rtx->state_list[state].type) {
    case empty:
        return (first_char_of (rtx->state_list[state].next1, first_set)) ;
    case anchor:
        return (first_char_of (rtx->state_list[state].next1, first_set)) ;
    case alternation:
        if (first_char_of (rtx->state_list[state].next1, first_set))
            return (1) ;
        return (first_char_of (rtx->state_list[state].next2, first_set)) ;
    case closure:
        first_char_of (rtx->state_list[state].next2, first_set) ;
        return (first_char_of (rtx->state_list[state].next1, first_set)) ;
    case final:
        for (i = 0 ;  i < CS_SETSIZE ;  i++)
            CS_SET (i, first_set) ;
        return (1) ;
    case match:
        if (rtx->state_list[state].x.match_char < 0) {
            for (i = 0 ;  i < CS_SETSIZE ;  i++)
                CS_SET (i, first_set) ;
            return (1) ;
        } else if (rtx->state_list[state].x.match_char == 0) {
            for (i = 0 ;  i < CS_SETSIZE ;  i++) {
                if (CS_ISSET (i, rtx->state_list[state].y.match_charset))
                    CS_SET (i, first_set) ;
            }
        } else {
            CS_SET (rtx->state_list[state].x.match_char, first_set) ;
        }
        return (0) ;
    case left_paren:
        return (first_char_of (rtx->state_list[state].next1, first_set)) ;
    case right_paren:
        return (first_char_of (rtx->state_list[state].next1, first_set)) ;
    default:
        return (0) ;
    }

}

/*!*****************************************************************************
    Function LAST_OF locates the last state in a regular expression graph.
    LAST_OF, passed the index of the first state in the graph, simply traverses
    the NEXT1 links until a null link (index = -1) is found.  Note that this
    technique assumes that the NEXT1 links lead to the last state in the
    graph; YYPARSE adheres to this convention and uses NEXT2 links only for
    backtracking in closure (RE* and RE+) expressions.
*******************************************************************************/

static  int  last_of (

#    if PROTOTYPES
        int  state)		/* The first state in the RE graph. */
#    else
        state)

        int  state ;		/* The first state in the RE graph. */
#    endif

{

    while (rtx->state_list[state].next1 >= 0)
        state = rtx->state_list[state].next1 ;

    return (state) ;

}

/*!*****************************************************************************
    Function LONGEST_PATH computes the longest path through an RE.  The
    longest path is an estimate of the maximum number of states that would
    be visited during an attempt to match a target string.
*******************************************************************************/


static  int  longest_path (

#    if PROTOTYPES
        int  state)
#    else
        state)

        int  state ;
#    endif

{    /* Local variables. */
    int  i, j ;



    LGI "(longest_path) State = %d\n", state) ;
    if ((state < 0) || (rtx->state_list[state].z.visited))  return (0) ;

/* Mark the state as visited during the traversal.  The VISITED flags keep
   the program from looping endlessly on cycles in the RE's graph (e.g.,
   closure states). */

    rtx->state_list[state].z.visited = 1 ;

/* Compute the longest path from this state to the end of the RE's graph. */

    switch (rtx->state_list[state].type) {
    case empty:
        return (longest_path (rtx->state_list[state].next1)) ;
    case anchor:
        return (longest_path (rtx->state_list[state].next1)) ;
    case alternation:
        i = 1 + longest_path (rtx->state_list[state].next1) ;
        j = 1 + longest_path (rtx->state_list[state].next2) ;
        return ((i > j) ? i : j) ;
    case closure:
        return (longest_path (rtx->state_list[state].next2) + 2 +
                longest_path (rtx->state_list[state].next1)) ;
    case final:
        return (0) ;
    case match:
        return (1 + longest_path (rtx->state_list[state].next1)) ;
    case left_paren:
        return (1 + longest_path (rtx->state_list[state].next1)) ;
    case right_paren:
        return (1 + longest_path (rtx->state_list[state].next1)) ;
    default:
        return (0) ;
    }

}

/*!*****************************************************************************
    Function SHORTEST_PATH computes the shortest path through an RE.  The
    shortest path equals the minimum length of a target string that would
    be matched by the RE.
*******************************************************************************/


static  int  shortest_path (

#    if PROTOTYPES
        int  state)
#    else
        state)

        int  state ;
#    endif

{    /* Local variables. */
    int  i, j ;



    LGI "(shortest_path) State = %d\n", state) ;
    if ((state < 0) || (rtx->state_list[state].z.visited))  return (0) ;

/* Mark the state as visited during the traversal.  The VISITED flags keep
   the program from looping endlessly on cycles in the RE's graph (e.g.,
   closure states). */

    rtx->state_list[state].z.visited = 1 ;

/* Compute the shortest path from this state to the end of the RE's graph. */

    switch (rtx->state_list[state].type) {
    case empty:
        return (shortest_path (rtx->state_list[state].next1)) ;
    case anchor:
        return (shortest_path (rtx->state_list[state].next1)) ;
    case alternation:
        i = shortest_path (rtx->state_list[state].next1) ;
        j = shortest_path (rtx->state_list[state].next2) ;
        return ((i < j) ? i : j) ;
    case closure:
        return ((shortest_path (rtx->state_list[state].next2) *
                 rtx->state_list[state].x.min_closure)  +
                shortest_path (rtx->state_list[state].next1)) ;
    case final:
        return (0) ;
    case match:
        return (1 + shortest_path (rtx->state_list[state].next1)) ;
    case left_paren:
        return (shortest_path (rtx->state_list[state].next1)) ;
    case right_paren:
        return (shortest_path (rtx->state_list[state].next1)) ;
    default:
        return (0) ;
    }

}
