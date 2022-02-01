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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../cl/cl.h"

#include "symtab.h"
#include "output.h"
#include "cqp.h"


/*
 * some internal helper functions
 */

/**
 * Frees the whole of a linked list of label entries.
 */
void
free_labellist(LabelEntry label)
{
  LabelEntry tmp;

  while (label) {
    tmp = label;
    label = tmp->next;

    cl_free(tmp->name);
    cl_free(tmp);
  }
}


/*
 * The SYMBOL LOOKUP part: SymbolTable and LabelEntry
 */

/** create new symbol table */
SymbolTable
new_symbol_table(void)
{
  SymbolTable st = cl_malloc(sizeof(struct _symbol_table));
  st->user = NULL;
  st->rdat = NULL;
  st->next_index = 0;
  return st;
}

/** delete symbol table (free all entries) */
void
delete_symbol_table(SymbolTable st)
{
  if (!st)
    return;
  free_labellist(st->user);
  free_labellist(st->rdat);
  cl_free(st);
}

/** Returns label entry, or NULL if undefined (flags are used _only_ to determine namespace) */
LabelEntry
find_label(SymbolTable st, char *s, int flags)
{
  LabelEntry label = NULL;
  if (st)
    for (label = (flags & LAB_RDAT) ?  st->rdat :  st->user ; label ; label = label->next)
      if (cl_streq(label->name, s))
        break;
  return label;
}


/**
 * Look up a label, add flags, and return label entry (NULL if undefined).
 * if create is set and label does not exist, it is added to the symbol table.
 */
LabelEntry
label_lookup(SymbolTable st, char *s, int flags, int create)
{
  LabelEntry label;
  int user_namespace, this_label; /* both bools */

  if (NULL != (label = find_label(st, s, flags))) {
    /* Found it ! add flags from this call */
    label->flags |= flags;
    return label;
  }

  /* are we in user namespace? (otherwise namespace is rdat, so far) */
  user_namespace = (flags & LAB_RDAT) ? 0 : 1;

  /* check if the requested label is the 'this' label (label _ in user namespace) */
  this_label = (user_namespace && cl_str_is(s, "_")) ? 1 : 0;

  /* add label to symbol table if create is set ('_' is always defined and created automatically) */
  if (create || this_label) {
    /* special labels: 'this' and all field names (used to keep track of target anchors in queries) */
    if (this_label || field_name_to_type(s) != NoField)
      flags |= LAB_SPECIAL;

    /* allocate new label entry and initialise data fields */
    label = cl_malloc(sizeof(struct _label_entry));
    label->name = cl_strdup(s);
    label->flags = flags;
    label->ref = this_label ? -1 : st->next_index++;

    /* insert into correct namespace (so far only user or rdat) */
    if (user_namespace) {
      label->next = st->user;
      st->user = label;
    }
    else {
      label->next = st->rdat;
      st->rdat = label;
    }

    return label;
  }
  /* label was not found, was not "_", and wasn't created because create was off */
  return NULL;
}


/**
 * Checks whether all used labels are defined (and vice versa);
 * only non-special labels in the user namespace of the argument
 * symbol table will be checked.
 *
 * Result is boolean.
 */
int
check_labels(SymbolTable st)
{
  LabelEntry label;
  int result = 1;

  if (st)
    for ( label = st->user ; label ; label = label->next )
      if (!(label->flags & LAB_SPECIAL)) {
        if (!(label->flags & LAB_USED)) {
          cqpmessage(Warning, "Label %s defined but not used", label->name);
          result = 0;
        }
        if (!(label->flags & LAB_DEFINED)) {
          cqpmessage(Warning, "Label %s used but not defined", label->name);
          result = 0;
        }
      }
  return result;
}


/** print symbol table contents (for debugging purposes) */
void
print_symbol_table(SymbolTable st)
{
  LabelEntry label_list[2];
  LabelEntry label;
  char *namespace[] = {"USER", "RDAT"};
  int i;

  if (!st)
    return;
  label_list[0] = st->user;
  label_list[1] = st->rdat;

  Rprintf("Contents of SYMBOL TABLE:\n");
  for (i = 0; i < 2; i++) {
    for (label = label_list[i] ; label ; label = label->next)
      Rprintf("\t%s\t%s(flags: %d)  ->  RefTab[%d]\n", namespace[i], label->name, label->flags, label->ref);
  }
}


LabelEntry
symbol_table_new_iterator(SymbolTable st, int flags)
{
  LabelEntry label = NULL;
  int user_namespace = (flags & LAB_RDAT) ? 0 : 1;

  if (st)
    if ((label = user_namespace ? st->user : st->rdat) && flags != (label->flags & flags))
      label = symbol_table_iterator(label, flags);
      /* if first label in list doesn't match flags, find next matching label */

  return label;
}


