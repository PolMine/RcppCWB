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

/**
 * @file
 *
 * This file contains the code of the token-sequence regex (not the string-level regex!).
 *
 * It is a (somewhat) modified version of the regular expression to DFA converter
 * originally written by Mark Hopkins <markh@csd4.csd.uwm.edu> in the early 1990s.
 *
 * This code - as well as numerous derivatives - can be found online in various places, e.g.
 *
 *     http://www.cs.cmu.edu/Groups/AI/areas/nlp/parsing/regex/
 *
 * (although that is the second release, and the CWB version seems to have been derived
 * from the first release).
 *
 * Hopkins placed the original code into the public domain (or, equivalent). To quote
 * the original readme,
 *
 *     "The source for RE -> DFA and RE -> NFA generating algorithms has been
 *     provided for free with no restrictions [...] You can use it in your
 *     projects any way you like with no licensing or copyright restrictions
 *     [...] Everything here is new and original (as far as I can determine)
 *     and was generated over a 6 hour period on 5/17/92 on my own time,
 *     and a 6 hour period on 5/17/93 on my own time."
 *
 * The original FTP location was ftp://csd4.csd.uwm.edu/pub/compilers/regex/
 * but this seems to be no longer online.
 *
 * On this basis, we judge inclusion of the code here to be compatible with Gnu GPL.
 */


/*
 * Original header comment (from Hopkins):
 * ---------------------------------------
 *
 * Derived from the syntax:
 * Rule = (ID "=" Ex ",")* Ex.
 * Ex = "0" | "1" | ID | "(" Ex ")" | "[" Ex "]" | Ex "+" | Ex "*" | Ex Ex | Ex "|" Ex.
 * with the usual precedence rules.
 *
 */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>

#include "../cl/cl.h"

#include "cqp.h"
#include "eval.h"
#include "options.h"
#include "regex2dfa.h"

/**
 * Global variable containing a search string that is to be converted to a DFA.
 * (Needs to be global; functions using the DFA write to it, and then the DFA
 * parser reads from it. Declared as an external global in cqp.h so other parts
 * of CQP can access it. The actual space is not allocated here, rather a pointer,
 * passed as an argument to regex2dfa(), is copied here.)
 */
char *searchstr;

/* DATA STRUCTURES: internal to the regex2dfa module */

typedef unsigned char byte;

/** Symbol object: a particular string, living in a hash table, which can thus be referenced. */
typedef struct symbol *Symbol;

/* An Exp is an Expression object */
typedef struct exp *Exp;

/* The Equation object: has a Value (an expression) plus may or may not be on the stack */
typedef struct equation *Equation;

/** Expression (X) tag: one of Sym, Zero, One, And, Or, Star, Plus, Opt */
typedef enum {
  SymX, ZeroX, OneX, AndX, OrX, StarX, PlusX, OptX
} ExpTag;

/** Stack tag: one of RULE, EQU, PAR, OPT, OR, AND */
typedef enum {
  RULE, EQU, PAR, OPT, OR, AND
} StackTag;

/** A StackCard combines a StackTag with a Q integer. */
typedef struct {
  StackTag Tag;
  int Q;
} StackCard;

/** A State object. */
typedef struct state {
  int Class, States, *SList;
  int Empty, Shifts;
  struct {
    Symbol LHS;
    int RHS;
  } *ShList;
} *State;

/** Table of state objects */
State STab = (State)NULL;

/** Index into the STab table of state objects. */
int Ss;
/* note that instead of a "current size" variable, the STab expands by 8
 * every time its "next index" in Ss is a multiple of 8. */


/* THE SCANNER */

/** Lexer symbols */
typedef enum {
  EndT, CommaT, RParT, RBrT, EqualT, BarT,
  ZeroT, OneT, IdenT, LParT, LBrT, PlusT, StarT
} Lexical;

/** Item object: combines an LHS symbol with an RHS integer array  (tracked by Size). */
typedef struct item {
  Symbol LHS;
  int Size, *RHS;
} *Item;
/** Buffer for item objects */
Item IBuf = (Item)NULL;
/** Index for the next available slot in the buffer of Items (@see IBuf). */
int Is;
/** Number of currently available slots in the item buffer (@see IBuf). */
int IMax;



/** content for Expression object */
struct exp {
  ExpTag Tag;
  int Hash, Class;
  Exp Tail;
  union {
    Symbol Leaf;
    int *Arg;
  } Body;
};

/** content for Symbol object */
struct symbol {
  char *Name;
  int Hash;
  Symbol Next, Tail;
};

