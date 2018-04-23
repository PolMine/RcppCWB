/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     QID = 259,
     LABEL = 260,
     STRING = 261,
     FLAG = 262,
     TAGSTART = 263,
     TAGEND = 264,
     VARIABLE = 265,
     IPAddress = 266,
     IPSubnet = 267,
     INTEGER = 268,
     FLOAT = 269,
     FIELD = 270,
     FIELDLABEL = 271,
     ANCHORTAG = 272,
     ANCHORENDTAG = 273,
     SEARCH_STRATEGY = 274,
     TAB_SYM = 275,
     CAT_SYM = 276,
     DEFINE_SYM = 277,
     DIFF_SYM = 278,
     DISCARD_SYM = 279,
     EXPAND_SYM = 280,
     EXIT_SYM = 281,
     FLAT_SYM = 282,
     INTER_SYM = 283,
     JOIN_SYM = 284,
     SUBSET_SYM = 285,
     LEFT_SYM = 286,
     RIGHT_SYM = 287,
     SAVE_SYM = 288,
     SCATTER_SYM = 289,
     SHOW_SYM = 290,
     CD_SYM = 291,
     TO_SYM = 292,
     WITHIN_SYM = 293,
     SET_SYM = 294,
     EXEC_SYM = 295,
     CUT_SYM = 296,
     OCCURS_SYM = 297,
     INFO_SYM = 298,
     GROUP_SYM = 299,
     WHERE_SYM = 300,
     ESCAPE_SYM = 301,
     MEET_SYM = 302,
     UNION_SYM = 303,
     MU_SYM = 304,
     SORT_SYM = 305,
     COUNT_SYM = 306,
     ASC_SYM = 307,
     DESC_SYM = 308,
     REVERSE_SYM = 309,
     BY_SYM = 310,
     FOREACH_SYM = 311,
     ON_SYM = 312,
     YES_SYM = 313,
     OFF_SYM = 314,
     NO_SYM = 315,
     SLEEP_SYM = 316,
     REDUCE_SYM = 317,
     MAXIMAL_SYM = 318,
     WITH_SYM = 319,
     WITHOUT_SYM = 320,
     DELETE_SYM = 321,
     SIZE_SYM = 322,
     DUMP_SYM = 323,
     UNDUMP_SYM = 324,
     TABULATE_SYM = 325,
     NOT_SYM = 326,
     CONTAINS_SYM = 327,
     MATCHES_SYM = 328,
     GCDEL = 329,
     APPEND = 330,
     LET = 331,
     GET = 332,
     NEQ = 333,
     IMPLIES = 334,
     RE_PAREN = 335,
     EOL_SYM = 336,
     ELLIPSIS = 337,
     MATCHALL = 338,
     LCSTART = 339,
     LCEND = 340,
     LCMATCHALL = 341,
     PLUSEQ = 342,
     MINUSEQ = 343,
     UNLOCK_SYM = 344,
     USER_SYM = 345,
     HOST_SYM = 346,
     UNDEFINED_MACRO = 347,
     MACRO_SYM = 348,
     RANDOMIZE_SYM = 349,
     FROM_SYM = 350,
     INCLUSIVE_SYM = 351,
     EXCLUSIVE_SYM = 352,
     NULL_SYM = 353
   };
#endif
/* Tokens.  */
#define ID 258
#define QID 259
#define LABEL 260
#define STRING 261
#define FLAG 262
#define TAGSTART 263
#define TAGEND 264
#define VARIABLE 265
#define IPAddress 266
#define IPSubnet 267
#define INTEGER 268
#define FLOAT 269
#define FIELD 270
#define FIELDLABEL 271
#define ANCHORTAG 272
#define ANCHORENDTAG 273
#define SEARCH_STRATEGY 274
#define TAB_SYM 275
#define CAT_SYM 276
#define DEFINE_SYM 277
#define DIFF_SYM 278
#define DISCARD_SYM 279
#define EXPAND_SYM 280
#define EXIT_SYM 281
#define FLAT_SYM 282
#define INTER_SYM 283
#define JOIN_SYM 284
#define SUBSET_SYM 285
#define LEFT_SYM 286
#define RIGHT_SYM 287
#define SAVE_SYM 288
#define SCATTER_SYM 289
#define SHOW_SYM 290
#define CD_SYM 291
#define TO_SYM 292
#define WITHIN_SYM 293
#define SET_SYM 294
#define EXEC_SYM 295
#define CUT_SYM 296
#define OCCURS_SYM 297
#define INFO_SYM 298
#define GROUP_SYM 299
#define WHERE_SYM 300
#define ESCAPE_SYM 301
#define MEET_SYM 302
#define UNION_SYM 303
#define MU_SYM 304
#define SORT_SYM 305
#define COUNT_SYM 306
#define ASC_SYM 307
#define DESC_SYM 308
#define REVERSE_SYM 309
#define BY_SYM 310
#define FOREACH_SYM 311
#define ON_SYM 312
#define YES_SYM 313
#define OFF_SYM 314
#define NO_SYM 315
#define SLEEP_SYM 316
#define REDUCE_SYM 317
#define MAXIMAL_SYM 318
#define WITH_SYM 319
#define WITHOUT_SYM 320
#define DELETE_SYM 321
#define SIZE_SYM 322
#define DUMP_SYM 323
#define UNDUMP_SYM 324
#define TABULATE_SYM 325
#define NOT_SYM 326
#define CONTAINS_SYM 327
#define MATCHES_SYM 328
#define GCDEL 329
#define APPEND 330
#define LET 331
#define GET 332
#define NEQ 333
#define IMPLIES 334
#define RE_PAREN 335
#define EOL_SYM 336
#define ELLIPSIS 337
#define MATCHALL 338
#define LCSTART 339
#define LCEND 340
#define LCMATCHALL 341
#define PLUSEQ 342
#define MINUSEQ 343
#define UNLOCK_SYM 344
#define USER_SYM 345
#define HOST_SYM 346
#define UNDEFINED_MACRO 347
#define MACRO_SYM 348
#define RANDOMIZE_SYM 349
#define FROM_SYM 350
#define INCLUSIVE_SYM 351
#define EXCLUSIVE_SYM 352
#define NULL_SYM 353




/* Copy the first part of user declarations.  */
#line 1 "parser.y"

/* 
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 * 
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */


#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "../cl/globals.h"
#include "../cl/special-chars.h"
#include "../cl/attributes.h"
#include "../cl/macros.h"

#include "cqp.h"
#include "options.h"
#include "ranges.h"
#include "symtab.h"
#include "treemacros.h"
#include "tree.h"
#include "eval.h"
#include "corpmanag.h"
#include "regex2dfa.h"
#include "builtins.h"
#include "groups.h"
#include "targets.h"
#include "attlist.h"
#include "concordance.h"
#include "output.h"
#include "print-modes.h"
#include "variables.h"

#include "parse_actions.h"

/* CQPserver user authentication */
#include "../CQi/auth.h"

/* macro expansion */
#include "macro.h"
 


/* ============================================================ YACC IF */

extern int yychar;

extern int yylex(void);

void yyerror (char *s)
{
  cqpmessage(Error, "CQP Syntax Error: %s\n\t%s <--", s, QueryBuffer);
  generate_code = 0;
}

void warn_query_lock_violation(void) {
  if (which_app != cqpserver)
    fprintf(stderr, "WARNING: query lock violation attempted\n");
  query_lock_violation++;       /* this is for the CQPserver */
}

/* ============================================================ */

#define SYNCHRONIZE

void
synchronize(void)
{
#if defined(SYNCHRONIZE)
  int macro_status;

  /* delete macro buffers & disable macro expansion while sync'ing */
  delete_macro_buffers(1); /* print stack trace on STDERR */
  macro_status = enable_macros;
  enable_macros = 0;

  if (cqp_input_string != NULL) {
    fprintf(stderr, "Synchronizing to end of line ... \n");
    while (!(yychar <= 0))
      yychar = yylex();
  }
  else {
    fprintf(stderr, "Synchronizing until next ';'...\n");
    while (!(yychar <= 0 || yychar == ';'))
      yychar = yylex();
  }

  enable_macros = macro_status; /* reset enable_macros to previous value */
#endif
}

