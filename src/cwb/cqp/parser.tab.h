/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
/* Line 1529 of yacc.c.  */
#line 317 "parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