/** Block of lookup data */
char *Action[7] =
{
  /*  $,)]=|01x([+*  */
  ".ABCH|&&&&&+*", /* RULE:       -> Exp $ */
  "I=BCH|&&&&&+*", /* EQU:   Exp  -> Exp "=" Exp $    */
  "DD)FH|&&&&&+*", /* PAR:   Exp  -> '(' Exp $ ')'    */
  "EEG]H|&&&&&+*", /* OPT:   Exp  -> '[' Exp $ ']'    */
  "vvvvv|&&&&&+*", /* OR:    Exp  -> Exp '|' Exp $    */
  "xxxxxx&&&&&+*"  /* AND:   Exp  -> Exp Exp $        */
};

/** pointer to "last word" within the character space (tracks a marked point when ChP is incremented further along). */
char *LastW;

/** 16kb length of this module's "character space" buffer. */
#define MAX_CHAR 0x4000
/* This is the module's "character space" */
static char ChArr[MAX_CHAR];
/* pointer into the "character space" buffer. */
char *ChP;

/** Tracks the current line number within the input string (starting from 1). */
int LINE;

/** The number of errors enocuntered while parsing a regex to a DFA */
int ERRORS;

/** The maximum number of errors that the regex2dfa module will allow before killing the program */
#define MAX_ERRORS 25

/** Maximum size of the symbol hash array (number of buckets) */
#define HASH_MAX 0x200

/** Global hash table containing Symbols (particular strings). */
Symbol HashTab[HASH_MAX];
Symbol FirstB; /**< Pointer to the list in the first-created bucket of HashTab. */
Symbol LastB; /**< Pointer to the list in the most recently created bucket of HashTab. */

/** Size of the expression hash array (number of buckets). @see ExpHash */
#define NN 0x200
/** Expression hash table : array of pointers, where each pointer = head of a bucket linked list. */
Exp ExpHash[NN];

/* content for Equation object */
struct equation {
  Exp Value;
  int Hash;
  unsigned Stack:1; /**< Boolean: is this object on the stack? */
};


/** Equation table : array of Equation objects. */
Equation EquTab = (Equation)NULL;
/** Index of the next unfilled "slot" in the EquTab */
int Equs;
/** Current number of slots in the @see EquTab */
int EquMax;
/** number of slots added to the EquTab when it is reallocated. */
#define EQU_EXTEND 0x200


/** Size of the Stack of StackCards */
#define STACK_MAX 200
/** Stack (of stack cards) */
StackCard Stack[STACK_MAX];
/** Tracking pointer into the Stack (of stack cards) @see Stack */
StackCard *SP;


/** The XStack array of integers. This is where "Q" integers are pushed to/popped from. */
int *XStack = NULL;
/** Index of next unfilled "slot" in the XStack.*/
int Xs;
/** Current maximum size of the XStack. */
int XMax;
/** number of slots added to the XStack when it is reallocated. */
#define X_EXTEND 4


/** The Equiv Table or ETab: array of Equiv structs (each of which links an L State to an R State). */
struct Equiv {
  State L, R;
} *ETab = NULL;

/** Index of next unfilled "slot" in the ETab.*/
int Es;

/** Current maximum size of the ETab. */
int EMax;

/** Number of slots added to the ETab when it is reallocated */
#define ETAB_EXTEND 8


/** Index into searchstr showing the next character that will be read. */
int currpos;

/**
 * Gets the next character from the searchstr, and
 * increments its pointer; returns EOF if we are at
 * the end of the string.
 *
 * @see searchstr
 * @see currpos
 */
static int
GET(void)
{
  unsigned char ch = searchstr[currpos];

  if (ch == '\0')
    return EOF;
  else {
    currpos++;
    return ch;
  }
}

/**
 * Ungets a character from the search string (by decrementing the index into the
 * search string; the actual string itself isn't modified).
 *
 * @param Ch  Ignored.
 */
static void
UNGET(int Ch)
{
    if (currpos)
    --currpos;
}

/**
 * Prints an error message to stdout, and
 * exits the program if there are now just too many errors.
 * (In almost every case, exit() will be called straight after
 * this function anyway.)
 */
static void
REGEX2DFA_ERROR(char *Format, ...)
{
  va_list AP;
  Rprintf("[%d] ", LINE);
  va_start(AP, Format);
  Rprintf(Format, AP);
  va_end(AP);
  fputc('\n', stderr);
  if (++ERRORS == MAX_ERRORS) {
    Rprintf("regex2dfa: Reached the %d error limit.\n", MAX_ERRORS);
    exit(cqp_error_status ? cqp_error_status : 1);
  }
}

