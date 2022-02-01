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
     NQRID = 260,
     LABEL = 261,
     STRING = 262,
     FLAG = 263,
     TAGSTART = 264,
     TAGEND = 265,
     VARIABLE = 266,
     IPAddress = 267,
     IPSubnet = 268,
     INTEGER = 269,
     DOUBLEFLOAT = 270,
     FIELD = 271,
     FIELDLABEL = 272,
     ANCHORTAG = 273,
     ANCHORENDTAG = 274,
     SEARCH_STRATEGY = 275,
     TAB_SYM = 276,
     CAT_SYM = 277,
     DEFINE_SYM = 278,
     DIFF_SYM = 279,
     DISCARD_SYM = 280,
     EXPAND_SYM = 281,
     EXIT_SYM = 282,
     INTER_SYM = 283,
     JOIN_SYM = 284,
     SUBSET_SYM = 285,
     LEFT_SYM = 286,
     RIGHT_SYM = 287,
     SAVE_SYM = 288,
     SHOW_SYM = 289,
     CD_SYM = 290,
     TO_SYM = 291,
     WITHIN_SYM = 292,
     SET_SYM = 293,
     EXEC_SYM = 294,
     CUT_SYM = 295,
     INFO_SYM = 296,
     GROUP_SYM = 297,
     WHERE_SYM = 298,
     ESCAPE_SYM = 299,
     MEET_SYM = 300,
     UNION_SYM = 301,
     MU_SYM = 302,
     SORT_SYM = 303,
     COUNT_SYM = 304,
     ASC_SYM = 305,
     DESC_SYM = 306,
     REVERSE_SYM = 307,
     BY_SYM = 308,
     FOREACH_SYM = 309,
     ON_SYM = 310,
     YES_SYM = 311,
     OFF_SYM = 312,
     NO_SYM = 313,
     SLEEP_SYM = 314,
     REDUCE_SYM = 315,
     MAXIMAL_SYM = 316,
     WITH_SYM = 317,
     WITHOUT_SYM = 318,
     DELETE_SYM = 319,
     SIZE_SYM = 320,
     DUMP_SYM = 321,
     UNDUMP_SYM = 322,
     TABULATE_SYM = 323,
     NOT_SYM = 324,
     CONTAINS_SYM = 325,
     MATCHES_SYM = 326,
     GCDEL = 327,
     APPEND = 328,
     LEFT_APPEND = 329,
     LET = 330,
     GET = 331,
     NEQ = 332,
     IMPLIES = 333,
     RE_PAREN = 334,
     EOL_SYM = 335,
     ELLIPSIS = 336,
     MATCHALL = 337,
     LCSTART = 338,
     LCEND = 339,
     LCMATCHALL = 340,
     EXTENSION = 341,
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
#define NQRID 260
#define LABEL 261
#define STRING 262
#define FLAG 263
#define TAGSTART 264
#define TAGEND 265
#define VARIABLE 266
#define IPAddress 267
#define IPSubnet 268
#define INTEGER 269
#define DOUBLEFLOAT 270
#define FIELD 271
#define FIELDLABEL 272
#define ANCHORTAG 273
#define ANCHORENDTAG 274
#define SEARCH_STRATEGY 275
#define TAB_SYM 276
#define CAT_SYM 277
#define DEFINE_SYM 278
#define DIFF_SYM 279
#define DISCARD_SYM 280
#define EXPAND_SYM 281
#define EXIT_SYM 282
#define INTER_SYM 283
#define JOIN_SYM 284
#define SUBSET_SYM 285
#define LEFT_SYM 286
#define RIGHT_SYM 287
#define SAVE_SYM 288
#define SHOW_SYM 289
#define CD_SYM 290
#define TO_SYM 291
#define WITHIN_SYM 292
#define SET_SYM 293
#define EXEC_SYM 294
#define CUT_SYM 295
#define INFO_SYM 296
#define GROUP_SYM 297
#define WHERE_SYM 298
#define ESCAPE_SYM 299
#define MEET_SYM 300
#define UNION_SYM 301
#define MU_SYM 302
#define SORT_SYM 303
#define COUNT_SYM 304
#define ASC_SYM 305
#define DESC_SYM 306
#define REVERSE_SYM 307
#define BY_SYM 308
#define FOREACH_SYM 309
#define ON_SYM 310
#define YES_SYM 311
#define OFF_SYM 312
#define NO_SYM 313
#define SLEEP_SYM 314
#define REDUCE_SYM 315
#define MAXIMAL_SYM 316
#define WITH_SYM 317
#define WITHOUT_SYM 318
#define DELETE_SYM 319
#define SIZE_SYM 320
#define DUMP_SYM 321
#define UNDUMP_SYM 322
#define TABULATE_SYM 323
#define NOT_SYM 324
#define CONTAINS_SYM 325
#define MATCHES_SYM 326
#define GCDEL 327
#define APPEND 328
#define LEFT_APPEND 329
#define LET 330
#define GET 331
#define NEQ 332
#define IMPLIES 333
#define RE_PAREN 334
#define EOL_SYM 335
#define ELLIPSIS 336
#define MATCHALL 337
#define LCSTART 338
#define LCEND 339
#define LCMATCHALL 340
#define EXTENSION 341
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
#line 139 "parser.y"
{
  Evaltree                  evalt;
  Constrainttree            boolt;
  enum b_ops                boolo;
  int                       ival;
  double                    fval;
  int                       index;
  char                     *strval;
  CorpusList               *cl;

  struct {
    int a, b;
  } intpair;

  Context                   context;
  ActualParamList          *apl;

  enum ctxtdir              direction;

  struct Redir              redir;

  struct InputRedir         in_redir;

  struct {
    int ok;
    int ival;
    char *cval;
  }                         varval;

  struct {
    FieldType field;
    int inclusive;
  }                         base;

  struct {
    char *variableValue;
    char operator;
  }                         varsetting;

  struct {
    int mindist;
    int maxdist;
  }                         Distance;

  struct {
    FieldType anchor;
    int offset;
  }                         Anchor;

  struct {
    FieldType anchor1;
    int offset1;
    FieldType anchor2;
    int offset2;
  }                         AnchorPair;

  struct {
    char *name;
    int flags;
  }                         AttributeSpecification;

  RangeSetOp                rngsetop;

  SortClause                sortclause;

  FieldType                 field;

  SearchStrategy            search_strategy;

  TabulationItem            tabulation_item;
}
/* Line 1529 of yacc.c.  */
#line 317 "parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

