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


#ifndef _cqp_symtab_h_
#define _cqp_symtab_h_


/*
 * The SYMBOL LOOKUP part: SymbolTable and LabelEntry
 */

/** whether label has been defined or is defined by this call */
#define LAB_DEFINED 1
/** whether label has been used (i.e. read out) or is used by this call */
#define LAB_USED    2
/** special labels must not be set/modified by user; defined/used consistency isn't checked for special labels */
#define LAB_SPECIAL 4
/** namespace #2 used for s-attribute region boundaries by query engine */
#define LAB_RDAT    8

/**
 * LabelEntry: the symbol tables are made up of two linked lists of these objects.
 *
 * @see SymbolTable.
 */
typedef struct _label_entry {
  int        flags;
  char      *name;
  int        ref;             /**< array index the label refers to */
  struct _label_entry *next;
} *LabelEntry;


/**
 * The SymbolTable object.
 *
 * cqp-2.2+ uses a global symbol table to store label references which gives erroneous
 * results for queries that contain optional elements. A proper treatment of labels requires
 * each of the simulations traversing the NFA in parallel to have its own symbol table.
 * Since the actual symbols are the same for all states, it is more efficient to split the
 * symbol tables into symbol lookup and the actual data. Each simulation has its own data array
 * (which stores corpus positions), but symbol lookup is shared between all simulations and
 * returns an _index_ into the data array. If a simulation branches -- which happens at the
 * left edge of alternatives or optional elements -- the symbol data array must be duplicated.
 *
 * A symbol table now contains multiple namespaces (accessed by flags such as LAB_RDAT)
 */
typedef struct _symbol_table {
  LabelEntry  user;       /**< user namespace */
  LabelEntry  rdat;       /**< namespace for LAB_RDAT labels */
  int next_index;         /**< next free reference table index */
} *SymbolTable;

SymbolTable new_symbol_table(void);

void delete_symbol_table(SymbolTable st);

LabelEntry find_label(SymbolTable st, char *s, int flags);

LabelEntry label_lookup(SymbolTable st, char *s, int flags, int create);

int check_labels(SymbolTable st);

void print_symbol_table(SymbolTable st);

/* iterate through labels (visits only labels where the corresponding flags are set) */
LabelEntry symbol_table_new_iterator(SymbolTable st, int flags);
LabelEntry symbol_table_iterator(LabelEntry prev, int flags);




/*
 * The DATA ARRAY part: RefTab
 */


/**
 * The RefTab object (represents a reference table).
 */
typedef struct _RefTab {
  int size;
  int *data;
} *RefTab;

RefTab new_reftab(SymbolTable st);
void delete_reftab(RefTab rt);
void dup_reftab(RefTab rt1, RefTab rt2);
void reset_reftab(RefTab rt);

void set_reftab(RefTab rt, int index, int value);
int  get_reftab(RefTab rt, int index, int cpos);

void print_label_values(SymbolTable st, RefTab rt, int cpos);


/**
 * The StateQueue object represents a wait queue of FSA simulation states (FSAState objects), each consisting of a cpos and a complete reference table.
 *
 * Such waiting queues are used to implement region elements (`<<name>>`) in finite-state CQP queries. When a state enters the
 * element at the start of a region, it is added to the wait queue with cpos set to the next token after the region and references
 * updated according to labels and/or target markers set by the user. The simulation then waits in a loop, incrementing the cpos
 * until the next state from the waiting queue can be emitted.
 *
 * FSAState objects are currently always internal to a StateQueue, but that's how FSA simulation states should have been represented in the first place,
 * so we use the general class name.
 */

typedef struct _FSAState {
  struct _FSAState *next; /**< pointer to next state in wait queue */
  int cpos;               /**< cpos of this simulation state (determines when it will be emitted) */
  RefTab rt;              /**< its reference table (using a RefTab isn't optimal because it requires a second `malloc()`, but we prefer the easier and cleaner implementation for now */
} *FSAState;

typedef struct _StateQueue {
  FSAState head;          /**< the StateQueue object just wraps the head pointer of the wait queue so that it can be shared between multiple FSA transitions (AVS objects) */
  SymbolTable st;         /**< the underlying SymbolTable can be used to make sure that all states in the queue refer to the same symbols */
  int n_states;           /**< number of states waiting in the queue (mostly for debugging purposes) */
} *StateQueue;

FSAState FSAState_new(SymbolTable st);
void FSAState_delete(FSAState *self_ref);

StateQueue StateQueue_new(SymbolTable st);
void StateQueue_delete(StateQueue *self_ref);
void StateQueue_clear(StateQueue self);
RefTab StateQueue_push(StateQueue self, int cpos, RefTab rt);
int StateQueue_pop(StateQueue self, RefTab output_rt);
int StateQueue_next_cpos(StateQueue self);
int StateQueue_length(StateQueue self);

#endif