/** Gets the Lexical symbol corresponding to the next non-whitespace character in the searchstr (lexer function) */
static Lexical
LEX(void)
{
  int Ch; /* the current character */

  do {
    Ch = GET();
  } while (isspace(Ch));

  switch (Ch) {
  case EOF:
    return EndT;
  case '|':
    return BarT;
  case '(':
    return LParT;
  case ')':
    return RParT;
  case '[':
    return LBrT;
  case ']':
    return RBrT;
  case '0':
    return ZeroT;
  case '1':
    return OneT;
  case '*':
    return StarT;
  case '+':
    return PlusT;
  case '=':
    return EqualT;
  case ',':
    return CommaT;
  }

  if (isalpha(Ch) || Ch == '_' || Ch == '$') {
    for (LastW = ChP; isalnum(Ch) || Ch == '_' || Ch == '$'; ChP++) {
      if (ChP - ChArr == MAX_CHAR) {
        Rprintf("Out of character space.\n");
        exit(cqp_error_status ? cqp_error_status : 1);
      }
      *ChP = Ch;
      Ch = GET();
    }
    if (Ch != EOF)
      UNGET(Ch);
    if (ChP - ChArr == MAX_CHAR) {
      Rprintf("Out of character space.\n");
      exit(cqp_error_status ? cqp_error_status : 1);
    }
    *ChP++ = '\0';
    return IdenT;
  }
  else if (Ch == '"') {
    Ch = GET();
    for (LastW = ChP; Ch != '"' && Ch != EOF; ChP++) {
      if (ChP - ChArr == MAX_CHAR) {
        Rprintf("Out of character space.\n");
        exit(cqp_error_status ? cqp_error_status : 1);
      }
      *ChP = Ch;
      Ch = GET();
    }
    if (Ch == EOF) {
      Rprintf("Missing closing \".\n");
      exit(cqp_error_status ? cqp_error_status : 1);
    }
    if (ChP - ChArr == MAX_CHAR) {
      Rprintf("Out of character space.\n");
      exit(cqp_error_status ? cqp_error_status : 1);
    }
    *ChP++ = '\0';
    return IdenT;
  }
  else {
    REGEX2DFA_ERROR("extra character %c", Ch);
    return EndT;
  }
}


/** Creates a one-byte hash of the string S. */
static byte
Hash(char *S)
{
  int H = 0;
  char *T;

  for (T = S; *T != '\0'; T++)
    H = (H << 1) ^ *T;
  return H & 0xff;
}

/**
 * Get the Symbol corresponding to the string argument S.
 *
 * How it works: (1) Look up the symbol contained in string S in the Symbol table, and return if found.
 * If not found, (2) a new Symbol object is added to the correct bucket of the table and returned.
 * In the latter case, the new Symbol's pointer will be stored in LastB/FirstB as necessary.
 *
 * @return  Symbol object for the string in question.
 */
static Symbol
LookUp(char *S)
{
  Symbol Sym;
  byte H; /* hash key: leads us to the bucket to look in / add to. */

  for (H = Hash(S), Sym = HashTab[H]; Sym != 0; Sym = Sym->Next)
    if (strcmp(Sym->Name, S) == 0)
      return Sym;
  Sym = (Symbol)cl_malloc(sizeof *Sym);
  Sym->Name = cl_strdup(S);
  Sym->Hash = H;
  Sym->Next = HashTab[H];
  HashTab[H] = Sym;
  Sym->Tail = 0;
  if (FirstB == 0)
    FirstB = Sym;
  else
    LastB->Tail = Sym;
  return LastB = Sym;
}

static int
DUP(int A, int B)
{
  long L, S;

  S = A + B;
  if (S < NN)
    L = S*(S + 1)/2 + A;
  else {
    S = 2*(NN - 1) - S;
    A = NN - 1 - A;
    L = S*(S + 1)/2 + A;
    L = NN*NN - 1 - L;
  }
  return (int)(L/NN);
}

/** Finds or creates the Expression whose Leaf is the given Symbol, and sets its Class to the Q integer. */
static void
Store(Symbol S, int Q)
{
  int H = 0x100 + S->Hash;
  Exp E;

  for (E = ExpHash[H]; E != 0; E = E->Tail)
    if (S == E->Body.Leaf)
      break;
  if (E == 0) {
    E = (Exp)cl_malloc(sizeof *E);
    E->Tag = SymX;
    E->Body.Leaf = S;
    E->Hash = H;
    E->Tail = ExpHash[H];
    ExpHash[H] = E;
  }
  E->Class = Q;
}