LabelEntry
symbol_table_iterator(LabelEntry prev, int flags)
{
  if (prev)
    for (prev = prev->next ; prev ; prev = prev->next)
      if ((prev->flags & flags) == flags)
        break;
  return prev;
}




/*
 * The DATA ARRAY part: RefTab
 */

/**
 * Create new reference table of required size for the given symbol table.
 *
 * NB If further labels are added to st, you must reallocate the reference table
 * to make room for the new reference indices
 */
RefTab
new_reftab(SymbolTable st)
{
  RefTab rt = (RefTab)cl_malloc(sizeof(struct _RefTab));
  rt->size = st->next_index;
  rt->data = (int *)cl_malloc(rt->size * sizeof(int));
  return rt;
}


/** Deletes (and frees) a reference table. */
void
delete_reftab(RefTab rt)
{
  if (rt)
    cl_free(rt->data);
  cl_free(rt);
}

/** Copies rt1 to rt2; doesn't allocate new reftab for efficiency reasons. */
void
dup_reftab(RefTab rt1, RefTab rt2)
{
  assert(rt1 && rt2);
  if (rt1->size != rt2->size) {
    Rprintf("dup_reftab()<symtab.c>: Tried to dup() RefTab (%d entries) to RefTab of different size (%d entries)\n", rt1->size, rt2->size);
    exit(cqp_error_status ? cqp_error_status : 1);
  }
  memcpy(rt2->data, rt1->data, rt1->size * sizeof(int));
}

/** resets all referenced corpus position to -1 -> undefine all references */
void
reset_reftab(RefTab rt)
{
  int i;
  if(rt)
    for (i = 0; i < rt->size; i++)
      rt->data[i] = -1;
}

/** set references (cpos value in get_reftab is returned for 'this' label (_), set to -1 if n/a) */
void
set_reftab(RefTab rt, int index, int value)
{
  if (rt) {
    if (index < 0 || index >= rt->size) {
      cqpmessage(Error, "RefTab index #%d not in range 0 .. %d", index, rt->size - 1);
      exit(cqp_error_status ? cqp_error_status : 1);
    }
    else
      rt->data[index] = value;
  }
}

/** read references (cpos value in get_reftab is returned for 'this' label (_), set to -1 if n/a) */
int
get_reftab(RefTab rt, int index, int cpos)
{
  if (-1 == index)  /* -1 == 'this' label returns <cpos> value */
    return cpos;

  if (!rt)          /* NULL is used for dummy reftabs */
    return -1;

  if (index < 0 || index >= rt->size) {
    Rprintf("get_reftab()<symtab.c>: RefTab index #%d not in range 0 .. %d", index, rt->size - 1);
    return -1;
  }

  return rt->data[index];
}

/**
 * Prints the current label values (for debugging).
 *
 * @param st    The SymbolTable
 * @param rt    The reference table
 * @param cpos  The corpus position
 */
void
print_label_values(SymbolTable st, RefTab rt, int cpos)
{
  LabelEntry label = NULL;
  int i;

  char *prefix[] = {"", "USER", "RDAT"};

  Rprintf("Label values:\n");
  if ( !st || !rt || st->next_index != rt->size ) {
    Rprintf("ERROR\n");
    return;
  }

  for (i = 1; i <= 2; i++) {
    if (1 == i)
      label = st->user;
    else if (2 == i)
      label = st->rdat;

    Rprintf("%s:\t", prefix[i]);
    for ( ; label ; label = label->next)
      Rprintf("%s=%d  ", label->name, get_reftab(rt, label->ref, cpos));
    Rprintf("\n");
  }
}

/*
 * Queue of waiting FSA simulation states, used in the implementation of region elements (<<name>>) in finite-state queries
 */

/**
 * Allocate a new FSA simulation state object based on the specified symbol table.
 *
 * @param st  a SymbolTable object, determines the label references stored in the FSA state
 *
 * @return    a newly allocated FSAState object, must be deallocated by user code with `FSAState_delete`
 */
FSAState FSAState_new(SymbolTable st) {
  FSAState self;
  assert(st != NULL);
  self = cl_malloc(sizeof(struct _FSAState));
  self->cpos = -1;
  self->next = NULL;
  self->rt = new_reftab(st);
  return self;
}

/**
 * Deallocate FSAState object, freeing all associated memory.
 *
 * @param self_ref  reference to an FSAState object, i.e. a pointer to a variable of type FSAState
 *
 * `*self_ref` will be set to NULL after deallocation. It is valid to pass a reference to a NULL value
 * (i.e. `*self_ref == NULL`), in which case the function is a no-op.
 */
void FSAState_delete(FSAState *self_ref) {
  assert(self_ref != NULL);
  if (*self_ref == NULL)
    return;
  delete_reftab((*self_ref)->rt);
  cl_free(*self_ref); /* also sets *self_ref to NULL */
}