#define YYERROR_VERBOSE



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 114 "parser.y"
{
  Evaltree           evalt;
  Constrainttree     boolt;
  enum b_ops         boolo;
  int                ival;
  double             fval;
  int                index;
  char              *strval;
  CorpusList        *cl;

  struct {
    int a, b;
  } intpair;

  Context            context;
  ActualParamList   *apl;

  enum ctxtdir       direction;

  struct Redir       redir;

  struct InputRedir  in_redir;

  struct {
    int ok;
    int ival;
    char *cval;
  }                  varval;

  struct {
    FieldType field;
    int inclusive;
  }                  base;

  struct {
    char *variableValue;
    char operator;
  }                  varsetting;

  struct {
    int mindist;
    int maxdist;
  }                  Distance;

  struct {
    FieldType anchor;
    int offset;
  }                  Anchor;

  struct {
    FieldType anchor1;
    int offset1;
    FieldType anchor2;
    int offset2;
  }                  AnchorPair;

  struct {
    char *name;
    int flags;
  }                  AttributeSpecification;

  RangeSetOp         rngsetop;

  SortClause         sortclause;

  FieldType          field;

  SearchStrategy     search_strategy;

  TabulationItem     tabulation_item;
}
/* Line 193 of yacc.c.  */
#line 477 "parser.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 490 "parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   478

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  121
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  137
/* YYNRULES -- Number of rules.  */
#define YYNRULES  325
/* YYNRULES -- Number of states.  */
#define YYNSTATES  492

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   353

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   103,     2,     2,     2,   110,   102,     2,
     112,   113,   100,   101,   109,   108,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   119,   104,
     107,   105,   106,   111,   116,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   117,     2,   118,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   114,    99,   115,   120,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    10,    11,    15,    16,
      21,    24,    25,    29,    30,    33,    35,    37,    41,    44,
      46,    48,    49,    52,    54,    56,    58,    60,    62,    64,
      66,    68,    70,    72,    74,    76,    78,    80,    82,    84,
      86,    88,    90,    92,    94,    96,    99,   101,   105,   113,
     117,   121,   123,   124,   127,   130,   132,   133,   136,   138,
     141,   144,   147,   150,   152,   155,   158,   162,   164,   166,
     168,   170,   172,   173,   180,   183,   186,   188,   191,   195,
     200,   205,   210,   213,   216,   219,   222,   226,   229,   231,
     236,   241,   242,   254,   258,   259,   261,   263,   264,   266,
     268,   270,   272,   274,   276,   278,   281,   284,   287,   289,
     300,   311,   319,   321,   322,   323,   329,   330,   340,   342,
     346,   349,   351,   355,   358,   359,   363,   368,   374,   376,
     377,   384,   387,   392,   393,   395,   397,   398,   400,   401,
     407,   413,   417,   422,   424,   425,   430,   435,   439,   441,
     445,   448,   452,   455,   459,   467,   473,   475,   476,   477,
     480,   484,   486,   490,   492,   494,   496,   501,   506,   508,
     509,   510,   511,   512,   519,   523,   525,   528,   530,   533,
     536,   539,   542,   544,   548,   550,   552,   554,   558,   564,
     566,   568,   571,   577,   579,   581,   583,   585,   586,   590,
     592,   593,   595,   596,   598,   600,   603,   605,   609,   611,
     615,   617,   619,   620,   623,   624,   625,   632,   633,   635,
     636,   640,   641,   644,   645,   647,   648,   650,   651,   653,
     654,   659,   660,   662,   664,   665,   668,   670,   672,   673,
     675,   679,   683,   687,   691,   694,   696,   700,   705,   707,
     710,   713,   715,   716,   718,   721,   723,   726,   728,   730,
     732,   735,   740,   742,   744,   746,   748,   750,   752,   754,
     756,   758,   763,   765,   769,   771,   773,   775,   777,   779,
     786,   789,   791,   792,   798,   802,   805,   809,   810,   814,
     820,   825,   826,   827,   833,   836,   839,   842,   846,   847,
     850,   851,   859,   867,   872,   874,   875,   878,   882,   889,
     892,   894,   896,   899,   900,   902,   903,   905,   906,   908,
     909,   911,   912,   914,   915,   917
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     122,     0,    -1,    -1,   123,   124,    -1,   104,    -1,    -1,
      -1,   125,   129,   104,    -1,    -1,    89,    13,   126,   104,
      -1,   134,   104,    -1,    -1,   127,   133,   104,    -1,    -1,
     128,    26,    -1,     1,    -1,   130,    -1,     3,   105,   130,
      -1,   131,   218,    -1,   222,    -1,   144,    -1,    -1,   132,
     185,    -1,   141,    -1,   135,    -1,   136,    -1,   148,    -1,
     177,    -1,   175,    -1,   153,    -1,   151,    -1,   150,    -1,
     158,    -1,   159,    -1,   160,    -1,   169,    -1,   179,    -1,
     180,    -1,   181,    -1,   182,    -1,   162,    -1,   242,    -1,
     246,    -1,   248,    -1,   250,    -1,    46,   251,    -1,    81,
      -1,    21,   221,   137,    -1,    21,   221,   254,    13,   255,
      13,   137,    -1,    21,   144,   137,    -1,    33,   221,   137,
      -1,   138,    -1,    -1,   106,     6,    -1,    75,     6,    -1,
     140,    -1,    -1,   107,     6,    -1,    35,    -1,    35,     3,
      -1,    35,   142,    -1,    35,    36,    -1,   142,   143,    -1,
     143,    -1,   101,     3,    -1,   108,     3,    -1,   145,   222,
     222,    -1,   146,    -1,    29,    -1,    48,    -1,    28,    -1,
      23,    -1,    -1,    30,   221,    45,    16,   147,   206,    -1,
      24,   149,    -1,   149,   222,    -1,   222,    -1,    35,    10,
      -1,    22,    10,   152,    -1,    22,    10,    87,    10,    -1,
      22,    10,    88,    10,    -1,    22,    10,   105,    10,    -1,
     107,     6,    -1,   105,     6,    -1,    87,     6,    -1,    88,
       6,    -1,    39,     3,   157,    -1,    39,     3,    -1,    39,
      -1,    39,   222,    15,    15,    -1,    39,   222,    15,    98,
      -1,    -1,    39,   222,    15,    19,   154,   206,    38,   219,
     215,     3,   155,    -1,    95,    15,   156,    -1,    -1,    96,
      -1,    97,    -1,    -1,    13,    -1,     6,    -1,     3,    -1,
      57,    -1,    58,    -1,    59,    -1,    60,    -1,    13,     3,
      -1,    40,     6,    -1,    43,   222,    -1,    43,    -1,    44,
     222,   257,     3,    55,   257,     3,   214,   161,   137,    -1,
      44,   222,   257,     3,    56,   257,     3,   214,   161,   137,
      -1,    44,   222,   257,     3,   214,   161,   137,    -1,    25,
      -1,    -1,    -1,    70,   222,   163,   165,   137,    -1,    -1,
      70,   222,   164,   254,    13,   255,    13,   165,   137,    -1,
     166,    -1,   165,   109,   166,    -1,   167,   168,    -1,   257,
      -1,   257,   256,   257,    -1,     3,   208,    -1,    -1,    50,
     221,   170,    -1,    50,   221,    94,   216,    -1,    51,   221,
     171,   214,   137,    -1,   171,    -1,    -1,    55,     3,   208,
     172,   173,   174,    -1,   253,   257,    -1,   253,   257,   256,
     257,    -1,    -1,    52,    -1,    53,    -1,    -1,    54,    -1,
      -1,    62,   221,    37,    13,   176,    -1,    62,   221,    37,
      63,    73,    -1,    41,   221,    13,    -1,    41,   221,    13,
      13,    -1,   110,    -1,    -1,    66,   221,    64,    15,    -1,
      66,   221,    65,    15,    -1,    66,   221,   178,    -1,    13,
      -1,    13,    82,    13,    -1,    61,    13,    -1,    67,   222,
     252,    -1,    67,    10,    -1,    68,   222,   137,    -1,    68,
     222,   254,    13,   255,    13,   137,    -1,    69,     3,   184,
     183,   139,    -1,    52,    -1,    -1,    -1,    64,    15,    -1,
      64,    15,    15,    -1,   186,    -1,   186,    50,   170,    -1,
     187,    -1,   188,    -1,   238,    -1,   190,   210,   214,   189,
      -1,    49,   234,   189,   214,    -1,   103,    -1,    -1,    -1,
      -1,    -1,   191,   194,   192,   209,   193,   213,    -1,   194,
      99,   195,    -1,   195,    -1,   195,   196,    -1,   196,    -1,
     197,   198,    -1,   197,   100,    -1,   197,   101,    -1,   197,
     111,    -1,   197,    -1,   112,   194,   113,    -1,   202,    -1,
     200,    -1,   199,    -1,   114,    13,   115,    -1,   114,    13,
     109,   217,   115,    -1,    17,    -1,    18,    -1,     8,   106,
      -1,     8,   201,     6,   208,   106,    -1,     9,    -1,   225,
      -1,    78,    -1,   105,    -1,    -1,   203,   204,   205,    -1,
     116,    -1,    -1,     5,    -1,    -1,   206,    -1,   207,    -1,
       6,   208,    -1,    10,    -1,   117,   223,   118,    -1,    83,
      -1,    84,   223,    85,    -1,    86,    -1,     7,    -1,    -1,
      74,   223,    -1,    -1,    -1,   210,   119,     3,   211,   212,
     190,    -1,    -1,   103,    -1,    -1,    38,   219,   220,    -1,
      -1,    41,    13,    -1,    -1,    13,    -1,    -1,    13,    -1,
      -1,    13,    -1,    -1,    25,   219,    37,   220,    -1,    -1,
      31,    -1,    32,    -1,    -1,   215,     3,    -1,    13,    -1,
     222,    -1,    -1,     3,    -1,   223,    79,   223,    -1,   223,
      99,   223,    -1,   223,   102,   223,    -1,   112,   223,   113,
      -1,   103,   223,    -1,   224,    -1,   227,   229,   228,    -1,
     227,   225,     6,   208,    -1,   227,    -1,   226,    72,    -1,
     226,    73,    -1,    71,    -1,    -1,   233,    -1,   120,   233,
      -1,     3,    -1,   120,     3,    -1,    15,    -1,   230,    -1,
     227,    -1,     6,   208,    -1,    80,    10,   113,   208,    -1,
      10,    -1,    13,    -1,    14,    -1,   107,    -1,   106,    -1,
     105,    -1,    78,    -1,    76,    -1,    77,    -1,     3,   112,
     231,   113,    -1,   232,    -1,   231,   109,   232,    -1,   228,
      -1,     4,    -1,   235,    -1,   237,    -1,   205,    -1,   112,
      47,   234,   234,   236,   113,    -1,    13,    13,    -1,     3,
      -1,    -1,   112,    48,   234,   234,   113,    -1,    20,   239,
     213,    -1,   202,   240,    -1,   240,   241,   202,    -1,    -1,
     114,    13,   115,    -1,   114,    13,   109,   217,   115,    -1,
     114,   109,    13,   115,    -1,    -1,    -1,    90,     3,     6,
     243,   244,    -1,    91,    11,    -1,    91,    12,    -1,    91,
     100,    -1,   112,   245,   113,    -1,    -1,   245,     3,    -1,
      -1,   247,    93,     3,   112,    13,   113,   249,    -1,   247,
      93,     3,   112,     6,   113,   249,    -1,   247,    93,   107,
       6,    -1,    22,    -1,    -1,    35,    93,    -1,    35,    93,
       3,    -1,    35,    93,     3,   112,    13,   113,    -1,   249,
       6,    -1,     6,    -1,    94,    -1,    94,    13,    -1,    -1,
      15,    -1,    -1,    57,    -1,    -1,    95,    -1,    -1,    37,
      -1,    -1,    82,    -1,    -1,    15,    -1,    15,   117,    13,
     118,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   352,   352,   352,   357,   358,   361,   361,   364,   363,
     374,   375,   375,   377,   377,   379,   388,   389,   394,   397,
     398,   399,   399,   403,   404,   405,   406,   407,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
     420,   421,   422,   423,   424,   425,   430,   433,   436,   439,
     446,   450,   451,   458,   463,   471,   472,   478,   484,   487,
     501,   503,   508,   510,   514,   515,   518,   519,   522,   523,
     524,   525,   531,   528,   547,   550,   553,   558,   563,   572,
     578,   584,   595,   596,   597,   598,   601,   627,   635,   638,
     639,   640,   640,   657,   662,   668,   669,   670,   673,   677,
     681,   686,   687,   688,   689,   690,   697,   700,   701,   704,
     710,   716,   725,   726,   732,   731,   736,   735,   741,   743,
     747,   759,   761,   766,   769,   778,   790,   801,   813,   814,
     817,   835,   836,   840,   844,   845,   846,   849,   850,   855,
     859,   863,   867,   873,   874,   877,   883,   889,   897,   898,
     903,   907,   908,   912,   914,   918,   922,   923,   927,   928,
     933,   944,   945,   955,   956,   957,   960,   965,   969,   970,
     973,   974,   976,   973,   980,   982,   985,   987,   990,   998,
    1003,  1008,  1013,  1016,  1017,  1022,  1027,  1034,  1039,  1048,
    1049,  1052,  1053,  1055,  1058,  1059,  1060,  1061,  1064,  1070,
    1071,  1074,  1075,  1078,  1079,  1082,  1083,  1095,  1096,  1104,
    1105,  1113,  1144,  1147,  1148,  1153,  1152,  1158,  1161,  1162,
    1165,  1173,  1181,  1182,  1185,  1186,  1189,  1190,  1193,  1194,
    1197,  1203,  1210,  1211,  1212,  1215,  1216,  1219,  1220,  1224,
    1244,  1245,  1246,  1247,  1248,  1249,  1252,  1253,  1262,  1265,
    1266,  1269,  1270,  1273,  1274,  1275,  1276,  1277,  1278,  1281,
    1282,  1283,  1287,  1304,  1312,  1322,  1323,  1324,  1325,  1326,
    1327,  1330,  1333,  1335,  1352,  1363,  1366,  1367,  1368,  1376,
    1383,  1389,  1390,  1398,  1404,  1410,  1414,  1418,  1421,  1422,
    1424,  1425,  1430,  1430,  1432,  1433,  1434,  1437,  1438,  1442,
    1444,  1448,  1457,  1467,  1473,  1474,  1478,  1481,  1485,  1494,
    1504,  1508,  1509,  1512,  1517,  1518,  1520,  1521,  1523,  1524,
    1526,  1527,  1529,  1530,  1534,  1535
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ID", "QID", "LABEL", "STRING", "FLAG",
  "TAGSTART", "TAGEND", "VARIABLE", "IPAddress", "IPSubnet", "INTEGER",
  "FLOAT", "FIELD", "FIELDLABEL", "ANCHORTAG", "ANCHORENDTAG",
  "SEARCH_STRATEGY", "TAB_SYM", "CAT_SYM", "DEFINE_SYM", "DIFF_SYM",
  "DISCARD_SYM", "EXPAND_SYM", "EXIT_SYM", "FLAT_SYM", "INTER_SYM",
  "JOIN_SYM", "SUBSET_SYM", "LEFT_SYM", "RIGHT_SYM", "SAVE_SYM",
  "SCATTER_SYM", "SHOW_SYM", "CD_SYM", "TO_SYM", "WITHIN_SYM", "SET_SYM",
  "EXEC_SYM", "CUT_SYM", "OCCURS_SYM", "INFO_SYM", "GROUP_SYM",
  "WHERE_SYM", "ESCAPE_SYM", "MEET_SYM", "UNION_SYM", "MU_SYM", "SORT_SYM",
  "COUNT_SYM", "ASC_SYM", "DESC_SYM", "REVERSE_SYM", "BY_SYM",
  "FOREACH_SYM", "ON_SYM", "YES_SYM", "OFF_SYM", "NO_SYM", "SLEEP_SYM",
  "REDUCE_SYM", "MAXIMAL_SYM", "WITH_SYM", "WITHOUT_SYM", "DELETE_SYM",
  "SIZE_SYM", "DUMP_SYM", "UNDUMP_SYM", "TABULATE_SYM", "NOT_SYM",
  "CONTAINS_SYM", "MATCHES_SYM", "GCDEL", "APPEND", "LET", "GET", "NEQ",
  "IMPLIES", "RE_PAREN", "EOL_SYM", "ELLIPSIS", "MATCHALL", "LCSTART",
  "LCEND", "LCMATCHALL", "PLUSEQ", "MINUSEQ", "UNLOCK_SYM", "USER_SYM",
  "HOST_SYM", "UNDEFINED_MACRO", "MACRO_SYM", "RANDOMIZE_SYM", "FROM_SYM",
  "INCLUSIVE_SYM", "EXCLUSIVE_SYM", "NULL_SYM", "'|'", "'*'", "'+'", "'&'",
  "'!'", "';'", "'='", "'>'", "'<'", "'-'", "','", "'%'", "'?'", "'('",
  "')'", "'{'", "'}'", "'@'", "'['", "']'", "':'", "'~'", "$accept",
  "line", "@1", "command", "@2", "@3", "@4", "@5", "CorpusCommand",
  "UnnamedCorpusCommand", "CYCommand", "@6", "InteractiveCommand",
  "EOLCmd", "Cat", "Saving", "OptionalRedir", "Redir",
  "OptionalInputRedir", "InputRedir", "Showing", "AttributeSelections",
  "AttributeSelection", "CorpusSetExpr", "SetOp", "SubsetExpr", "@7",
  "Discard", "DiscArgs", "VarPrintCmd", "VarDefCmd", "VariableValueSpec",
  "OptionSetCmd", "@8", "OptBase", "InclusiveExclusive", "VarValue",
  "ExecCmd", "InfoCmd", "GroupCmd", "OptExpansion", "TabulateCmd", "@9",
  "@10", "TabulationItems", "TabulationItem", "TabulationRange",
  "OptAttributeSpec", "SortCmd", "OptionalSortClause", "SortClause",
  "SortBoundaries", "SortDirection", "OptReverse", "Reduction",
  "OptPercent", "Delete", "LineRange", "SleepCmd", "SizeCmd", "DumpCmd",
  "UndumpCmd", "OptAscending", "OptWithTargetKeyword", "Query", "AQuery",
  "StandardQuery", "MUQuery", "OptKeep", "SearchPattern", "@11", "@12",
  "@13", "RegWordfExpr", "RegWordfTerm", "RegWordfFactor", "RegWordfPower",
  "Repeat", "AnchorPoint", "XMLTag", "RegexpOp", "NamedWfPattern",
  "OptTargetSign", "OptRefId", "WordformPattern", "ExtConstraint",
  "LookaheadConstraint", "OptionalFlag", "GlobalConstraint",
  "AlignmentConstraints", "@14", "OptNot", "SearchSpace", "CutStatement",
  "OptNumber", "OptInteger", "OptMaxNumber", "ReStructure", "OptDirection",
  "Description", "OptionalCID", "CID", "BoolExpr", "RelExpr", "MvalOp",
  "OptionalNot", "RelLHS", "RelRHS", "RelOp", "FunctionCall",
  "FunctionArgList", "SingleArg", "LabelReference", "MUStatement",
  "MeetStatement", "MeetContext", "UnionStatement", "TABQuery",
  "TabPatterns", "TabOtherPatterns", "OptDistance", "AuthorizeCmd", "@15",
  "OptionalGrants", "Grants", "Macro", "OptDEFINE_SYM", "ShowMacro",
  "MultiString", "RandomizeCmd", "OtherCommand", "OptionalFIELD", "OptON",
  "OptFROM", "OptTO", "OptELLIPSIS", "Anchor", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   124,
      42,    43,    38,    33,    59,    61,    62,    60,    45,    44,
      37,    63,    40,    41,   123,   125,    64,    91,    93,    58,
     126
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   121,   123,   122,   122,   122,   125,   124,   126,   124,
     124,   127,   124,   128,   124,   124,   129,   129,   130,   131,
     131,   132,   131,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   133,   133,   133,   133,
     133,   133,   133,   133,   133,   133,   134,   135,   135,   135,
     136,   137,   137,   138,   138,   139,   139,   140,   141,   141,
     141,   141,   142,   142,   143,   143,   144,   144,   145,   145,
     145,   145,   147,   146,   148,   149,   149,   150,   151,   151,
     151,   151,   152,   152,   152,   152,   153,   153,   153,   153,
     153,   154,   153,   155,   155,   156,   156,   156,   157,   157,
     157,   157,   157,   157,   157,   157,   158,   159,   159,   160,
     160,   160,   161,   161,   163,   162,   164,   162,   165,   165,
     166,   167,   167,   168,   168,   169,   169,   169,   170,   170,
     171,   172,   172,   172,   173,   173,   173,   174,   174,   175,
     175,   175,   175,   176,   176,   177,   177,   177,   178,   178,
     179,   180,   180,   181,   181,   182,   183,   183,   184,   184,
     184,   185,   185,   186,   186,   186,   187,   188,   189,   189,
     191,   192,   193,   190,   194,   194,   195,   195,   196,   196,
     196,   196,   196,   197,   197,   197,   197,   198,   198,   199,
     199,   200,   200,   200,   201,   201,   201,   201,   202,   203,
     203,   204,   204,   205,   205,   206,   206,   206,   206,   207,
     207,   208,   208,   209,   209,   211,   210,   210,   212,   212,
     213,   213,   214,   214,   215,   215,   216,   216,   217,   217,
     218,   218,   219,   219,   219,   220,   220,   221,   221,   222,
     223,   223,   223,   223,   223,   223,   224,   224,   224,   225,
     225,   226,   226,   227,   227,   227,   227,   227,   227,   228,
     228,   228,   228,   228,   228,   229,   229,   229,   229,   229,
     229,   230,   231,   231,   232,   233,   234,   234,   234,   235,
     236,   236,   236,   237,   238,   239,   240,   240,   241,   241,
     241,   241,   243,   242,   242,   242,   242,   244,   244,   245,
     245,   246,   246,   246,   247,   247,   248,   248,   248,   249,
     249,   250,   250,   251,   252,   252,   253,   253,   254,   254,
     255,   255,   256,   256,   257,   257
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     0,     0,     3,     0,     4,
       2,     0,     3,     0,     2,     1,     1,     3,     2,     1,
       1,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     1,     3,     7,     3,
       3,     1,     0,     2,     2,     1,     0,     2,     1,     2,
       2,     2,     2,     1,     2,     2,     3,     1,     1,     1,
       1,     1,     0,     6,     2,     2,     1,     2,     3,     4,
       4,     4,     2,     2,     2,     2,     3,     2,     1,     4,
       4,     0,    11,     3,     0,     1,     1,     0,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     1,    10,
      10,     7,     1,     0,     0,     5,     0,     9,     1,     3,
       2,     1,     3,     2,     0,     3,     4,     5,     1,     0,
       6,     2,     4,     0,     1,     1,     0,     1,     0,     5,
       5,     3,     4,     1,     0,     4,     4,     3,     1,     3,
       2,     3,     2,     3,     7,     5,     1,     0,     0,     2,
       3,     1,     3,     1,     1,     1,     4,     4,     1,     0,
       0,     0,     0,     6,     3,     1,     2,     1,     2,     2,
       2,     2,     1,     3,     1,     1,     1,     3,     5,     1,
       1,     2,     5,     1,     1,     1,     1,     0,     3,     1,
       0,     1,     0,     1,     1,     2,     1,     3,     1,     3,
       1,     1,     0,     2,     0,     0,     6,     0,     1,     0,
       3,     0,     2,     0,     1,     0,     1,     0,     1,     0,
       4,     0,     1,     1,     0,     2,     1,     1,     0,     1,
       3,     3,     3,     3,     2,     1,     3,     4,     1,     2,
       2,     1,     0,     1,     2,     1,     2,     1,     1,     1,
       2,     4,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     3,     1,     1,     1,     1,     1,     6,
       2,     1,     0,     5,     3,     2,     3,     0,     3,     5,
       4,     0,     0,     5,     2,     2,     2,     3,     0,     2,
       0,     7,     7,     4,     1,     0,     2,     3,     6,     2,
       1,     1,     2,     0,     1,     0,     1,     0,     1,     0,
       1,     0,     1,     0,     1,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     4,     0,     0,     1,    15,    46,     0,     3,    21,
     305,     0,     0,     8,   239,    71,    70,    68,   238,    69,
       0,    16,   231,   170,    20,     0,    67,    19,   238,   304,
       0,   238,    58,    88,     0,   238,   108,     0,   313,   238,
     238,     0,   238,   238,     0,     0,     0,     0,     0,     0,
     311,     0,    24,    25,    23,    26,    31,    30,    29,    32,
      33,    34,    40,    35,    28,    27,    36,    37,    38,    39,
      41,    42,     0,    43,    44,    14,    10,     0,    21,   239,
       0,   237,     7,   234,    18,   200,     0,    22,   161,   163,
     164,   217,   200,   165,     0,    52,    52,     0,    74,    76,
      52,    59,    77,    61,   306,     0,     0,    60,    63,    87,
       0,   106,     0,   107,     0,    45,   129,     0,   150,     0,
       0,   152,   315,    52,   158,   116,     0,   294,   295,   296,
     312,    12,     0,     9,    17,     0,   232,   233,     0,   199,
     287,   202,   221,   212,   206,   208,     0,   210,     0,     0,
     278,   203,   204,   169,   276,   277,   129,   223,   252,   193,
     189,   190,   200,   171,   175,   177,   182,   186,   185,   184,
      66,     0,     0,    49,    51,   318,    47,     0,     0,     0,
       0,     0,    78,    75,    50,   307,    64,    65,    62,   100,
      99,    98,   101,   102,   103,   104,    86,     0,   141,   324,
       0,     0,   227,   125,   128,   223,     0,   148,     0,     0,
     147,   314,   151,   153,     0,     0,   157,     0,   319,   292,
       0,     0,    72,   225,   291,   201,     0,   234,   284,   211,
     205,   255,   275,   257,     0,     0,     0,     0,   245,   248,
     258,   253,     0,     0,     0,   168,   223,   162,     0,     0,
     169,   251,   195,   196,   191,     0,   194,     0,     0,   200,
     214,   176,   179,   180,   181,     0,   178,    54,    53,   321,
      84,    79,    85,    80,    83,    81,    82,     0,   105,    89,
      91,    90,   142,     0,   223,   212,   226,   126,    52,   144,
       0,     0,   145,   146,   321,   159,   156,    56,    52,   118,
     124,   121,     0,   298,     0,   303,     0,   236,     0,   230,
       0,   200,   198,   225,     0,   244,     0,   256,   254,     0,
     209,     0,     0,   269,   270,   268,   267,   266,   265,     0,
       0,     0,     0,   207,   167,   222,   215,   166,   212,   249,
     250,   183,   174,     0,   172,     0,   320,     0,     0,     0,
       0,     0,     0,   113,   133,   127,   143,   139,   140,   149,
       0,   160,     0,   155,    55,     0,   115,   212,   120,   322,
       0,   321,   300,   293,     0,     0,    73,   235,     0,     0,
     286,   220,   212,   262,   263,   264,     0,   259,   274,     0,
     272,   243,   240,   241,   242,   212,   246,   282,     0,   219,
       0,   213,   221,   229,   187,    52,   308,     0,   325,     0,
       0,   112,    52,   316,   136,     0,    52,    57,   119,   123,
     122,     0,     0,     0,     0,   229,   288,     0,   260,     0,
       0,   271,   247,   281,     0,     0,   283,   218,   170,   192,
     173,   228,     0,    48,   234,   223,   223,   111,   134,   135,
     138,   131,   154,     0,   299,   297,   310,   302,   301,     0,
     290,   212,   273,   280,   279,   216,   188,   225,   113,   113,
     137,   130,     0,    52,   309,   289,   261,   224,     0,    52,
      52,   132,   117,    94,   109,   110,     0,    92,    97,    95,
      96,    93
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     8,     9,    77,    10,    11,    20,    21,
      22,    23,    51,    12,    52,    53,   173,   174,   363,   364,
      54,   107,   108,    24,    25,    26,   306,    55,    98,    56,
      57,   182,    58,   349,   487,   491,   196,    59,    60,    61,
     412,    62,   217,   218,   298,   299,   300,   368,    63,   203,
     204,   414,   450,   471,    64,   357,    65,   210,    66,    67,
      68,    69,   297,   216,    87,    88,    89,    90,   246,    91,
      92,   260,   402,   163,   164,   165,   166,   266,   167,   168,
     255,   169,   141,   226,   150,   151,   152,   230,   344,   157,
     399,   438,   228,   250,   308,   287,   442,    84,   138,   309,
      80,    81,   237,   238,   256,   257,   239,   388,   330,   240,
     389,   390,   241,   153,   154,   435,   155,    93,   142,   224,
     311,    70,   303,   373,   422,    71,    72,    73,   457,    74,
     115,   212,   415,   177,   347,   370,   301
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -300
static const yytype_int16 yypact[] =
{
      23,  -300,    73,   244,  -300,  -300,  -300,    98,  -300,   178,
     329,    31,    13,  -300,    28,  -300,  -300,  -300,   117,  -300,
      56,  -300,   113,    44,  -300,   117,  -300,  -300,   188,   180,
     117,   117,    46,   162,   168,   117,   117,   117,  -300,   117,
     117,   197,   117,   117,   134,   117,   220,   117,   238,    57,
     233,   147,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,
    -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,
    -300,  -300,   170,  -300,  -300,  -300,  -300,   156,   188,  -300,
     226,  -300,  -300,   201,  -300,   153,    12,  -300,   232,  -300,
    -300,  -300,    36,  -300,   117,   -27,    39,   -29,   117,  -300,
     -27,  -300,  -300,  -300,   275,   288,   297,    63,  -300,   155,
     286,  -300,   290,  -300,   289,  -300,    15,   253,  -300,   270,
     102,  -300,   300,    39,   254,   302,   313,  -300,  -300,  -300,
    -300,  -300,    35,  -300,  -300,   305,  -300,  -300,   285,  -300,
    -300,   318,   293,   317,  -300,  -300,    10,  -300,   151,    10,
    -300,  -300,  -300,   223,  -300,  -300,   253,     9,    45,  -300,
    -300,  -300,    36,   240,    24,  -300,   175,  -300,  -300,  -300,
    -300,   336,   339,  -300,  -300,  -300,  -300,   333,   167,   215,
     224,   341,  -300,  -300,  -300,   236,  -300,  -300,  -300,  -300,
    -300,   346,  -300,  -300,  -300,  -300,  -300,    21,   342,   235,
     351,   354,   345,  -300,  -300,   322,    62,   283,   352,   356,
    -300,  -300,  -300,  -300,   353,   359,   324,   289,   287,  -300,
     266,   375,  -300,   372,    49,  -300,    18,   201,  -300,  -300,
    -300,   274,  -300,  -300,    10,    10,   252,   -14,  -300,   330,
    -300,  -300,    12,    12,    70,  -300,   322,  -300,   374,   385,
     223,  -300,  -300,  -300,  -300,   383,  -300,   186,   -36,    36,
     319,  -300,  -300,  -300,  -300,   379,  -300,  -300,  -300,   357,
    -300,  -300,  -300,  -300,  -300,  -300,  -300,   387,  -300,  -300,
    -300,  -300,  -300,   391,   164,   317,  -300,  -300,   -27,   295,
     337,   396,  -300,  -300,   357,   397,  -300,   304,    87,  -300,
     410,     2,   401,   303,   176,  -300,    29,   413,   414,  -300,
      34,   153,  -300,   372,     6,  -300,   -19,  -300,  -300,    10,
    -300,    10,    10,  -300,  -300,  -300,  -300,  -300,  -300,   412,
       6,    12,    12,  -300,  -300,  -300,  -300,  -300,   317,  -300,
    -300,  -300,    24,    10,  -300,    -9,  -300,   408,   309,    29,
     306,   289,   289,   400,    40,  -300,  -300,  -300,  -300,  -300,
     415,  -300,   420,  -300,  -300,   289,  -300,   317,  -300,  -300,
     289,   357,  -300,  -300,   314,   316,  -300,  -300,    94,   417,
    -300,  -300,   317,  -300,  -300,  -300,   421,  -300,  -300,   122,
    -300,  -300,   138,   331,  -300,   317,  -300,    59,   321,   335,
     326,   125,   293,   426,  -300,   -27,  -300,   402,  -300,   438,
     439,  -300,   -27,  -300,   228,   289,   -27,  -300,  -300,  -300,
    -300,   430,     8,   440,   440,   426,  -300,   332,  -300,   338,
       6,  -300,  -300,  -300,   431,   340,  -300,  -300,  -300,  -300,
    -300,  -300,   334,  -300,   201,   322,   322,  -300,  -300,  -300,
     394,     2,  -300,   289,  -300,  -300,  -300,   444,   444,   343,
    -300,   317,  -300,  -300,  -300,  -300,  -300,   432,   400,   400,
    -300,  -300,   289,    87,  -300,  -300,  -300,  -300,   449,   -27,
     -27,  -300,  -300,   360,  -300,  -300,   441,  -300,   200,  -300,
    -300,  -300
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,   376,
    -300,  -300,  -300,  -300,  -300,  -300,   -96,  -300,  -300,  -300,
    -300,  -300,   350,   433,  -300,  -300,  -300,  -300,  -300,  -300,
    -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,
    -170,  -300,  -300,  -300,     7,    97,  -300,  -300,  -300,   307,
     347,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,
    -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,   209,    27,
    -300,  -300,  -300,   308,   207,  -157,  -300,  -300,  -300,  -300,
    -300,   -82,  -300,  -300,   241,  -260,  -300,  -277,  -300,  -300,
    -300,  -300,    66,  -203,     4,  -300,    47,  -300,  -222,   160,
     301,   150,  -143,  -300,   230,  -300,  -299,   144,  -300,  -300,
    -300,    48,   239,  -176,  -300,  -300,  -300,  -300,  -300,  -300,
    -300,  -300,  -300,  -300,  -300,  -300,  -300,  -300,    52,  -300,
    -300,  -300,  -300,   -86,  -268,    26,  -113
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -324
static const yytype_int16 yytable[] =
{
     176,   200,   288,   140,   184,   313,   244,   261,   354,   231,
     232,   454,   382,   231,   232,   387,   383,  -323,   143,   384,
     385,   233,   144,    -5,   143,   233,   360,   213,   144,  -200,
    -200,   387,   158,   159,  -200,   143,   279,   214,   220,   144,
     280,   160,   161,   334,   158,   159,   376,   378,   171,   101,
     248,  -197,  -319,   160,   161,  -317,   102,    75,   178,   179,
     319,   400,   433,   259,    85,   319,   331,   332,   127,   128,
     201,   320,   434,     4,  -285,   289,   180,   341,   181,   172,
     321,   353,   103,   322,   369,   321,   386,  -285,   322,   407,
     419,   315,   316,    86,   391,   145,   146,   413,   147,  -285,
     403,   145,   146,   421,   147,   428,   404,  -200,  -200,   202,
    -200,    13,   145,   234,   171,   207,   251,    76,   432,   281,
      79,   455,   235,   252,   148,   290,   236,     1,   249,   149,
     236,   387,   302,    78,   175,   149,   162,    79,    83,   104,
     139,  -200,   221,   379,   121,   172,   149,   105,   162,   319,
     253,   254,   139,  -285,   106,   397,   398,   129,   189,    27,
      82,   190,   171,   310,   105,   109,   208,   209,   191,   321,
    -239,   106,   322,   270,   111,    94,   392,   271,   393,   394,
      99,    14,   374,   110,   476,   261,   113,   114,   333,   375,
      97,    79,   355,   172,   122,   123,   365,   125,   242,   243,
     401,    15,   366,   425,   319,   248,    16,    17,    18,   426,
     118,    15,   192,   193,   194,   195,    16,    17,    18,   351,
     352,   272,   467,   124,   321,   273,    19,   322,    27,   380,
     274,   430,   136,   137,   275,   431,    19,   321,   409,   410,
     322,   126,   468,   469,   170,     5,   130,    -6,   183,    -6,
      -6,   131,    -6,    -6,    -6,   317,   232,   420,   339,   340,
     133,    -6,    -6,   132,    -6,   -11,   -11,    -6,   -11,   139,
     -13,   135,    -6,    -6,    -6,   262,   263,   -11,   185,   -11,
     448,   449,   156,   -11,   -11,   -11,   264,   -11,   -11,   265,
     -11,   186,    -6,    -6,   -11,   -11,   489,   490,   479,   480,
     187,   197,   451,   198,   199,   -11,   -11,   206,   201,   443,
     -11,   -11,   -11,   -11,   -11,   211,   447,  -114,   215,   219,
     452,   222,   223,   225,   229,     6,   245,    -6,    -6,    96,
      -6,   227,   100,     7,   -11,   -11,   112,   -11,   -11,   259,
     116,   117,   267,   119,   120,   268,   269,   276,   277,   278,
      28,    29,   283,    30,   284,   282,    -6,   285,   286,   481,
      -6,    -6,    31,   248,    32,   291,   294,   292,    33,    34,
      35,   293,    36,    37,   295,    38,   296,   482,   304,    39,
      40,   305,   175,   484,   485,   307,   314,   335,   336,   338,
      41,    42,   345,   343,   346,    43,    44,    45,    46,    47,
     348,   251,  -252,  -252,   350,   356,   323,   324,   325,   359,
     358,   362,   361,   367,   371,   372,  -224,   377,   395,    48,
      49,   405,   406,    50,   408,   411,   417,   423,   416,   424,
     427,   429,   439,   322,   436,   326,   327,   328,   437,   441,
     444,   445,   446,   453,   463,   477,   456,   460,   470,   466,
     474,   461,   483,   464,   134,   486,   488,   188,   475,   337,
     473,    95,   418,   247,   205,   465,   342,   312,   440,   329,
     258,   478,   459,   381,   396,   318,   458,   472,   462
};

static const yytype_uint16 yycheck[] =
{
      96,   114,   205,    85,   100,   227,   149,   164,   285,     3,
       4,     3,     6,     3,     4,   314,    10,    15,     6,    13,
      14,    15,    10,     0,     6,    15,   294,   123,    10,     5,
       6,   330,     8,     9,    10,     6,    15,   123,     3,    10,
      19,    17,    18,   246,     8,     9,   306,    13,    75,     3,
      41,     6,    13,    17,    18,    15,    10,    26,    87,    88,
      79,   338,     3,    99,    20,    79,   242,   243,    11,    12,
      55,    85,    13,     0,    25,    13,   105,   113,   107,   106,
      99,   284,    36,   102,    82,    99,    80,    38,   102,   349,
     367,   234,   235,    49,   113,    83,    84,    57,    86,    50,
     109,    83,    84,   371,    86,   382,   115,    83,    84,    94,
      86,    13,    83,   103,    75,    13,    71,   104,   395,    98,
       3,   113,   112,    78,   112,    63,   120,   104,   119,   117,
     120,   430,   218,   105,    95,   117,   112,     3,    25,    93,
     116,   117,   107,   109,    10,   106,   117,   101,   112,    79,
     105,   106,   116,   104,   108,   331,   332,   100,     3,     9,
     104,     6,    75,   114,   101,     3,    64,    65,    13,    99,
      15,   108,   102,     6,     6,    25,   319,    10,   321,   322,
      30,     3,     6,    33,   461,   342,    36,    37,   118,    13,
      10,     3,   288,   106,    44,    45,   109,    47,    47,    48,
     343,    23,   298,   109,    79,    41,    28,    29,    30,   115,
      13,    23,    57,    58,    59,    60,    28,    29,    30,    55,
      56,     6,   444,     3,    99,    10,    48,   102,    78,   311,
       6,   109,    31,    32,    10,   113,    48,    99,   351,   352,
     102,     3,   445,   446,    94,     1,    13,     3,    98,     5,
       6,   104,     8,     9,    10,     3,     4,   370,    72,    73,
     104,    17,    18,    93,    20,    21,    22,    23,    24,   116,
      26,    45,    28,    29,    30,   100,   101,    33,     3,    35,
      52,    53,    50,    39,    40,    41,   111,    43,    44,   114,
      46,     3,    48,    49,    50,    51,    96,    97,   468,   469,
       3,    15,   415,    13,    15,    61,    62,    37,    55,   405,
      66,    67,    68,    69,    70,    15,   412,    15,    64,     6,
     416,    16,    37,     5,     7,    81,   103,    83,    84,    28,
      86,    38,    31,    89,    90,    91,    35,    93,    94,    99,
      39,    40,     6,    42,    43,     6,    13,     6,   112,     3,
      21,    22,   117,    24,     3,    13,   112,     3,    13,   472,
     116,   117,    33,    41,    35,    82,    13,    15,    39,    40,
      41,    15,    43,    44,    15,    46,    52,   473,   112,    50,
      51,     6,    95,   479,   480,    13,   112,    13,     3,     6,
      61,    62,    13,    74,    37,    66,    67,    68,    69,    70,
      13,    71,    72,    73,    13,   110,    76,    77,    78,    13,
      73,   107,    15,     3,    13,   112,     3,     3,     6,    90,
      91,    13,   113,    94,   118,    25,     6,   113,    13,   113,
      13,    10,   106,   102,   113,   105,   106,   107,   103,    13,
      38,     3,     3,    13,    13,    13,     6,   115,    54,   115,
       6,   113,     3,   113,    78,    95,    15,   107,   115,   250,
     453,    28,   365,   156,   117,   438,   259,   226,   402,   239,
     162,   467,   425,   313,   330,   236,   424,   451,   430
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   104,   122,   123,     0,     1,    81,    89,   124,   125,
     127,   128,   134,    13,     3,    23,    28,    29,    30,    48,
     129,   130,   131,   132,   144,   145,   146,   222,    21,    22,
      24,    33,    35,    39,    40,    41,    43,    44,    46,    50,
      51,    61,    62,    66,    67,    68,    69,    70,    90,    91,
      94,   133,   135,   136,   141,   148,   150,   151,   153,   158,
     159,   160,   162,   169,   175,   177,   179,   180,   181,   182,
     242,   246,   247,   248,   250,    26,   104,   126,   105,     3,
     221,   222,   104,    25,   218,    20,    49,   185,   186,   187,
     188,   190,   191,   238,   222,   144,   221,    10,   149,   222,
     221,     3,    10,    36,    93,   101,   108,   142,   143,     3,
     222,     6,   221,   222,   222,   251,   221,   221,    13,   221,
     221,    10,   222,   222,     3,   222,     3,    11,    12,   100,
      13,   104,    93,   104,   130,    45,    31,    32,   219,   116,
     202,   203,   239,     6,    10,    83,    84,    86,   112,   117,
     205,   206,   207,   234,   235,   237,    50,   210,     8,     9,
      17,    18,   112,   194,   195,   196,   197,   199,   200,   202,
     222,    75,   106,   137,   138,    95,   137,   254,    87,    88,
     105,   107,   152,   222,   137,     3,     3,     3,   143,     3,
       6,    13,    57,    58,    59,    60,   157,    15,    13,    15,
     257,    55,    94,   170,   171,   171,    37,    13,    64,    65,
     178,    15,   252,   137,   254,    64,   184,   163,   164,     6,
       3,   107,    16,    37,   240,     5,   204,    38,   213,     7,
     208,     3,     4,    15,   103,   112,   120,   223,   224,   227,
     230,   233,    47,    48,   223,   103,   189,   170,    41,   119,
     214,    71,    78,   105,   106,   201,   225,   226,   194,    99,
     192,   196,   100,   101,   111,   114,   198,     6,     6,    13,
       6,    10,     6,    10,     6,    10,     6,   112,     3,    15,
      19,    98,    13,   117,     3,     3,    13,   216,   214,    13,
      63,    82,    15,    15,    13,    15,    52,   183,   165,   166,
     167,   257,   254,   243,   112,     6,   147,    13,   215,   220,
     114,   241,   205,   219,   112,   223,   223,     3,   233,    79,
      85,    99,   102,    76,    77,    78,   105,   106,   107,   225,
     229,   234,   234,   118,   214,    13,     3,   189,     6,    72,
      73,   113,   195,    74,   209,    13,    37,   255,    13,   154,
      13,    55,    56,   214,   208,   137,   110,   176,    73,    13,
     255,    15,   107,   139,   140,   109,   137,     3,   168,    82,
     256,    13,   112,   244,     6,    13,   206,     3,    13,   109,
     202,   220,     6,    10,    13,    14,    80,   227,   228,   231,
     232,   113,   223,   223,   223,     6,   228,   234,   234,   211,
     208,   223,   193,   109,   115,    13,   113,   206,   118,   257,
     257,    25,   161,    57,   172,   253,    13,     6,   166,   208,
     257,   255,   245,   113,   113,   109,   115,    13,   208,    10,
     109,   113,   208,     3,    13,   236,   113,   103,   212,   106,
     213,    13,   217,   137,    38,     3,     3,   137,    52,    53,
     173,   257,   137,    13,     3,   113,     6,   249,   249,   217,
     115,   113,   232,    13,   113,   190,   115,   219,   214,   214,
      54,   174,   256,   165,     6,   115,   208,    13,   215,   161,
     161,   257,   137,     3,   137,   137,    95,   155,    15,    96,
      97,   156
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 352 "parser.y"
    { prepare_parse(); ;}
    break;

  case 3:
#line 353 "parser.y"
    { if (generate_code)
                                           addHistoryLine();
                                         resetQueryBuffer();
                                         YYACCEPT; ;}
    break;

  case 4:
#line 357 "parser.y"
    { YYACCEPT; ;}
    break;

  case 5:
#line 358 "parser.y"
    { YYACCEPT; ;}
    break;

  case 6:
#line 361 "parser.y"
    { prepare_input(); ;}
    break;

  case 7:
#line 362 "parser.y"
    { after_CorpusCommand((yyvsp[(2) - (3)].cl)); ;}
    break;

  case 8:
#line 364 "parser.y"
    { 
                      if ((yyvsp[(2) - (2)].ival) == query_lock) {
                        query_lock = 0;
                      }
                      else {
                        fprintf(stderr, "ALERT! Query lock violation.\n");
                        printf("\n"); /* so CQP.pm won't block -- should no longer be needed after switching to .EOL. mechanism */
                        exit(1);
                      }
                    ;}
    break;

  case 11:
#line 375 "parser.y"
    {if (query_lock) {warn_query_lock_violation(); YYABORT;} ;}
    break;

  case 12:
#line 376 "parser.y"
    { ;}
    break;

  case 13:
#line 377 "parser.y"
    {if (query_lock) {warn_query_lock_violation(); YYABORT;} ;}
    break;

  case 14:
#line 378 "parser.y"
    { exit_cqp++; ;}
    break;

  case 15:
#line 379 "parser.y"
    { /* in case of syntax errors, don't save 
                                              history file */
                                           synchronize();
                                           resetQueryBuffer();
                                           YYABORT; /* Oli did this:   yyerrok; */
                                           /* but we don't want to continue processing a line, when part of it failed */
                                         ;}
    break;

  case 16:
#line 388 "parser.y"
    { (yyval.cl) = (yyvsp[(1) - (1)].cl); ;}
    break;

  case 17:
#line 390 "parser.y"
    { (yyval.cl) = in_CorpusCommand((yyvsp[(1) - (3)].strval), (yyvsp[(3) - (3)].cl)); ;}
    break;

  case 18:
#line 394 "parser.y"
    { (yyval.cl) = in_UnnamedCorpusCommand((yyvsp[(1) - (2)].cl)); ;}
    break;

  case 19:
#line 397 "parser.y"
    { if (query_lock) {warn_query_lock_violation(); YYABORT;} (yyval.cl) = ActivateCorpus((yyvsp[(1) - (1)].cl)); ;}
    break;

  case 20:
#line 398 "parser.y"
    { (yyval.cl) = after_CorpusSetExpr((yyvsp[(1) - (1)].cl)); ;}
    break;

  case 21:
#line 399 "parser.y"
    { prepare_Query(); ;}
    break;

  case 22:
#line 400 "parser.y"
    { (yyval.cl) = after_Query((yyvsp[(2) - (2)].cl)); ;}
    break;

  case 46:
#line 430 "parser.y"
    { printf("-::-EOL-::-\n"); fflush(stdout); ;}
    break;

  case 47:
#line 434 "parser.y"
    { do_cat((yyvsp[(2) - (3)].cl), &((yyvsp[(3) - (3)].redir)), 0, -1); ;}
    break;

  case 48:
#line 437 "parser.y"
    { do_cat((yyvsp[(2) - (7)].cl), &((yyvsp[(7) - (7)].redir)), (yyvsp[(4) - (7)].ival), (yyvsp[(6) - (7)].ival)); ;}
    break;

  case 49:
#line 440 "parser.y"
    { if (generate_code) 
                              do_cat((yyvsp[(2) - (3)].cl), &((yyvsp[(3) - (3)].redir)), 0, -1);
                            drop_temp_corpora();
                          ;}
    break;

  case 50:
#line 447 "parser.y"
    { do_save((yyvsp[(2) - (3)].cl), &((yyvsp[(3) - (3)].redir))); cl_free((yyvsp[(3) - (3)].redir).name); ;}
    break;

  case 52:
#line 451 "parser.y"
    { (yyval.redir).name = (char *)NULL;
                                          (yyval.redir).mode = (char *)NULL;
                                          (yyval.redir).stream = stdout;
                                          (yyval.redir).is_pipe = 0;
                                        ;}
    break;

  case 53:
#line 458 "parser.y"
    { (yyval.redir).name = (yyvsp[(2) - (2)].strval);
                                          (yyval.redir).mode = "w";
                                          (yyval.redir).stream = NULL;
                                          (yyval.redir).is_pipe = 0;
                                        ;}
    break;

  case 54:
#line 463 "parser.y"
    { (yyval.redir).name = (yyvsp[(2) - (2)].strval);
                                          (yyval.redir).mode = "a";
                                          (yyval.redir).stream = NULL;
                                          (yyval.redir).is_pipe = 0;
                                        ;}
    break;

  case 56:
#line 472 "parser.y"
    { (yyval.in_redir).name = (char *)NULL;
                                          (yyval.in_redir).stream = stdin;
                                          (yyval.in_redir).is_pipe = 0;
                                        ;}
    break;

  case 57:
#line 478 "parser.y"
    { (yyval.in_redir).name = (yyvsp[(2) - (2)].strval);
                                          (yyval.in_redir).stream = NULL;
                                          (yyval.in_redir).is_pipe = 0;
                                        ;}
    break;

  case 58:
#line 484 "parser.y"
    { 
                                          show_corpora_files(UNDEF);
                                        ;}
    break;

  case 59:
#line 487 "parser.y"
    { 
                                          if (strncasecmp((yyvsp[(2) - (2)].strval), "var", 3) == 0) {
                                            do_PrintAllVariables();
                                          }
                                          else if ((strncasecmp((yyvsp[(2) - (2)].strval), "sys", 3) == 0) || (strncasecmp((yyvsp[(2) - (2)].strval), "corp", 4) == 0)) {
                                            show_corpora_files(SYSTEM);
                                          }
                                          else if ((strncasecmp((yyvsp[(2) - (2)].strval), "sub", 3) == 0) || (strcasecmp((yyvsp[(2) - (2)].strval), "named") == 0) || (strcasecmp((yyvsp[(2) - (2)].strval), "queries") == 0)) {
                                            show_corpora_files(SUB);    
                                          }
                                          else {
                                            cqpmessage(Error, "show what?");
                                          }
                                        ;}
    break;

  case 61:
#line 504 "parser.y"
    { PrintContextDescriptor(&CD); ;}
    break;

  case 64:
#line 514 "parser.y"
    { do_attribute_show((yyvsp[(2) - (2)].strval), 1); ;}
    break;

  case 65:
#line 515 "parser.y"
    { do_attribute_show((yyvsp[(2) - (2)].strval), 0); ;}
    break;

  case 66:
#line 518 "parser.y"
    { if (query_lock) {warn_query_lock_violation(); YYABORT;} (yyval.cl) = do_setop((yyvsp[(1) - (3)].rngsetop), (yyvsp[(2) - (3)].cl), (yyvsp[(3) - (3)].cl)); ;}
    break;

  case 68:
#line 522 "parser.y"
    { (yyval.rngsetop) = RUnion; ;}
    break;

  case 69:
#line 523 "parser.y"
    { (yyval.rngsetop) = RUnion; ;}
    break;

  case 70:
#line 524 "parser.y"
    { (yyval.rngsetop) = RIntersection; ;}
    break;

  case 71:
#line 525 "parser.y"
    { (yyval.rngsetop) = RDiff; ;}
    break;

  case 72:
#line 531 "parser.y"
    { 
                                          do_start_timer();
                                          prepare_do_subset((yyvsp[(2) - (4)].cl), (yyvsp[(4) - (4)].field));  
                                          next_environment();   /* create environment for pattern compilation (will be freed in prepare_input() before next command) */
                                        ;}
    break;

  case 73:
#line 536 "parser.y"
    { 
                                          if (generate_code) {
                                            (yyval.cl) = do_subset((yyvsp[(4) - (6)].field), (yyvsp[(6) - (6)].boolt));
                                            do_timing("Subset computed");
                                          }
                                          else 
                                            (yyval.cl) = NULL;
                                        ;}
    break;

  case 74:
#line 547 "parser.y"
    {  ;}
    break;

  case 75:
#line 550 "parser.y"
    { if ((yyvsp[(2) - (2)].cl))
                                            dropcorpus((yyvsp[(2) - (2)].cl));
                                        ;}
    break;

  case 76:
#line 553 "parser.y"
    { if ((yyvsp[(1) - (1)].cl))
                                            dropcorpus((yyvsp[(1) - (1)].cl));
                                        ;}
    break;

  case 77:
#line 558 "parser.y"
    { do_PrintVariableValue((yyvsp[(2) - (2)].strval)); 
                                    free((yyvsp[(2) - (2)].strval));
                                  ;}
    break;

  case 78:
#line 565 "parser.y"
    { do_SetVariableValue((yyvsp[(2) - (3)].strval), 
                                                        (yyvsp[(3) - (3)].varsetting).operator, 
                                                        (yyvsp[(3) - (3)].varsetting).variableValue); 
                                    free((yyvsp[(2) - (3)].strval));
                                    free((yyvsp[(3) - (3)].varsetting).variableValue);
                                  ;}
    break;

  case 79:
#line 572 "parser.y"
    {
                                    do_AddSubVariables((yyvsp[(2) - (4)].strval), /*add*/1, (yyvsp[(4) - (4)].strval));
                                    free((yyvsp[(2) - (4)].strval));
                                    free((yyvsp[(4) - (4)].strval));
                                  ;}
    break;

  case 80:
#line 578 "parser.y"
    {
                                    do_AddSubVariables((yyvsp[(2) - (4)].strval), /*sub*/0, (yyvsp[(4) - (4)].strval));
                                    free((yyvsp[(2) - (4)].strval));
                                    free((yyvsp[(4) - (4)].strval));
                                  ;}
    break;

  case 81:
#line 584 "parser.y"
    {
                                    char *temp = cl_strdup("");
                                    do_SetVariableValue((yyvsp[(2) - (4)].strval), '=', temp);         /* cheap trick, this is :o) */
                                    free(temp);
                                    do_AddSubVariables((yyvsp[(2) - (4)].strval), /*add*/1, (yyvsp[(4) - (4)].strval));
                                    free((yyvsp[(2) - (4)].strval));
                                    free((yyvsp[(4) - (4)].strval));
                                  ;}
    break;

  case 82:
#line 595 "parser.y"
    { (yyval.varsetting).variableValue = (yyvsp[(2) - (2)].strval); (yyval.varsetting).operator = '<'; ;}
    break;

  case 83:
#line 596 "parser.y"
    { (yyval.varsetting).variableValue = (yyvsp[(2) - (2)].strval); (yyval.varsetting).operator = '='; ;}
    break;

  case 84:
#line 597 "parser.y"
    { (yyval.varsetting).variableValue = (yyvsp[(2) - (2)].strval); (yyval.varsetting).operator = '+'; ;}
    break;

  case 85:
#line 598 "parser.y"
    { (yyval.varsetting).variableValue = (yyvsp[(2) - (2)].strval); (yyval.varsetting).operator = '-'; ;}
    break;

  case 86:
#line 601 "parser.y"
    { char *msg;

                                          if ((yyvsp[(3) - (3)].varval).cval != NULL && (yyvsp[(3) - (3)].varval).ival >= 0) {
                                            msg = set_context_option_value((yyvsp[(2) - (3)].strval), (yyvsp[(3) - (3)].varval).cval, (yyvsp[(3) - (3)].varval).ival);
                                          }
                                          else if ((yyvsp[(3) - (3)].varval).cval != NULL) {
                                            /* get rid of quotes at start and end of value */
                                            /* -- removed because quotes should be stripped by lexer ({string} rule in parser.l) */
                                            /*
                                            if (($3.cval[0] == '"') && ($3.cval[strlen($3.cval)-1] == '"')
                                                || ($3.cval[0] == '\'') && ($3.cval[strlen($3.cval)-1] == '\'') ) {
                                              

                                              $3.cval[strlen($3.cval)-1] = '\0';
                                              $3.cval = $3.cval + 1;
                                            }
                                            */
                                            msg = set_string_option_value((yyvsp[(2) - (3)].strval), (yyvsp[(3) - (3)].varval).cval);
                                          }
                                          else
                                            msg = set_integer_option_value((yyvsp[(2) - (3)].strval), (yyvsp[(3) - (3)].varval).ival);

                                          if (msg != NULL)
                                            cqpmessage(Warning,
                                                       "Option set error:\n%s", msg);
                                        ;}
    break;

  case 87:
#line 627 "parser.y"
    { int opt;

                                          if ((opt = find_option((yyvsp[(2) - (2)].strval))) >= 0)
                                            print_option_value(opt);
                                          else
                                            cqpmessage(Warning,
                                                     "Unknown option: ``%s''\n", (yyvsp[(2) - (2)].strval));
                                        ;}
    break;

  case 88:
#line 635 "parser.y"
    {
                                          print_option_values();
                                        ;}
    break;

  case 89:
#line 638 "parser.y"
    { do_set_target((yyvsp[(2) - (4)].cl), (yyvsp[(3) - (4)].field), (yyvsp[(4) - (4)].field)); ;}
    break;

  case 90:
#line 639 "parser.y"
    { do_set_target((yyvsp[(2) - (4)].cl), (yyvsp[(3) - (4)].field), NoField); ;}
    break;

  case 91:
#line 640 "parser.y"
    {
                                          if (generate_code) {
                                            old_query_corpus = query_corpus;
                                            query_corpus = (yyvsp[(2) - (4)].cl);  /* set query_corpus for compiling the ExtConstraint pattern */
                                            next_environment(); /* create environment for pattern compilation (will be freed in prepare_input() before next command) */
                                            do_start_timer();
                                          }
                                        ;}
    break;

  case 92:
#line 650 "parser.y"
    {
                                          do_set_complex_target((yyvsp[(2) - (11)].cl), (yyvsp[(3) - (11)].field), (yyvsp[(4) - (11)].search_strategy), (yyvsp[(6) - (11)].boolt), (yyvsp[(8) - (11)].direction), (yyvsp[(9) - (11)].ival), (yyvsp[(10) - (11)].strval), (yyvsp[(11) - (11)].base).field, (yyvsp[(11) - (11)].base).inclusive);
                                          if (generate_code) 
                                            do_timing("``set target ...'' completed");
                                        ;}
    break;

  case 93:
#line 657 "parser.y"
    {
                                          /* from (match|keyword|target) [inclusive|exclusive] */
                                          (yyval.base).field = (yyvsp[(2) - (3)].field);
                                          (yyval.base).inclusive = (yyvsp[(3) - (3)].ival);
                                        ;}
    break;

  case 94:
#line 662 "parser.y"
    { 
                                          (yyval.base).field = MatchField;
                                          (yyval.base).inclusive = 0;
                                        ;}
    break;

  case 95:
#line 668 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 96:
#line 669 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 97:
#line 670 "parser.y"
    { (yyval.ival) = 0; /* default is exclusive */ ;}
    break;

  case 98:
#line 673 "parser.y"
    { (yyval.varval).ival = (yyvsp[(1) - (1)].ival);
                                          (yyval.varval).cval = NULL;
                                          (yyval.varval).ok = 1;
                                        ;}
    break;

  case 99:
#line 677 "parser.y"
    { (yyval.varval).ival = -1;
                                          (yyval.varval).cval = (yyvsp[(1) - (1)].strval);
                                          (yyval.varval).ok = 1;
                                        ;}
    break;

  case 100:
#line 681 "parser.y"
    {
                                          (yyval.varval).ival = -1;
                                          (yyval.varval).cval = (yyvsp[(1) - (1)].strval);
                                          (yyval.varval).ok = 1;
                                        ;}
    break;

  case 101:
#line 686 "parser.y"
    { (yyval.varval).ival = 1; (yyval.varval).cval = NULL; (yyval.varval).ok = 1; ;}
    break;

  case 102:
#line 687 "parser.y"
    { (yyval.varval).ival = 1; (yyval.varval).cval = NULL; (yyval.varval).ok = 1; ;}
    break;

  case 103:
#line 688 "parser.y"
    { (yyval.varval).ival = 0; (yyval.varval).cval = NULL; (yyval.varval).ok = 1; ;}
    break;

  case 104:
#line 689 "parser.y"
    { (yyval.varval).ival = 0; (yyval.varval).cval = NULL; (yyval.varval).ok = 1; ;}
    break;

  case 105:
#line 690 "parser.y"
    {
                                          (yyval.varval).ival = (yyvsp[(1) - (2)].ival);
                                          (yyval.varval).cval = (yyvsp[(2) - (2)].strval);
                                          (yyval.varval).ok = 1;
                                        ;}
    break;

  case 106:
#line 697 "parser.y"
    { do_exec((yyvsp[(2) - (2)].strval)); ;}
    break;

  case 107:
#line 700 "parser.y"
    { do_info((yyvsp[(2) - (2)].cl)); ;}
    break;

  case 108:
#line 701 "parser.y"
    { do_info(current_corpus); ;}
    break;

  case 109:
#line 706 "parser.y"
    { 
                                  do_group((yyvsp[(2) - (10)].cl), (yyvsp[(3) - (10)].Anchor).anchor, (yyvsp[(3) - (10)].Anchor).offset, (yyvsp[(4) - (10)].strval), (yyvsp[(6) - (10)].Anchor).anchor, (yyvsp[(6) - (10)].Anchor).offset, (yyvsp[(7) - (10)].strval), (yyvsp[(8) - (10)].ival), (yyvsp[(9) - (10)].ival), &((yyvsp[(10) - (10)].redir))); 
                                  cl_free((yyvsp[(10) - (10)].redir).name);
                                ;}
    break;

  case 110:
#line 712 "parser.y"
    { 
                                  do_group((yyvsp[(2) - (10)].cl), (yyvsp[(3) - (10)].Anchor).anchor, (yyvsp[(3) - (10)].Anchor).offset, (yyvsp[(4) - (10)].strval), (yyvsp[(6) - (10)].Anchor).anchor, (yyvsp[(6) - (10)].Anchor).offset, (yyvsp[(7) - (10)].strval), (yyvsp[(8) - (10)].ival), (yyvsp[(9) - (10)].ival), &((yyvsp[(10) - (10)].redir)));
                                  cl_free((yyvsp[(10) - (10)].redir).name);
                                ;}
    break;

  case 111:
#line 718 "parser.y"
    { 
                                  do_group2((yyvsp[(2) - (7)].cl), (yyvsp[(3) - (7)].Anchor).anchor, (yyvsp[(3) - (7)].Anchor).offset, (yyvsp[(4) - (7)].strval), (yyvsp[(5) - (7)].ival), (yyvsp[(6) - (7)].ival), &((yyvsp[(7) - (7)].redir)));
                                  cl_free((yyvsp[(7) - (7)].redir).name);
                                ;}
    break;

  case 112:
#line 725 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 113:
#line 726 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 114:
#line 732 "parser.y"
    { free_tabulation_list(); ;}
    break;

  case 115:
#line 734 "parser.y"
    { print_tabulation((yyvsp[(2) - (5)].cl), 0, INT_MAX, &((yyvsp[(5) - (5)].redir))); cl_free((yyvsp[(5) - (5)].redir).name); ;}
    break;

  case 116:
#line 736 "parser.y"
    { free_tabulation_list(); ;}
    break;

  case 117:
#line 738 "parser.y"
    { print_tabulation((yyvsp[(2) - (9)].cl), (yyvsp[(5) - (9)].ival), (yyvsp[(7) - (9)].ival), &((yyvsp[(9) - (9)].redir))); cl_free((yyvsp[(9) - (9)].redir).name); ;}
    break;

  case 118:
#line 742 "parser.y"
    { append_tabulation_item((yyvsp[(1) - (1)].tabulation_item)); ;}
    break;

  case 119:
#line 744 "parser.y"
    { append_tabulation_item((yyvsp[(3) - (3)].tabulation_item)); ;}
    break;

  case 120:
#line 748 "parser.y"
    {
                     (yyval.tabulation_item) = new_tabulation_item();
                     (yyval.tabulation_item)->attribute_name = (yyvsp[(2) - (2)].AttributeSpecification).name;
                     (yyval.tabulation_item)->flags   = (yyvsp[(2) - (2)].AttributeSpecification).flags;
                     (yyval.tabulation_item)->anchor1 = (yyvsp[(1) - (2)].AnchorPair).anchor1;
                     (yyval.tabulation_item)->offset1 = (yyvsp[(1) - (2)].AnchorPair).offset1;
                     (yyval.tabulation_item)->anchor2 = (yyvsp[(1) - (2)].AnchorPair).anchor2;
                     (yyval.tabulation_item)->offset2 = (yyvsp[(1) - (2)].AnchorPair).offset2;
                   ;}
    break;

  case 121:
#line 760 "parser.y"
    { (yyval.AnchorPair).anchor1 = (yyval.AnchorPair).anchor2 = (yyvsp[(1) - (1)].Anchor).anchor; (yyval.AnchorPair).offset1 = (yyval.AnchorPair).offset2 = (yyvsp[(1) - (1)].Anchor).offset; ;}
    break;

  case 122:
#line 762 "parser.y"
    { (yyval.AnchorPair).anchor1 = (yyvsp[(1) - (3)].Anchor).anchor; (yyval.AnchorPair).offset1 = (yyvsp[(1) - (3)].Anchor).offset;
                       (yyval.AnchorPair).anchor2 = (yyvsp[(3) - (3)].Anchor).anchor; (yyval.AnchorPair).offset2 = (yyvsp[(3) - (3)].Anchor).offset; ;}
    break;

  case 123:
#line 767 "parser.y"
    { (yyval.AttributeSpecification).name = (yyvsp[(1) - (2)].strval); (yyval.AttributeSpecification).flags = (yyvsp[(2) - (2)].ival); ;}
    break;

  case 124:
#line 769 "parser.y"
    { (yyval.AttributeSpecification).name = NULL; (yyval.AttributeSpecification).flags = 0; ;}
    break;

  case 125:
#line 779 "parser.y"
    { 
                  int ok;
                  if ((yyvsp[(2) - (3)].cl) && generate_code) {
                    do_start_timer();
                    ok = SortSubcorpus((yyvsp[(2) - (3)].cl), (yyvsp[(3) - (3)].sortclause), 0, NULL);
                    FreeSortClause((yyvsp[(3) - (3)].sortclause));
                    do_timing("Query result sorted");
                    if (autoshow && ok && ((yyvsp[(2) - (3)].cl)->size > 0))
                      catalog_corpus((yyvsp[(2) - (3)].cl), NULL, 0, -1, GlobalPrintMode);
                  }
                ;}
    break;

  case 126:
#line 791 "parser.y"
    {
                  int ok;
                  if ((yyvsp[(2) - (4)].cl) && generate_code) {
                    do_start_timer();
                    ok = SortSubcorpusRandomize((yyvsp[(2) - (4)].cl), (yyvsp[(4) - (4)].ival));
                    do_timing("Query result randomized");
                    if (autoshow && ok && ((yyvsp[(2) - (4)].cl)->size > 0))
                      catalog_corpus((yyvsp[(2) - (4)].cl), NULL, 0, -1, GlobalPrintMode);
                  }
                ;}
    break;

  case 127:
#line 802 "parser.y"
    { 
                  int ok;
                  if ((yyvsp[(2) - (5)].cl) && generate_code) {
                    ok = SortSubcorpus((yyvsp[(2) - (5)].cl), (yyvsp[(3) - (5)].sortclause), ((yyvsp[(4) - (5)].ival) >= 1) ? (yyvsp[(4) - (5)].ival) : 1, &((yyvsp[(5) - (5)].redir)));
                    FreeSortClause((yyvsp[(3) - (5)].sortclause));
                    cl_free((yyvsp[(5) - (5)].redir).name);
                  }
                ;}
    break;

  case 128:
#line 813 "parser.y"
    { (yyval.sortclause) = (yyvsp[(1) - (1)].sortclause); ;}
    break;

  case 129:
#line 814 "parser.y"
    { (yyval.sortclause) = NULL; ;}
    break;

  case 130:
#line 818 "parser.y"
    {
                  if (generate_code) {
                    (yyval.sortclause) = cl_malloc(sizeof(SortClauseBuffer));
                    (yyval.sortclause)->attribute_name  = (yyvsp[(2) - (6)].strval);
                    (yyval.sortclause)->flags           = (yyvsp[(3) - (6)].ival);
                    (yyval.sortclause)->anchor1         = (yyvsp[(4) - (6)].AnchorPair).anchor1;
                    (yyval.sortclause)->offset1         = (yyvsp[(4) - (6)].AnchorPair).offset1;
                    (yyval.sortclause)->anchor2         = (yyvsp[(4) - (6)].AnchorPair).anchor2;
                    (yyval.sortclause)->offset2         = (yyvsp[(4) - (6)].AnchorPair).offset2;
                    (yyval.sortclause)->sort_ascending  = (yyvsp[(5) - (6)].ival);
                    (yyval.sortclause)->sort_reverse    = (yyvsp[(6) - (6)].ival);
                  }
                  else
                    (yyval.sortclause) = NULL;
                ;}
    break;

  case 131:
#line 835 "parser.y"
    { (yyval.AnchorPair).anchor1 = (yyval.AnchorPair).anchor2 = (yyvsp[(2) - (2)].Anchor).anchor; (yyval.AnchorPair).offset1 = (yyval.AnchorPair).offset2 = (yyvsp[(2) - (2)].Anchor).offset; ;}
    break;

  case 132:
#line 837 "parser.y"
    { (yyval.AnchorPair).anchor1 = (yyvsp[(2) - (4)].Anchor).anchor; (yyval.AnchorPair).offset1 = (yyvsp[(2) - (4)].Anchor).offset;
                              (yyval.AnchorPair).anchor2 = (yyvsp[(4) - (4)].Anchor).anchor; (yyval.AnchorPair).offset2 = (yyvsp[(4) - (4)].Anchor).offset; ;}
    break;

  case 133:
#line 840 "parser.y"
    { (yyval.AnchorPair).anchor1 = MatchField;    (yyval.AnchorPair).offset1 = 0;
                              (yyval.AnchorPair).anchor2 = MatchEndField; (yyval.AnchorPair).offset2 = 0; ;}
    break;

  case 134:
#line 844 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 135:
#line 845 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 136:
#line 846 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 137:
#line 849 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 138:
#line 850 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 139:
#line 856 "parser.y"
    {
                                          do_reduce((yyvsp[(2) - (5)].cl), (yyvsp[(4) - (5)].ival), (yyvsp[(5) - (5)].ival));
                                        ;}
    break;

  case 140:
#line 860 "parser.y"
    {
                                          RangeSetop((yyvsp[(2) - (5)].cl), RMaximalMatches, NULL, NULL);
                                        ;}
    break;

  case 141:
#line 864 "parser.y"
    {
                                          do_cut((yyvsp[(2) - (3)].cl), 0, (yyvsp[(3) - (3)].ival)-1);
                                        ;}
    break;

  case 142:
#line 868 "parser.y"
    {
                                          do_cut((yyvsp[(2) - (4)].cl), (yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].ival));
                                        ;}
    break;

  case 143:
#line 873 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 144:
#line 874 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 145:
#line 878 "parser.y"
    {
                                          if ((yyvsp[(2) - (4)].cl) && generate_code) {
                                            do_delete_lines((yyvsp[(2) - (4)].cl), (yyvsp[(4) - (4)].field), SELECTED_LINES); 
                                          }
                                        ;}
    break;

  case 146:
#line 884 "parser.y"
    { 
                                          if ((yyvsp[(2) - (4)].cl) && generate_code) {
                                            do_delete_lines((yyvsp[(2) - (4)].cl), (yyvsp[(4) - (4)].field), UNSELECTED_LINES); 
                                          }
                                        ;}
    break;

  case 147:
#line 890 "parser.y"
    { 
                                          if ((yyvsp[(2) - (3)].cl) && generate_code) {
                                             do_delete_lines_num((yyvsp[(2) - (3)].cl), (yyvsp[(3) - (3)].Distance).mindist, (yyvsp[(3) - (3)].Distance).maxdist);
                                           }
                                         ;}
    break;

  case 148:
#line 897 "parser.y"
    { (yyval.Distance).mindist = (yyvsp[(1) - (1)].ival); (yyval.Distance).maxdist = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 149:
#line 898 "parser.y"
    { (yyval.Distance).mindist = (yyvsp[(1) - (3)].ival); (yyval.Distance).maxdist = (yyvsp[(3) - (3)].ival); ;}
    break;

  case 150:
#line 903 "parser.y"
    { do_sleep((yyvsp[(2) - (2)].ival)); ;}
    break;

  case 151:
#line 907 "parser.y"
    { do_size((yyvsp[(2) - (3)].cl), (yyvsp[(3) - (3)].field)); ;}
    break;

  case 152:
#line 908 "parser.y"
    { do_printVariableSize((yyvsp[(2) - (2)].strval)); free((yyvsp[(2) - (2)].strval)); ;}
    break;

  case 153:
#line 913 "parser.y"
    { do_dump((yyvsp[(2) - (3)].cl), 0, INT_MAX, &((yyvsp[(3) - (3)].redir))); cl_free((yyvsp[(3) - (3)].redir).name); ;}
    break;

  case 154:
#line 915 "parser.y"
    { do_dump((yyvsp[(2) - (7)].cl), (yyvsp[(4) - (7)].ival), (yyvsp[(6) - (7)].ival), &((yyvsp[(7) - (7)].redir))); cl_free((yyvsp[(7) - (7)].redir).name); ;}
    break;

  case 155:
#line 919 "parser.y"
    { do_undump((yyvsp[(2) - (5)].strval), (yyvsp[(3) - (5)].ival), !(yyvsp[(4) - (5)].ival), &((yyvsp[(5) - (5)].in_redir))); cl_free((yyvsp[(5) - (5)].in_redir).name); ;}
    break;

  case 156:
#line 922 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 157:
#line 923 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 158:
#line 927 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 159:
#line 929 "parser.y"
    { 
            if ((yyvsp[(2) - (2)].field) == TargetField) { (yyval.ival) = 1; }
            else { yyerror("Invalid extension anchor in undump command"); YYABORT; } 
          ;}
    break;

  case 160:
#line 934 "parser.y"
    { 
            if ( (((yyvsp[(2) - (3)].field) == TargetField) && ((yyvsp[(3) - (3)].field) == KeywordField))
                 || (((yyvsp[(3) - (3)].field) == TargetField) && ((yyvsp[(2) - (3)].field) == KeywordField))
               ) { (yyval.ival) = 2; }
            else { yyerror("Invalid extension anchor in undump command"); YYABORT; } 
          ;}
    break;

  case 162:
#line 946 "parser.y"
    {
                  if ((yyvsp[(1) - (3)].cl) && (yyvsp[(3) - (3)].sortclause) && (yyvsp[(1) - (3)].cl)->size > 0) {
                    SortSubcorpus((yyvsp[(1) - (3)].cl), (yyvsp[(3) - (3)].sortclause), 0, NULL);
                    FreeSortClause((yyvsp[(3) - (3)].sortclause));
                  }
                  (yyval.cl) = (yyvsp[(1) - (3)].cl);
                ;}
    break;

  case 166:
#line 962 "parser.y"
    { (yyval.cl) = do_StandardQuery((yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].ival)); ;}
    break;

  case 167:
#line 966 "parser.y"
    { (yyval.cl) = do_MUQuery((yyvsp[(2) - (4)].evalt), (yyvsp[(3) - (4)].ival), (yyvsp[(4) - (4)].ival)); ;}
    break;

  case 168:
#line 969 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 169:
#line 970 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 170:
#line 973 "parser.y"
    { if (generate_code) { CurEnv->match_label = labellookup(CurEnv->labels, "match", LAB_DEFINED, 1); } ;}
    break;

  case 171:
#line 974 "parser.y"
    { within_gc = 1; 
                                          if (generate_code) { CurEnv->matchend_label = labellookup(CurEnv->labels, "matchend", LAB_DEFINED, 1); } ;}
    break;

  case 172:
#line 976 "parser.y"
    { within_gc = 0; ;}
    break;

  case 173:
#line 977 "parser.y"
    { do_SearchPattern((yyvsp[(2) - (6)].evalt), (yyvsp[(4) - (6)].boolt)); ;}
    break;

  case 174:
#line 981 "parser.y"
    { (yyval.evalt) = reg_disj((yyvsp[(1) - (3)].evalt), (yyvsp[(3) - (3)].evalt)); ;}
    break;

  case 175:
#line 982 "parser.y"
    { (yyval.evalt) = (yyvsp[(1) - (1)].evalt); ;}
    break;

  case 176:
#line 986 "parser.y"
    { (yyval.evalt) = reg_seq((yyvsp[(1) - (2)].evalt), (yyvsp[(2) - (2)].evalt)); ;}
    break;

  case 177:
#line 987 "parser.y"
    { (yyval.evalt) = (yyvsp[(1) - (1)].evalt); ;}
    break;

  case 178:
#line 990 "parser.y"
    { if (generate_code) {
                                           (yyval.evalt) = (yyvsp[(2) - (2)].evalt);
                                           (yyval.evalt)->node.left = (yyvsp[(1) - (2)].evalt);
                                           (yyval.evalt)->node.right = NULL;
                                         }
                                         else
                                           (yyval.evalt) = NULL;
                                       ;}
    break;

  case 179:
#line 998 "parser.y"
    { if (generate_code)
                                           NEW_EVALNODE((yyval.evalt), re_repeat, (yyvsp[(1) - (2)].evalt), NULL, 0, repeat_inf);
                                         else
                                           (yyval.evalt) = NULL;
                                       ;}
    break;

  case 180:
#line 1003 "parser.y"
    { if (generate_code)
                                           NEW_EVALNODE((yyval.evalt), re_repeat, (yyvsp[(1) - (2)].evalt), NULL, 1, repeat_inf);
                                         else
                                           (yyval.evalt) = NULL;
                                       ;}
    break;

  case 181:
#line 1008 "parser.y"
    { if (generate_code)
                                           NEW_EVALNODE((yyval.evalt), re_repeat, (yyvsp[(1) - (2)].evalt), NULL, 0, 1);
                                         else
                                           (yyval.evalt) = NULL;
                                       ;}
    break;

  case 182:
#line 1013 "parser.y"
    { (yyval.evalt) = (yyvsp[(1) - (1)].evalt); ;}
    break;

  case 183:
#line 1016 "parser.y"
    { (yyval.evalt) = (yyvsp[(2) - (3)].evalt); ;}
    break;

  case 184:
#line 1017 "parser.y"
    { if (generate_code)
                                            NEW_EVALLEAF((yyval.evalt), (yyvsp[(1) - (1)].index));
                                          else
                                            (yyval.evalt) = NULL;
                                        ;}
    break;

  case 185:
#line 1022 "parser.y"
    { if (generate_code)
                                            NEW_EVALLEAF((yyval.evalt), (yyvsp[(1) - (1)].index));
                                          else
                                            (yyval.evalt) = NULL;
                                        ;}
    break;

  case 186:
#line 1027 "parser.y"
    { if (generate_code)
                                            NEW_EVALLEAF((yyval.evalt), (yyvsp[(1) - (1)].index));
                                          else
                                            (yyval.evalt) = NULL;
                                        ;}
    break;

  case 187:
#line 1034 "parser.y"
    { if (generate_code)
                                            NEW_EVALNODE((yyval.evalt), re_repeat, NULL, NULL, (yyvsp[(2) - (3)].ival), (yyvsp[(2) - (3)].ival));
                                          else
                                            (yyval.evalt) = NULL;
                                        ;}
    break;

  case 188:
#line 1040 "parser.y"
    { if (generate_code)
                                            NEW_EVALNODE((yyval.evalt), re_repeat, NULL, NULL, (yyvsp[(2) - (5)].ival), (yyvsp[(4) - (5)].ival));
                                          else
                                            (yyval.evalt) = NULL;
                                        ;}
    break;

  case 189:
#line 1048 "parser.y"
    { (yyval.index) = do_AnchorPoint((yyvsp[(1) - (1)].field), 0); ;}
    break;

  case 190:
#line 1049 "parser.y"
    { (yyval.index) = do_AnchorPoint((yyvsp[(1) - (1)].field), 1); ;}
    break;

  case 191:
#line 1052 "parser.y"
    { (yyval.index) = do_XMLTag((yyvsp[(1) - (2)].strval), 0, 0, NULL, 0); ;}
    break;

  case 192:
#line 1054 "parser.y"
    { (yyval.index) = do_XMLTag((yyvsp[(1) - (5)].strval), 0, (yyvsp[(2) - (5)].ival), (yyvsp[(3) - (5)].strval), (yyvsp[(4) - (5)].ival)); ;}
    break;

  case 193:
#line 1055 "parser.y"
    { (yyval.index) = do_XMLTag((yyvsp[(1) - (1)].strval), 1, 0, NULL, 0); ;}
    break;

  case 194:
#line 1058 "parser.y"
    { (yyval.ival) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 195:
#line 1059 "parser.y"
    { (yyval.ival) = OP_EQUAL | OP_NOT; ;}
    break;

  case 196:
#line 1060 "parser.y"
    { (yyval.ival) = OP_EQUAL; ;}
    break;

  case 197:
#line 1061 "parser.y"
    { (yyval.ival) = OP_EQUAL; ;}
    break;

  case 198:
#line 1066 "parser.y"
    { (yyval.index) = do_NamedWfPattern((yyvsp[(1) - (3)].ival), (yyvsp[(2) - (3)].strval), (yyvsp[(3) - (3)].index)); ;}
    break;

  case 199:
#line 1070 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 200:
#line 1071 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 201:
#line 1074 "parser.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 202:
#line 1075 "parser.y"
    { (yyval.strval) = NULL; ;}
    break;

  case 203:
#line 1078 "parser.y"
    { (yyval.index) = do_WordformPattern((yyvsp[(1) - (1)].boolt), 0); ;}
    break;

  case 204:
#line 1079 "parser.y"
    { (yyval.index) = do_WordformPattern((yyvsp[(1) - (1)].boolt), 1); ;}
    break;

  case 205:
#line 1082 "parser.y"
    { (yyval.boolt) = do_StringConstraint((yyvsp[(1) - (2)].strval), (yyvsp[(2) - (2)].ival)); ;}
    break;

  case 206:
#line 1083 "parser.y"
    { (yyval.boolt) = NULL;
                                        if (!FindVariable((yyvsp[(1) - (1)].strval))) {
                                          cqpmessage(Error, 
                                                     "%s: no such variable", 
                                                     (yyvsp[(1) - (1)].strval));
                                          generate_code = 0;
                                        }
                                        else {
                                          (yyval.boolt) = do_SimpleVariableReference((yyvsp[(1) - (1)].strval));
                                        }
                                        free((yyvsp[(1) - (1)].strval));
                                      ;}
    break;

  case 207:
#line 1095 "parser.y"
    { (yyval.boolt) = (yyvsp[(2) - (3)].boolt); ;}
    break;

  case 208:
#line 1096 "parser.y"
    { if (generate_code) {
                                          NEW_BNODE((yyval.boolt));
                                          (yyval.boolt)->constnode.type = cnode;
                                          (yyval.boolt)->constnode.val  = 1;
                                        }
                                      ;}
    break;

  case 209:
#line 1104 "parser.y"
    { (yyval.boolt) = (yyvsp[(2) - (3)].boolt); ;}
    break;

  case 210:
#line 1105 "parser.y"
    { if (generate_code) {
                                          NEW_BNODE((yyval.boolt));
                                          (yyval.boolt)->constnode.type = cnode;
                                          (yyval.boolt)->constnode.val  = 1;
                                        }
                                      ;}
    break;

  case 211:
#line 1113 "parser.y"
    { int flags, i;
                                          flags = 0;

                                          for (i = 0; (yyvsp[(1) - (1)].strval)[i] != '\0'; i++) {
                                            switch ((yyvsp[(1) - (1)].strval)[i]) {
                                            case 'c':
                                              flags |= IGNORE_CASE;
                                              break;
                                            case 'd':
                                              flags |= IGNORE_DIAC;
                                              break;
                                            case 'l':
                                              flags = IGNORE_REGEX; /* literal */
                                              break;
                                            default:
                                              cqpmessage(Warning, "Unknown flag %s%c (ignored)", "%", (yyvsp[(1) - (1)].strval)[i]);
                                              break;
                                            }
                                          }

                                          /* %l supersedes all others */
                                          if (flags & IGNORE_REGEX) {
                                            if (flags != IGNORE_REGEX) {
                                              cqpmessage(Warning, "%s and %s flags cannot be combined with %s (ignored)",
                                                         "%c", "%d", "%l");
                                            }
                                            flags = IGNORE_REGEX;
                                          }

                                          (yyval.ival) = flags;
                                        ;}
    break;

  case 212:
#line 1144 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 213:
#line 1147 "parser.y"
    { (yyval.boolt) = (yyvsp[(2) - (2)].boolt); ;}
    break;

  case 214:
#line 1148 "parser.y"
    { (yyval.boolt) = NULL; ;}
    break;

  case 215:
#line 1153 "parser.y"
    { prepare_AlignmentConstraints((yyvsp[(3) - (3)].strval)); ;}
    break;

  case 216:
#line 1155 "parser.y"
    { if (generate_code)
                                           CurEnv->negated = (yyvsp[(5) - (6)].ival);
                                       ;}
    break;

  case 217:
#line 1158 "parser.y"
    { ;}
    break;

  case 218:
#line 1161 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 219:
#line 1162 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 220:
#line 1166 "parser.y"
    { if (generate_code) {
                    CurEnv->search_context.direction = (yyvsp[(2) - (3)].direction);
                    CurEnv->search_context.type = (yyvsp[(3) - (3)].context).type;
                    CurEnv->search_context.size = (yyvsp[(3) - (3)].context).size;
                    CurEnv->search_context.attrib = (yyvsp[(3) - (3)].context).attrib;
                  }
                ;}
    break;

  case 221:
#line 1173 "parser.y"
    { if (generate_code) {
                                            CurEnv->search_context.type  = word;
                                            CurEnv->search_context.size  = hard_boundary;
                                            CurEnv->search_context.attrib = NULL;
                                          }
                                        ;}
    break;

  case 222:
#line 1181 "parser.y"
    { (yyval.ival) = abs((yyvsp[(2) - (2)].ival)); ;}
    break;

  case 223:
#line 1182 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 224:
#line 1185 "parser.y"
    { (yyval.ival) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 225:
#line 1186 "parser.y"
    { (yyval.ival) = 1; ;}
    break;

  case 226:
#line 1189 "parser.y"
    { (yyval.ival) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 227:
#line 1190 "parser.y"
    { (yyval.ival) = 0; ;}
    break;

  case 228:
#line 1193 "parser.y"
    { (yyval.ival) = (yyvsp[(1) - (1)].ival); ;}
    break;

  case 229:
#line 1194 "parser.y"
    { (yyval.ival) = repeat_inf; ;}
    break;

  case 230:
#line 1198 "parser.y"
    { expansion.direction = (yyvsp[(2) - (4)].direction);
                                          expansion.type = (yyvsp[(4) - (4)].context).type;
                                          expansion.size = (yyvsp[(4) - (4)].context).size;
                                          expansion.attrib = (yyvsp[(4) - (4)].context).attrib;
                                        ;}
    break;

  case 231:
#line 1203 "parser.y"
    { expansion.direction = leftright;
                                          expansion.type = word;
                                          expansion.size = 0;
                                          expansion.attrib = NULL;
                                        ;}
    break;

  case 232:
#line 1210 "parser.y"
    { (yyval.direction) = left; ;}
    break;

  case 233:
#line 1211 "parser.y"
    { (yyval.direction) = right; ;}
    break;

  case 234:
#line 1212 "parser.y"
    { (yyval.direction) = leftright; ;}
    break;

  case 235:
#line 1215 "parser.y"
    { do_Description(&((yyval.context)), (yyvsp[(1) - (2)].ival), (yyvsp[(2) - (2)].strval)); ;}
    break;

  case 236:
#line 1216 "parser.y"
    { do_Description(&((yyval.context)), (yyvsp[(1) - (1)].ival), NULL); ;}
    break;

  case 237:
#line 1219 "parser.y"
    { (yyval.cl) = (yyvsp[(1) - (1)].cl); ;}
    break;

  case 238:
#line 1220 "parser.y"
    { (yyval.cl) = findcorpus("Last", UNDEF, 0); ;}
    break;

  case 239:
#line 1224 "parser.y"
    { CorpusList *cl;

                                          cqpmessage(Message, "CID: %s", (yyvsp[(1) - (1)].strval));

                                          if ((cl = findcorpus((yyvsp[(1) - (1)].strval), UNDEF, 1)) == NULL) {
                                            cqpmessage(Error,
                                                       "Corpus ``%s'' is undefined", (yyvsp[(1) - (1)].strval));
                                            generate_code = 0;
                                            (yyval.cl) = NULL;
                                          }
                                          else if (!access_corpus(cl)) {
                                            cqpmessage(Warning,
                                                       "Corpus ``%s'' can't be accessed", (yyvsp[(1) - (1)].strval));
                                            (yyval.cl) = NULL;
                                          }
                                          else
                                            (yyval.cl) = cl;
                                        ;}
    break;

  case 240:
#line 1244 "parser.y"
    { (yyval.boolt) = bool_implies((yyvsp[(1) - (3)].boolt), (yyvsp[(3) - (3)].boolt)); ;}
    break;

  case 241:
#line 1245 "parser.y"
    { (yyval.boolt) = bool_or((yyvsp[(1) - (3)].boolt), (yyvsp[(3) - (3)].boolt)); ;}
    break;

  case 242:
#line 1246 "parser.y"
    { (yyval.boolt) = bool_and((yyvsp[(1) - (3)].boolt), (yyvsp[(3) - (3)].boolt)); ;}
    break;

  case 243:
#line 1247 "parser.y"
    { (yyval.boolt) = (yyvsp[(2) - (3)].boolt); ;}
    break;

  case 244:
#line 1248 "parser.y"
    { (yyval.boolt) = bool_not((yyvsp[(2) - (2)].boolt)); ;}
    break;

  case 245:
#line 1249 "parser.y"
    { (yyval.boolt) = (yyvsp[(1) - (1)].boolt); ;}
    break;

  case 246:
#line 1252 "parser.y"
    { (yyval.boolt) = do_RelExpr((yyvsp[(1) - (3)].boolt), (yyvsp[(2) - (3)].boolo), (yyvsp[(3) - (3)].boolt)); ;}
    break;

  case 247:
#line 1254 "parser.y"
    {
                 if ((yyvsp[(2) - (4)].ival) & OP_NOT) {
                   (yyval.boolt) = do_RelExpr((yyvsp[(1) - (4)].boolt), cmp_neq, do_mval_string((yyvsp[(3) - (4)].strval), (yyvsp[(2) - (4)].ival), (yyvsp[(4) - (4)].ival)));
                 }
                 else {
                   (yyval.boolt) = do_RelExpr((yyvsp[(1) - (4)].boolt), cmp_eq,  do_mval_string((yyvsp[(3) - (4)].strval), (yyvsp[(2) - (4)].ival), (yyvsp[(4) - (4)].ival)));
                 }
              ;}
    break;

  case 248:
#line 1262 "parser.y"
    { (yyval.boolt) = do_RelExExpr((yyvsp[(1) - (1)].boolt)); ;}
    break;

  case 249:
#line 1265 "parser.y"
    {(yyval.ival) = OP_CONTAINS | (yyvsp[(1) - (2)].ival);;}
    break;

  case 250:
#line 1266 "parser.y"
    {(yyval.ival) = OP_MATCHES  | (yyvsp[(1) - (2)].ival);;}
    break;

  case 251:
#line 1269 "parser.y"
    {(yyval.ival) = OP_NOT;;}
    break;

  case 252:
#line 1270 "parser.y"
    {(yyval.ival) = 0;;}
    break;

  case 253:
#line 1273 "parser.y"
    { (yyval.boolt) = do_LabelReference((yyvsp[(1) - (1)].strval), 0); ;}
    break;

  case 254:
#line 1274 "parser.y"
    { (yyval.boolt) = do_LabelReference((yyvsp[(2) - (2)].strval), 1); ;}
    break;

  case 255:
#line 1275 "parser.y"
    { (yyval.boolt) = do_IDReference((yyvsp[(1) - (1)].strval), 0); ;}
    break;

  case 256:
#line 1276 "parser.y"
    { (yyval.boolt) = do_IDReference((yyvsp[(2) - (2)].strval), 1); ;}
    break;

  case 257:
#line 1277 "parser.y"
    { (yyval.boolt) = do_IDReference(cl_strdup(field_type_to_name((yyvsp[(1) - (1)].field))), 0); ;}
    break;

  case 258:
#line 1278 "parser.y"
    { (yyval.boolt) = (yyvsp[(1) - (1)].boolt); ;}
    break;

  case 259:
#line 1281 "parser.y"
    { (yyval.boolt) = (yyvsp[(1) - (1)].boolt); ;}
    break;

  case 260:
#line 1282 "parser.y"
    { (yyval.boolt) = do_flagged_string((yyvsp[(1) - (2)].strval), (yyvsp[(2) - (2)].ival)); ;}
    break;

  case 261:
#line 1284 "parser.y"
    { 
                                          (yyval.boolt) = do_flagged_re_variable((yyvsp[(2) - (4)].strval), (yyvsp[(4) - (4)].ival)); 
                                        ;}
    break;

  case 262:
#line 1287 "parser.y"
    { if (generate_code) {
                                            if (!FindVariable((yyvsp[(1) - (1)].strval))) {
                                              cqpmessage(Error, 
                                                         "%s: no such variable", 
                                                         (yyvsp[(1) - (1)].strval));
                                              generate_code = 0;
                                              (yyval.boolt) = NULL;
                                            }
                                            else {
                                              NEW_BNODE((yyval.boolt));
                                              (yyval.boolt)->type = var_ref;
                                              (yyval.boolt)->varref.varName = (yyvsp[(1) - (1)].strval);
                                            }
                                          }
                                          else
                                            (yyval.boolt) = NULL;
                                        ;}
    break;

  case 263:
#line 1304 "parser.y"
    { if (generate_code) {
                                            NEW_BNODE((yyval.boolt));
                                            (yyval.boolt)->type = int_leaf;
                                            (yyval.boolt)->leaf.ctype.iconst = (yyvsp[(1) - (1)].ival);
                                          }
                                          else
                                            (yyval.boolt) = NULL;
                                        ;}
    break;

  case 264:
#line 1312 "parser.y"
    { if (generate_code) {
                                            NEW_BNODE((yyval.boolt));
                                            (yyval.boolt)->type = float_leaf;
                                            (yyval.boolt)->leaf.ctype.fconst = (yyvsp[(1) - (1)].fval);
                                          }
                                          else
                                            (yyval.boolt) = NULL;
                                        ;}
    break;

  case 265:
#line 1322 "parser.y"
    { (yyval.boolo) = cmp_lt; ;}
    break;

  case 266:
#line 1323 "parser.y"
    { (yyval.boolo) = cmp_gt; ;}
    break;

  case 267:
#line 1324 "parser.y"
    { (yyval.boolo) = cmp_eq; ;}
    break;

  case 268:
#line 1325 "parser.y"
    { (yyval.boolo) = cmp_neq; ;}
    break;

  case 269:
#line 1326 "parser.y"
    { (yyval.boolo) = cmp_let; ;}
    break;

  case 270:
#line 1327 "parser.y"
    { (yyval.boolo) = cmp_get; ;}
    break;

  case 271:
#line 1330 "parser.y"
    { (yyval.boolt) = FunctionCall((yyvsp[(1) - (4)].strval), (yyvsp[(3) - (4)].apl)); ;}
    break;

  case 272:
#line 1333 "parser.y"
    { (yyval.apl) = (yyvsp[(1) - (1)].apl);
                                            ;}
    break;

  case 273:
#line 1336 "parser.y"
    { ActualParamList *last;

                                              if (generate_code) {
                                                assert((yyvsp[(1) - (3)].apl) != NULL);

                                                last = (yyvsp[(1) - (3)].apl);
                                                while (last->next != NULL)
                                                  last = last->next;
                                                last->next = (yyvsp[(3) - (3)].apl);
                                                (yyval.apl) = (yyvsp[(1) - (3)].apl);
                                              }
                                              else
                                                (yyval.apl) = NULL;
                                            ;}
    break;

  case 274:
#line 1352 "parser.y"
    { if (generate_code) {
                                             New((yyval.apl), ActualParamList);

                                             (yyval.apl)->param = (yyvsp[(1) - (1)].boolt);
                                             (yyval.apl)->next = NULL;
                                           }
                                           else
                                             (yyval.apl) = NULL;
                                         ;}
    break;

  case 275:
#line 1363 "parser.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 276:
#line 1366 "parser.y"
    { (yyval.evalt) = (yyvsp[(1) - (1)].evalt); ;}
    break;

  case 277:
#line 1367 "parser.y"
    { (yyval.evalt) = (yyvsp[(1) - (1)].evalt); ;}
    break;

  case 278:
#line 1368 "parser.y"
    { if (generate_code) {
                                            NEW_EVALLEAF((yyval.evalt), (yyvsp[(1) - (1)].index));
                                          }
                                          else
                                            (yyval.evalt) = NULL;
                                        ;}
    break;

  case 279:
#line 1380 "parser.y"
    { (yyval.evalt) = do_MeetStatement((yyvsp[(3) - (6)].evalt), (yyvsp[(4) - (6)].evalt), &((yyvsp[(5) - (6)].context))); ;}
    break;

  case 280:
#line 1384 "parser.y"
    { (yyval.context).type = word;
                                          (yyval.context).size = (yyvsp[(1) - (2)].ival);
                                          (yyval.context).size2 = (yyvsp[(2) - (2)].ival);
                                          (yyval.context).attrib = NULL;
                                        ;}
    break;

  case 281:
#line 1389 "parser.y"
    { do_StructuralContext(&((yyval.context)), (yyvsp[(1) - (1)].strval)); ;}
    break;

  case 282:
#line 1390 "parser.y"
    { (yyval.context).type = word;
                                          (yyval.context).size = 1;
                                          (yyval.context).size2 = 1;
                                          (yyval.context).attrib = NULL;
                                        ;}
    break;

  case 283:
#line 1401 "parser.y"
    { (yyval.evalt) = do_UnionStatement((yyvsp[(3) - (5)].evalt), (yyvsp[(4) - (5)].evalt)); ;}
    break;

  case 284:
#line 1406 "parser.y"
    { (yyval.cl) = do_TABQuery((yyvsp[(2) - (3)].evalt)); ;}
    break;

  case 285:
#line 1411 "parser.y"
    { (yyval.evalt) = make_first_tabular_pattern((yyvsp[(1) - (2)].index), (yyvsp[(2) - (2)].evalt)); ;}
    break;

  case 286:
#line 1416 "parser.y"
    { (yyval.evalt) = add_tabular_pattern((yyvsp[(1) - (3)].evalt), &((yyvsp[(2) - (3)].context)), (yyvsp[(3) - (3)].index)); ;}
    break;

  case 287:
#line 1418 "parser.y"
    { (yyval.evalt) = NULL; ;}
    break;

  case 288:
#line 1421 "parser.y"
    { do_OptDistance(&((yyval.context)), (yyvsp[(2) - (3)].ival), (yyvsp[(2) - (3)].ival)); ;}
    break;

  case 289:
#line 1423 "parser.y"
    { do_OptDistance(&((yyval.context)), (yyvsp[(2) - (5)].ival), (yyvsp[(4) - (5)].ival)); ;}
    break;

  case 290:
#line 1424 "parser.y"
    { do_OptDistance(&((yyval.context)), 0, (yyvsp[(3) - (4)].ival)); ;}
    break;

  case 291:
#line 1425 "parser.y"
    { do_OptDistance(&((yyval.context)), 0, 0); ;}
    break;

  case 292:
#line 1430 "parser.y"
    { add_user_to_list((yyvsp[(2) - (3)].strval), (yyvsp[(3) - (3)].strval)); ;}
    break;

  case 294:
#line 1432 "parser.y"
    { add_host_to_list((yyvsp[(2) - (2)].strval)); ;}
    break;

  case 295:
#line 1433 "parser.y"
    { add_hosts_in_subnet_to_list((yyvsp[(2) - (2)].strval)); ;}
    break;

  case 296:
#line 1434 "parser.y"
    { add_host_to_list(NULL); ;}
    break;

  case 299:
#line 1443 "parser.y"
    { add_grant_to_last_user((yyvsp[(2) - (2)].strval)); ;}
    break;

  case 301:
#line 1449 "parser.y"
    {
                                                if (enable_macros) 
                                                  define_macro((yyvsp[(3) - (7)].strval), (yyvsp[(5) - (7)].ival), NULL, (yyvsp[(7) - (7)].strval));  /* <macro.c> */
                                                else 
                                                  cqpmessage(Error, "CQP macros not enabled.");
                                                free((yyvsp[(7) - (7)].strval));  /* don't forget to free the allocated strings */
                                                free((yyvsp[(3) - (7)].strval));
                                                ;}
    break;

  case 302:
#line 1458 "parser.y"
    {
                                                if (enable_macros) 
                                                  define_macro((yyvsp[(3) - (7)].strval), 0, (yyvsp[(5) - (7)].strval), (yyvsp[(7) - (7)].strval));  /* <macro.c> */
                                                else 
                                                  cqpmessage(Error, "CQP macros not enabled.");
                                                free((yyvsp[(7) - (7)].strval));  /* don't forget to free the allocated strings */
                                                free((yyvsp[(5) - (7)].strval));
                                                free((yyvsp[(3) - (7)].strval));
                                                ;}
    break;

  case 303:
#line 1467 "parser.y"
    {
                                                load_macro_file((yyvsp[(4) - (4)].strval));
                                                free((yyvsp[(4) - (4)].strval));  /* don't forget to free the allocated string */
                                                ;}
    break;

  case 306:
#line 1478 "parser.y"
    {
                                          list_macros(NULL);
                                        ;}
    break;

  case 307:
#line 1481 "parser.y"
    {
                                          list_macros((yyvsp[(3) - (3)].strval));
                                          free((yyvsp[(3) - (3)].strval));
                                        ;}
    break;

  case 308:
#line 1486 "parser.y"
    {
                                          print_macro_definition((yyvsp[(3) - (6)].strval), (yyvsp[(5) - (6)].ival));
                                          free((yyvsp[(3) - (6)].strval));
                                        ;}
    break;

  case 309:
#line 1494 "parser.y"
    {
                                          int l1 = strlen((yyvsp[(1) - (2)].strval)), l2 = strlen((yyvsp[(2) - (2)].strval));
                                          char *s = (char *) cl_malloc(l1 + l2 + 2);
                                          strcpy(s, (yyvsp[(1) - (2)].strval)); s[l1] = ' ';
                                          strcpy(s+l1+1, (yyvsp[(2) - (2)].strval));
                                          s[l1+l2+1] = '\0';
                                          free((yyvsp[(1) - (2)].strval));
                                          free((yyvsp[(2) - (2)].strval));
                                          (yyval.strval) = s;
                                        ;}
    break;

  case 310:
#line 1504 "parser.y"
    { (yyval.strval) = (yyvsp[(1) - (1)].strval); ;}
    break;

  case 311:
#line 1508 "parser.y"
    { cl_randomize(); ;}
    break;

  case 312:
#line 1509 "parser.y"
    { cl_set_seed((yyvsp[(2) - (2)].ival)); ;}
    break;

  case 314:
#line 1517 "parser.y"
    { (yyval.field) = (yyvsp[(1) - (1)].field); ;}
    break;

  case 315:
#line 1518 "parser.y"
    { (yyval.field) = NoField; ;}
    break;

  case 324:
#line 1534 "parser.y"
    { (yyval.Anchor).anchor = (yyvsp[(1) - (1)].field); (yyval.Anchor).offset = 0; ;}
    break;

  case 325:
#line 1535 "parser.y"
    { (yyval.Anchor).anchor = (yyvsp[(1) - (4)].field); (yyval.Anchor).offset = (yyvsp[(3) - (4)].ival); ;}
    break;


/* Line 1267 of yacc.c.  */
#line 4083 "parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 1538 "parser.y"