static int
MakeExp(int Q, ExpTag Tag, ...)
{
  va_list AP;
  Symbol Sym = NULL;

  int H = 0;
  byte Args = 0;
  Exp HP, E;
  int Q0 = 0, Q1 = 0;

  va_start(AP, Tag);

  switch (Tag)
    {
    case SymX:
      Sym = va_arg(AP, Symbol);
      H = 0x100 + Sym->Hash;
      Args = 0;
      for (HP = ExpHash[H]; HP != 0; HP = HP->Tail)
        if (Sym == HP->Body.Leaf) {
          if (Q != -1 && Q != HP->Class)
            EquTab[Q].Value = HP;
          return HP->Class;
        }
      break;
    case ZeroX:
      H = 0;
      goto MakeNullary;
    case OneX:
      H = 1;
      goto MakeNullary;
    MakeNullary:
      Args = 0;
      HP = ExpHash[H];
      if (HP != 0)  {
        if (Q != -1 && Q != HP->Class)
          EquTab[Q].Value = HP;
        return HP->Class;
      }
      break;
    case PlusX:
      Q0 = va_arg(AP, int);
      H = 0x02 + EquTab[Q0].Hash*0x0a/0x200;
      goto MakeUnary;
    case StarX:
      Q0 = va_arg(AP, int);
      H = 0x0c + EquTab[Q0].Hash*0x14/0x200;
      goto MakeUnary;
    case OptX:
      Q0 = va_arg(AP, int);
      H = 0x20 + EquTab[Q0].Hash/0x10;
    MakeUnary:
      Args = 1;
      for (HP = ExpHash[H]; HP != 0; HP = HP->Tail)
        if (Q0 == HP->Body.Arg[0]) {
          if (Q != -1 && Q != HP->Class)
            EquTab[Q].Value = HP;
          return HP->Class;
        }
      break;
    case OrX:
      Q0 = va_arg(AP, int);
      Q1 = va_arg(AP, int);
      H = 0x40 + DUP(EquTab[Q0].Hash, EquTab[Q1].Hash)/8;
      goto MakeBinary;
    case AndX:
      Q0 = va_arg(AP, int);
      Q1 = va_arg(AP, int);
      H = 0x80 + DUP(EquTab[Q0].Hash, EquTab[Q1].Hash)/4;
    MakeBinary:
      Args = 2;
      for (HP = ExpHash[H]; HP != 0; HP = HP->Tail)
        if (Q0 == HP->Body.Arg[0] && Q1 == HP->Body.Arg[1]) {
          if (Q != -1 && Q != HP->Class)
            EquTab[Q].Value = HP;
          return HP->Class;
        }
      break;
    }
  va_end(AP);
  E = (Exp)cl_malloc(sizeof *E);
  E->Tag = Tag;
  if (Tag == SymX)
    E->Body.Leaf = Sym;
  else
    {
      E->Body.Arg = (int *) ((Args > 0) ? cl_malloc(Args*sizeof(int)) : NULL);
      if (Args > 0)
        E->Body.Arg[0] = Q0;
      if (Args > 1)
        E->Body.Arg[1] = Q1;
    }
  E->Hash = H;
  E->Tail = ExpHash[H];
  ExpHash[H] = E;
  if (Q == -1)
    {
      if (Equs == EquMax)
        {
          EquMax += EQU_EXTEND;
          EquTab = (Equation)cl_realloc(EquTab, sizeof *EquTab * EquMax);
        }
      EquTab[Equs].Hash = H;
      EquTab[Equs].Stack = 0;
      Q = Equs++;
    }
  EquTab[Q].Value = E;
  E->Class = Q;
  return Q;
}

/** Pushes a Tag / Q combination onto the stack of cards. */
static void
PUSH(StackTag Tag, int Q)
{
  if (SP >= Stack + STACK_MAX) {
    REGEX2DFA_ERROR("Expression too complex ... aborting.");
    exit(cqp_error_status ? cqp_error_status : 1);
  }
  SP->Tag = Tag;
  SP->Q = Q;
  SP++;
}


/* Parser stack macros */

/** Evals to the Tag of the card at the top of the stack. */
#define TOP ((SP - 1)->Tag)

static int unused_junk_value;
/** Moves the tracker pointer backwards by one, then returns the Q value
 *  of the stack card it is now pointing to. */