/**
 * Allocate a new StateQueue object, i.e. a wait list of FSA simulation states based on the same symbol table.
 *
 * @param st  a SymbolTable object, determines the label references stored in the FSA state waiting in the queue;
 *            note that the symbol table need not be complete when `StateQueue_new` is called, only when
 *            the first state enters the queue
 *
 * @return    a newly allocated StateQueue object, must be deallocated by user code with `StateQueue_delete`
 */
StateQueue StateQueue_new(SymbolTable st) {
  StateQueue self;
  assert(st != NULL);
  self = cl_malloc(sizeof(struct _StateQueue));
  self->head = NULL;
  self->st = st; /* weak reference, do not deallocate */
  self->n_states = 0;
  return self;
}

/**
 * Deallocate StateQueue object, freeing all remaining FSAState objects in the wait list.
 *
 * @param self_ref  reference to a StateQueue object, i.e. a pointer to a variable of type StateQueue
 *
 * `*self_ref` will be set to NULL after deallocation. It is valid to pass a reference to a NULL value
 * (i.e. `*self_ref == NULL`), in which case the function is a no-op.
 */
void StateQueue_delete(StateQueue *self_ref) {
  FSAState this, next;
  assert(self_ref != NULL);
  if (*self_ref == NULL)
    return;
  for (this = next = (*self_ref)->head;
      this;
      this = next) {
    next = this->next;
    FSAState_delete(&this);
  }
  /* (*self_ref)->st is a weak reference, don't deallocate here */
  cl_free(*self_ref); /* also sets *self_ref to NULL */
}

/**
 * Clear the StateQueue, i.e. remove all remaining FSAState objects from the wait list.
 *
 * @param self  a StateQueue object
 */
void StateQueue_clear(StateQueue self) {
  FSAState this;
  assert(self != NULL);
  while (self->head) {
    this = self->head;
    self->head = this->next;
    FSAState_delete(&this); /* TODO: move state to free list instead of deallocating */
  }
  self->n_states = 0;
}


/**
 * Enter FSA state into the wait list, sorted by increasing `cpos`.
 *
 * @param self  a StateQueue object
 * @param cpos  corpus position of the FSA state entering the queue, determining when it will be emitted
 * @param rt    a RefTab object containing the label references of the FSA state;
 *              must be based on the same SymbolTable as the StateQueue object
 *
 * @return      pointer to the RefTab object inside the state in the wait list, which contains a copy of `rt`
 *
 * If there are already one or more states with the same cpos in the list,
 * the new state will be inserted after them (so a competing state that has
 *  entered the queue earlier will also be emitted first.
 */
RefTab StateQueue_push(StateQueue self, int cpos, RefTab rt) {
  FSAState state, *this_ref;

  assert(self != NULL);
  assert(rt != NULL);
  state = FSAState_new(self->st);
  dup_reftab(rt, state->rt); /* throws an exception if reference tables don't have the same size */
  state->cpos = cpos;
  /* this_ref is a reference to variable holding a pointer to the currently selected element in the wait list;
   * the variable can either be self->head (first) or a FSAState.next field (later) */
  this_ref = &(self->head);
  while (*this_ref && (*this_ref)->cpos <= cpos)
    this_ref = &((*this_ref)->next);  /* advance over all waiting states with same or smaller cpos */
  /* this_ref now references the variable holding the pointer where state should be inserted */
  state->next = *this_ref;
  *this_ref = state;
  self->n_states++;
  return state->rt;
}

/**
 * Remove next FSA state to be emitted from the wait list.
 *
 * @param self       a StateQueue object
 * @param output_rt  a RefTab object to be overwritten with the label references of the waiting FSA state;
 *                   must be based on the same SymbolTable as the StateQueue
 *
 * @return cpos      the cpos of the FSA state to be emitted; -1 if the queue was already empty
 */
int StateQueue_pop(StateQueue self, RefTab output_rt) {
  FSAState state;
  int cpos;

  assert(self != NULL);
  assert(output_rt != NULL);
  state = self->head;
  if (state == NULL)
    return -1;
  dup_reftab(state->rt, output_rt); /* copy reference table of emitted state */
  cpos = state->cpos;               /* save its cpos as return value */
  self->head = state->next;         /* unchain the state from the wait list */
  FSAState_delete(&state);          /* TODO: move state to free list instead, in order to avoid frequent double malloc() calls */
  self->n_states--;
  return cpos;
}

/**
 * Return cpos of next FSA state waiting in the queue (or -1 if there are no waiting states).
 *
 * @param self  a StateQueue object
 *
 * @return      the cpos of the next waiting FSA state, or -1 if the queue is empty
 */
int StateQueue_next_cpos(StateQueue self) {
  if (self->head == NULL)
    return -1;
  else
    return self->head->cpos;
}

/**
 * Return number of FSA states currently waiting in the queue.
 *
 * @param self  a StateQueue object
 *
 * @return      the number of waiting FSA states (0 if the queue is empty)
 */
int StateQueue_length(StateQueue self) {
  return self->n_states;
}