#define POP() (unused_junk_value = ((--SP)->Q))

/** the regex parser proper; aborts or returns -1 in case of error */
static int
Parse(void)
{
  Lexical L;
  Symbol ID = NULL;
  int RHS; /* this value is what eventually gets returned. */

  SP = Stack;

LHS:
  /* get next symbol from the lexer */
  L = LEX();
  if (L == IdenT) {
      ID = LookUp(LastW);
      L = LEX();
      if (L == EqualT) {
          PUSH(EQU, -1);
          L = LEX();
      }
      else {
          PUSH(RULE, -1);
          RHS = MakeExp(-1, SymX, ID);
          goto END;
      }
  }
  else
    PUSH(RULE, -1);
EXP:
  /* take action on the current Lexical symbol */
  switch (L)   {
  case LParT:
    PUSH(PAR, -1);
    L = LEX();
    goto EXP;
  case LBrT:
    PUSH(OPT, -1);
    L = LEX();
    goto EXP;
  case ZeroT:
    RHS = MakeExp(-1, ZeroX);
    L = LEX();
    goto END;
  case OneT:
    RHS = MakeExp(-1, OneX);
    L = LEX();
    goto END;
  case IdenT:
    RHS = MakeExp(-1, SymX, LookUp(LastW));
    L = LEX();
    goto END;
  default:
    REGEX2DFA_ERROR("Corrupt expression.");
    return -1;
  }

END:
  /* finish off parsing by checking for anything left over indicating errors etc. */
  switch (Action[TOP][L])  {
  case 'A':
    REGEX2DFA_ERROR("Extra ','");
    exit(cqp_error_status ? cqp_error_status : 1);
  case 'B':
    REGEX2DFA_ERROR("Unmatched ).");
    L = LEX();
    goto END;
  case 'C':
    REGEX2DFA_ERROR("Unmatched ].");
    L = LEX();
    goto END;
  case 'D':
    REGEX2DFA_ERROR("Unmatched (.");
    POP();
    goto END;
  case 'E':
    REGEX2DFA_ERROR("Unmatched [.");
    goto MakeOpt;
  case 'F':
    REGEX2DFA_ERROR("( ... ].");
    POP();
    L = LEX();
    goto END;
  case 'G':
    REGEX2DFA_ERROR("[ ... ).");
    L = LEX();
    goto MakeOpt;
  case 'H':
    REGEX2DFA_ERROR("Left-hand side of '=' must be symbol.");
    exit(cqp_error_status ? cqp_error_status : 1);
  case 'I':
    REGEX2DFA_ERROR("Missing evaluation.");
    exit(cqp_error_status ? cqp_error_status : 1);
  case '.':
    POP();
    return RHS;
  case ')':
    POP();
    L = LEX();
    goto END;
  case ']':
    L = LEX();
MakeOpt:
    POP();
    RHS = MakeExp(-1, OptX, RHS);
    goto END;
  case '=':
    Store(ID, RHS);
    POP();
    goto LHS;
  case 'v':
    RHS = MakeExp(-1, OrX, POP(), RHS);
    goto END;
  case 'x':
    RHS = MakeExp(-1, AndX, POP(), RHS);
    goto END;
  case '*':
    RHS = MakeExp(-1, StarX, RHS);
    L = LEX();
    goto END;
  case '+':
    RHS = MakeExp(-1, PlusX, RHS);
    L = LEX();
    goto END;
  case '|':
    PUSH(OR, RHS); L = LEX();
    goto EXP;
  case '&':
    PUSH(AND, RHS);
    goto EXP;
  }

  assert(0 && "Not reached");
  return 0;
}

static void
PushQ(int Q)
{

  if (EquTab[Q].Stack)
    return;
  if (Xs == XMax) {
    XMax += X_EXTEND;
    XStack = cl_realloc(XStack, sizeof *XStack * XMax);
  }
  XStack[Xs++] = Q;
  EquTab[Q].Stack = 1;
}

static void
PopQ(void)
{
  int Q = XStack[--Xs];
  EquTab[Q].Stack = 0;
}


/**
 * Adds a State with the given SList into STab
 * (or finds one that already exists); returns
 * the index of that State in STab.
 */
static int
AddState(int States, int *SList)
{
  int D, I;
  State DP;

  /* foreach DP in STab */
  for (D = 0; D < Ss; D++) {
    DP = &STab[D];
    if (States != DP->States)
      continue;
    for (I = 0; I < States; I++)
      if (SList[I] != DP->SList[I])
        break;
    if (I >= States) {
      /* all states in DP were checked and match. So SList can be dumped as STab already has that State. */
      cl_free(SList);
      return D;
    }
  }
  /* TODO : an issue to work round.
   * Brilliant ... the cl_realloc() below might move the state table around in memory if it cannot
     be expanded in place, breaking any pointers into the table held in local variables of the calling
     function.  Fortunately, AddState() is only called from FormState() in a loop that modifies
     "embedded" variables, so that this bug only surfaces if the original memory location is overwritten
     immediately (while the loop is still running).  It can be triggered reliably by the query
         ([pos = "IN|TO"] [pos = "DT.*"]? [pos = "JJ.*"]* [pos = "N.*"]+){3};
     on a PowerPC G4 running Mac OS X 10.4 (God knows why it happens in this configuration).
     To avoid the problem, local pointers into STab[] should be updated after every call to AddState(). */
  if ((Ss&7) == 0)
    STab = cl_realloc(STab, sizeof *STab * (Ss + 8));
  STab[Ss].Class = Ss;
  STab[Ss].States = States;
  STab[Ss].SList = SList;
  return Ss++;
}

/** Adds to the item buffer: looks up (or creates) an Item with the LHS Symbol, and adds the Q to its RHS array if it's not already there. */
static void
AddBuf(Symbol LHS, int Q)
{
  /* Diff = place holder for strcmp results; I/J/S/T are loop counters.  */
  int Diff, I, J, S, T;
  Item IP;
  char *Name = LHS->Name;

  for (I = 0; I < Is; I++) {
    Diff = strcmp(IBuf[I].LHS->Name, Name);
    if (Diff == 0)
      goto FOUND;
    if (Diff > 0)
      break;
  }
  if (Is >= IMax) {
    IMax += 8;
    IBuf = cl_realloc(IBuf, sizeof *IBuf * IMax);
  }
  for (J = Is++; J > I; J--)
    IBuf[J] = IBuf[J - 1];
  IBuf[I].LHS = LHS, IBuf[I].Size = 0, IBuf[I].RHS = 0;

 FOUND:
  IP = &IBuf[I];
  for (S = 0; S < IP->Size; S++) {
    if (IP->RHS[S] == Q)
      return;
    if (IP->RHS[S] > Q)
      break;
  }
  if ((IP->Size&7) == 0)
    IP->RHS = cl_realloc(IP->RHS, sizeof *IP->RHS * (IP->Size + 8));
  for (T = IP->Size++; T > S; T--)
    IP->RHS[T] = IP->RHS[T - 1];
  IP->RHS[S] = Q;
}

/** NB. As used in regex2dfa, qv, Q is the return value from Parse() */
static void
FormState(int Q)
{
  int I, S, S1, X;
  int qX, Q1, Q2;
  int A, B;
  State SP;
  Exp E, E1;

  IBuf = NULL;
  IMax = 0;
  STab = NULL;
  Ss = 0;
  AddState(1, &Q);

  /* for each entry in the table of States (as SP) ... */
  for (S = 0; S < Ss; S++) {
    SP = &STab[S];
    for (Xs = 0, S1 = 0; S1 < SP->States; S1++)
      PushQ(SP->SList[S1]);
    for (SP->Empty = 0, Is = 0, X = 0; X < Xs; X++)  {
      qX = XStack[X];
    EVALUATE:
      E = EquTab[qX].Value;
      switch (E->Tag) {
      case SymX:
        AddBuf(E->Body.Leaf, MakeExp(-1, OneX));
        break;
      case OneX:
        SP->Empty = 1;
        break;
      case ZeroX:
        break;
      case OptX:
        Q1 = E->Body.Arg[0];
        MakeExp(qX, OrX, MakeExp(-1, OneX), E->Body.Arg[0]);
        goto EVALUATE;
      case PlusX:
        Q1 = E->Body.Arg[0];
        MakeExp(qX, AndX, Q1, MakeExp(-1, StarX, Q1));
        goto EVALUATE;
      case StarX:
        Q1 = E->Body.Arg[0];
        MakeExp(qX, OrX, MakeExp(-1, OneX), MakeExp(-1, PlusX, Q1));
        goto EVALUATE;
      case OrX:
        Q1 = E->Body.Arg[0];
        Q2 = E->Body.Arg[1];
        PushQ(Q1);
        PushQ(Q2);
        break;
      case AndX:
        Q1 = E->Body.Arg[0], Q2 = E->Body.Arg[1];
        E1 = EquTab[Q1].Value;
        switch (E1->Tag) {
        case SymX:
          AddBuf(E1->Body.Leaf, Q2);
          break;
        case OneX:
          EquTab[qX].Value = EquTab[Q2].Value;
          goto EVALUATE;
        case ZeroX:
          MakeExp(qX, ZeroX);
          break;
        case OptX:
          A = E1->Body.Arg[0];
          MakeExp(qX, OrX, Q2, MakeExp(-1, AndX, A, Q2));
          goto EVALUATE;
        case PlusX:
          A = E1->Body.Arg[0];
          MakeExp(qX, AndX, A, MakeExp(-1, OrX, Q2, qX));
          goto EVALUATE;
        case StarX:
          A = E1->Body.Arg[0];
          MakeExp(qX, OrX, Q2, MakeExp(-1, AndX, A, qX));
          goto EVALUATE;
        case OrX:
          A = E1->Body.Arg[0], B = E1->Body.Arg[1];
          MakeExp(qX, OrX, MakeExp(-1, AndX, A, Q2), MakeExp(-1, AndX, B, Q2));
          goto EVALUATE;
        case AndX:
          A = E1->Body.Arg[0], B = E1->Body.Arg[1];
          MakeExp(qX, AndX, A, MakeExp(-1, AndX, B, Q2));
          goto EVALUATE;
        }
      }
    }
    while (Xs > 0)
      PopQ();
    SP->Shifts = Is;
    SP->ShList = cl_malloc(sizeof *SP->ShList * Is);
    for (I = 0; I < Is; I++)  {
      int rhs_state = -1;
      SP->ShList[I].LHS = IBuf[I].LHS;
      rhs_state = AddState(IBuf[I].Size, IBuf[I].RHS);
      SP = &STab[S];        /* AddState() might have reallocated state table -> update pointer */
      SP->ShList[I].RHS = rhs_state;
    }
  }
  cl_free(IBuf);
  Is = IMax = 0;
}

static void
AddEquiv(int L, int R)
{
  int E;
  State SL, SR;

  L = STab[L].Class;
  R = STab[R].Class;

  if (L == R)
    return;
  if (L > R)
    {
      L ^= R;
      R ^= L;
      L ^= R;
    }

  SL = &STab[L];
  SR = &STab[R];

  for (E = 0; E < Es; E++)
    if (SL == ETab[E].L && SR == ETab[E].R)
      return;
  if (Es >= EMax)     {
    EMax += ETAB_EXTEND;
    ETab = cl_realloc(ETab, sizeof *ETab * EMax);
  }
  ETab[Es].L = SL;
  ETab[Es].R = SR;
  Es++;
}

static void
MergeStates(void)
{
  int Classes, S, S1, E, Sh;
  State SP, SP1, QL, QR;

  ETab = 0;
  EMax = 0;

  for (S = 0; S < Ss; S++) {
    SP = &STab[S];
    if (SP->Class != S)
      continue;
    for (S1 = 0; S1 < S; S1++) {
      SP1 = &STab[S1];
      if (SP1->Class != S1)
        continue;
      Es = 0;
      AddEquiv(S, S1);
      for (E = 0; E < Es; E++) {
        QL = ETab[E].L;
        QR = ETab[E].R;
        if (QL->Empty != QR->Empty || QL->Shifts != QR->Shifts)
          goto NOT_EQUAL;
        for (Sh = 0; Sh < QL->Shifts; Sh++)
          if (QL->ShList[Sh].LHS != QR->ShList[Sh].LHS)
            goto NOT_EQUAL;
        for (Sh = 0; Sh < QL->Shifts; Sh++)
          AddEquiv(QL->ShList[Sh].RHS, QR->ShList[Sh].RHS);
      }
    /* EQUAL: */
      break;
    NOT_EQUAL:
      continue;
    }
    if (S1 < S) for (E = 0; E < Es; E++) {
      State QL = ETab[E].L;
      QR = ETab[E].R;
      QR->Class = QL->Class;
    }
  }
  for (Classes = 0, S = 0; S < Ss; S++) {
    SP = &STab[S];
    SP->Class = (SP->Class == S) ? Classes++ : STab[SP->Class].Class;
  }
}

/** Write states to stdout. */
static void
WriteStates(void)
{
  int S, Sh, Classes, C;
  State SP;

  for (S = Classes = 0; S < Ss; S++) {
    SP = &STab[S];
    if (SP->Class != Classes)
      continue;
    Classes++;
    Rprintf("s%d =", SP->Class);
    if (SP->Empty) {
      Rprintf(" fin");
      if (SP->Shifts > 0)
        Rprintf(" |");
    }
    for (Sh = 0; Sh < SP->Shifts; Sh++) {
      C = SP->ShList[Sh].RHS;
      if (Sh > 0)
        Rprintf(" |");
      Rprintf(" %s s%d", SP->ShList[Sh].LHS->Name, STab[C].Class);
    }
    putchar('\n');
  }
}

/** Initialises the members of the argument DFA object. */
void
init_dfa(DFA *dfa)
{
  assert(dfa);
  dfa->TransTable = NULL;
  dfa->Final = NULL;
  dfa->Max_States = dfa->Max_Input = 0;
}

/** Frees all the memory associated with the argument DFA. */
void
free_dfa(DFA *dfa)
{
  int i;
  if (!dfa)
    return;

  if (dfa->TransTable)
    for(i = 0; i < dfa->Max_States; i++)
      cl_free(dfa->TransTable[i]);
  cl_free(dfa->TransTable);
  cl_free(dfa->Final);

  dfa->Max_States = 0;
  dfa->Max_Input = 0;
}

/** Prints the contents of a DFA to stdout. */
void
show_complete_dfa(DFA dfa)
{
  int i, j;

  for (i = 0; i < dfa.Max_States; i++) {
    Rprintf("s%d", i);
    if (dfa.Final[i])
      Rprintf("(final)");
    else
      putchar('\t');
    for (j = 0; j < dfa.Max_Input; j++)   {
      Rprintf("\t%d -> ", j);
      if (dfa.TransTable[i][j] == dfa.E_State)
        Rprintf("E\t");
      else
        Rprintf("s%d,",dfa.TransTable[i][j]);
    }
    putchar('\n');
  }
}

/**
 * Initialises the global variables in the regex2dfa module, also freeing any previously-allocated memory.
 */
static void
init(void)
{
  int i;

  cl_free(STab);
  cl_free(IBuf);
  cl_free(EquTab);
  cl_free(XStack);
  cl_free(ETab);

  for (i = 0; i < HASH_MAX; i++)
    cl_free(HashTab[i]);

  for (i = 0; i < NN; i++)
    cl_free(ExpHash[i]);

  ChP = ChArr;
  LINE = 1;
  ERRORS = 0;
  FirstB = NULL;
  Equs = 0;
  EquMax = 0;
  Xs = 0;
  XMax = 0;
  Ss = 0;
  currpos = 0;
}

/**
 * Converts a regular expression to a DFA. Public function.
 *
 * @param rxs         The regular expression.
 * @param automaton   Pointer to the DFA object to write to.
 */
void
regex2dfa(char *rxs, DFA *automaton)
{
  int Q, i, j;
  int S, Sh, Classes, C;
  State SP;

  searchstr = rxs;

  init();

  Q = Parse();

  if (ERRORS > 0)
    Rprintf("%d error(s)\n", ERRORS);
  if (Q == -1)
    exit(cqp_error_status ? cqp_error_status : 1);
  FormState(Q);
  MergeStates();

  automaton->Max_States = Ss;
  automaton->Max_Input = Environment[ee_ix].MaxPatIndex + 1;
  automaton->E_State = automaton->Max_States;

  if (show_dfa) /* this is a global CQP variable */
    WriteStates();

  /* allocate memory for the transition table and initialize it. */
  automaton->TransTable = (int **)cl_malloc(sizeof(int *) * automaton->Max_States);
  for (i = 0; i < Ss; i++)  {
    automaton->TransTable[i] = (int *)cl_malloc(sizeof(int) * automaton->Max_Input);
    for (j = 0; j < automaton->Max_Input; j++)
      automaton->TransTable[i][j] = automaton->E_State;
  }

  /* allocate memory for the table of final states. */
  automaton->Final = (Boolean *)cl_malloc(sizeof(Boolean) * (Ss + 1));

  /* initialize the table of final states. */
  for (i = 0; i <= automaton->Max_States; i++)
    automaton->Final[i] = False;

  for (S = Classes = 0; S < Ss; S++) {
    SP = &STab[S];
    if (SP->Class != Classes)
      continue;
    Classes++;
    if (SP->Empty)
      automaton->Final[SP->Class] = True;
    for (Sh = 0; Sh < SP->Shifts; Sh++) {
      C = SP->ShList[Sh].RHS;
      automaton->TransTable[SP->Class][atoi(SP->ShList[Sh].LHS->Name)] = STab[C].Class;
    }
  }
}
