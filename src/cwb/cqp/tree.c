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
#include <string.h>
#include <assert.h>

#include "../cl/cl.h"

#include "cqp.h"
#include "eval.h"
#include "treemacros.h"
#include "tree.h"
#include "options.h"
#include "symtab.h"
#include "builtins.h"
#include "matchlist.h"

/* helper function for pretty-printing functions below */
void print_indent(int indent) {
  for (; indent > 0; indent--)
    Rprintf("    ");
}

/**
 * Pretty-prints a single query element (i.e. a transition of the FSA)
 *
 * @param envidx  Index into the Environment global array.
 * @param index   Index into the patternlist element of that element of environment.
 * @param indent  The indent level to start printing at.
 */
static void
print_pattern(int envidx, int index, int indent)
{
  if (index >= 0 && index <= Environment[envidx].MaxPatIndex)
    switch (Environment[envidx].patternlist[index].type) {
    case Tag:
      Rprintf("#%d: <%s%s", index,
             (Environment[envidx].patternlist[index].tag.is_closing ? "/" : ""),
             Environment[envidx].patternlist[index].tag.attr->any.name);
      if (Environment[envidx].patternlist[index].tag.constraint) {
        Rprintf(" %s", Environment[envidx].patternlist[index].tag.constraint);
        if (Environment[envidx].patternlist[index].tag.flags)
          Rprintf(" %s%s%s%s", "%",
                 (Environment[envidx].patternlist[index].tag.flags & IGNORE_CASE) ? "c" : "",
                 (Environment[envidx].patternlist[index].tag.flags & IGNORE_DIAC) ? "d" : "",
                 (Environment[envidx].patternlist[index].tag.flags & IGNORE_REGEX) ? "l" : "");
      }
      Rprintf(">\n");
      break;
    case Pattern:
      Rprintf("#%d: [...]\n", index);
      print_indent(indent);
      print_booltree(Environment[envidx].patternlist[index].con.constraint, indent);
      break;
    case MatchAll:
      Rprintf("#%d: []\n", index);
      break;
    case Region:
      Rprintf("#%d: <<%s>> ", index, Environment[envidx].patternlist[index].region.name);
      switch (Environment[envidx].patternlist[index].region.opcode) {
      case AVSRegionEnter:
        Rprintf("ENTER ");
        if (Environment[envidx].patternlist[index].region.attr)
          Rprintf("(s-attribute)");
        else if (Environment[envidx].patternlist[index].region.nqr)
          Rprintf("(NQR)");
        else
          Rprintf("!! invalid !!");
        break;
      case AVSRegionWait:
        Rprintf("WAIT");
        break;
      case AVSRegionEmit:
        Rprintf("EMIT");
        break;
      default:
        Rprintf("!! INVALID !!");
      }
      Rprintf("\n");
      break;
    default:
      Rprintf("Unknown pattern type in print_pattern: %d\n", Environment[envidx].patternlist[index].type);
      break;
    }
  else
    Rprintf("Illegal index in print_pattern: %d\n", index);
}



/**
 * Translates the symbolic value for the repetition arguments.
 *
 * (The translated value is printed as a string to STDOUT.)
 *
 * @param i  The repetition argument symbol to translate.
 */
static void
print_rep_factor(int i)
{
  switch (i) {
  case repeat_inf:
    Rprintf("inf");
    break;
  case repeat_none:
    Rprintf("none");
    break;
  default:
    Rprintf("%d", i);
    break;
  }
}

/**
 * Pretty-prints an evaluation tree.
 *
 * This function traverses the evaluation tree in infix order and
 * prints it appropriately indented.
 *
 * @param envidx  Index into the Environment global array.
 * @param etptr   The evaluation tree to print.
 * @param indent  The indent level to start printing at.
 */
void
print_evaltree(int envidx, Evaltree etptr, int indent)
{
  int i;

  if (!etptr)
    return;

  switch (etptr->type) {

  case node:
    switch(etptr->node.op_id) {

    case re_od_concat:
      assert(etptr->node.min == repeat_none);
      assert(etptr->node.min == repeat_none);
      print_evaltree(envidx, etptr->node.left, indent + 1);
      print_indent(indent);
      Rprintf("    |\n");
      print_indent(indent);
      Rprintf("[.]-+\n");
      print_indent(indent);
      Rprintf("    |\n");
      print_evaltree(envidx, etptr->node.right, indent + 1);
      break;

    case re_oi_concat:
      assert(etptr->node.min == repeat_none);
      assert(etptr->node.min == repeat_none);
      print_evaltree(envidx, etptr->node.left, indent + 1);
      print_indent(indent);
      Rprintf("    |\n");
      print_indent(indent);
      Rprintf("[,]-+\n");
      print_indent(indent);
      Rprintf("    |\n");
      print_evaltree(envidx, etptr->node.right, indent + 1);
      break;

    case re_disj:
      assert(etptr->node.min == repeat_none);
      assert(etptr->node.min == repeat_none);
      print_evaltree(envidx, etptr->node.left, indent + 1);
      print_indent(indent);
      Rprintf("    |\n");
      print_indent(indent);
      Rprintf("[|]-+\n");
      print_indent(indent);
      Rprintf("    |\n");
      print_evaltree(envidx, etptr->node.right, indent + 1);
      break;

    case re_repeat:
      assert(etptr->node.min != repeat_none);
      assert(etptr->node.min != repeat_none);
      print_evaltree(envidx, etptr->node.left, indent + 1);
      print_indent(indent);
      Rprintf("    |\n");
      print_indent(indent);
      Rprintf("[*]-+  { ");
      print_rep_factor(etptr->node.min);
      Rprintf(" , ");
      print_rep_factor(etptr->node.max);
      Rprintf(" }\n");
      assert(etptr->node.right == NULL);
      break;
    }
    break;

  case leaf:
    print_indent(indent);
    if ((etptr->leaf.patindex >= 0) && (etptr->leaf.patindex <= Environment[envidx].MaxPatIndex))
      print_pattern(envidx, etptr->leaf.patindex, indent);

    break;

  case meet_union:
    Rprintf("\n");
    for (i = 1; i <= indent; i++)
      Rprintf("  ");

    switch (etptr->cooc.op_id) {
    case cooc_meet:
      Rprintf("Meet <%d/%d, %s>", etptr->cooc.lw, etptr->cooc.rw, etptr->cooc.struc ? etptr->cooc.struc->any.name : "words");
      break;
    case cooc_union:
      Rprintf("Union ");
      break;
    default:
      assert(0 && "Can't be");
      break;
    }
    print_evaltree(envidx, etptr->cooc.left, indent + 1);
    print_evaltree(envidx, etptr->cooc.right, indent + 1);

    break;

  case tabular:
    Rprintf("Tabular\n");
    while (etptr) {
      print_pattern(0, etptr->tab_el.patindex, 2);
      /* print the distance? */
      if (etptr->tab_el.next)
        Rprintf("  {%d,%d}\n", etptr->tab_el.next->tab_el.min_dist,  etptr->tab_el.next->tab_el.max_dist);
      etptr = etptr->tab_el.next;
    }

    break;
  }
}



/**
 * Deletes a boolean constraint tree node (recursive with depth-first).
 *
 * @param ctptr  The Constrainttree to delete. Null-safe.
 */
void
free_booltree(Constrainttree ctptr)
{
  ActualParamList *arg, *next;

  if (!ctptr)
    return;

  /* denotes the current node an operator? */
  switch (ctptr->type) {

  case bnode:
    free_booltree(ctptr->node.left);
    free_booltree(ctptr->node.right);
    break;

  case cnode:
    /* do nothing */
    break;

  case id_list:
    cl_free(ctptr->idlist.items);
    break;

  case var_ref:
    cl_free(ctptr->varref.varName);
    break;

  case func:
    for (arg = ctptr->func.args; arg; arg = next) {
      next = arg->next;
      free_booltree(arg->param);
      cl_free(arg);
    }
    break;

  case sbound:
    assert("Should not be" && 0);
    break;

  case pa_ref:
  case sa_ref:
    /* nothing to do */
    break;

  case string_leaf:
    switch (ctptr->leaf.pat_type) {
    case REGEXP:
      cl_delete_regex(ctptr->leaf.rx);
      /* fallthrough */
    case NORMAL:
      cl_free(ctptr->leaf.ctype.sconst);
      break;
    case CID:
      /* nothing to do */
      break;
    }
    break;

  case int_leaf:
  case float_leaf:
    /* nothing to do */
    break;
  }
  cl_free(ctptr);
}



/**
 * Deletes an evaluation tree node, and every node under it, with the depth-first method.
 *
 * @param tree_ptr  The evaluation tree to delete. This is a pointer to a pointer since
 *                  Evaltree is itself a pointer type.
 */
void
free_evaltree(Evaltree *tree_ptr)
{
  if (!(*tree_ptr))
    return;

  /* What type of node are we freeing? */
  switch ((*tree_ptr)->node.type) {

    /* nb the asserts in this switch aren't really necessary except as sanity checks, since
     * it is quite OK to call this function on a NULL pointer because of the initial "if". */

  case node:
    switch((*tree_ptr)->node.op_id) {

      /* delete the binary operator nodes: two children. */
    case re_od_concat:
    case re_oi_concat:
    case re_disj:
      /* assert that both children exist */
      assert((*tree_ptr)->node.left && (*tree_ptr)->node.right);
      /* delete left child, then right child, then current op node */
      free_evaltree(&((*tree_ptr)->node.left));
      free_evaltree(&((*tree_ptr)->node.right));
      cl_free((*tree_ptr));
      break;

      /* delete the unary operator nodes: one child */
    case re_repeat:
      free_evaltree(&((*tree_ptr)->node.left));
      cl_free((*tree_ptr));
      break;
    }
    break;

  case meet_union:
    assert((*tree_ptr)->cooc.left && (*tree_ptr)->cooc.right );
    free_evaltree(&((*tree_ptr)->cooc.left));
    free_evaltree(&((*tree_ptr)->cooc.right));
    cl_free((*tree_ptr));
    break;

  case leaf:
    /* free the current leaf */
    cl_free(*tree_ptr);
    break;

  case tabular:
    /* delete the next node */
    free_evaltree(&((*tree_ptr)->tab_el.next));
    /* and now this one! */
    cl_free((*tree_ptr));
    break;
  }
}

/**
 * Prints a boolean evaluation tree.
 *
 * This function is a pretty-printer for the Constrainttree data type. It
 * traverses a boolean evaluation tree and prints its contents. An indentation
 * level must be specified.
 *
 * (The indentation is needed because this function calls itself recursively for sub-branches.)
 *
 * @param ctptr   Constrainttree to print.
 * @param indent  Number of indent levels at which to start printing. Each
 *                indent level is realised as two spaces.
 */
void
print_booltree(Constrainttree ctptr, int indent)
{
  int i;
  ActualParamList *arg;

  if (!ctptr)
    return;

  if (tree_debug)
    Rprintf("booltree is not nil\n");
  /* denotes the current node an operator? */
  switch (ctptr->type) {
  case bnode:

    if (tree_debug)
      Rprintf("current node is operator (type = %d)\n", ctptr->node.type);

    switch(ctptr->node.op_id) {
    case b_and:
    case b_or:
    case b_implies:
    case cmp_gt:
    case cmp_lt:
    case cmp_get:
    case cmp_let:
    case cmp_eq:
    case cmp_neq:

      if (tree_debug)
        Rprintf("operator (id = %d) is binary\n", ctptr->node.op_id);

      print_booltree(ctptr->node.left, indent + 1);
      Rprintf("\n");
      print_indent(indent);
      switch(ctptr->node.op_id) {
      case b_and:
        Rprintf("&\n");
        break;
      case b_or:
        Rprintf("|\n");
        break;
      case b_implies:
        Rprintf("->\n");
        break;
      case cmp_gt:
        Rprintf(">\n");
        break;
      case cmp_lt:
        Rprintf("<\n");
        break;
      case cmp_get:
        Rprintf(">=\n");
        break;
      case cmp_let:
        Rprintf("<=\n");
        break;
      case cmp_eq:
        Rprintf("=\n");
        break;
      case cmp_neq:
        Rprintf("!=\n");
        break;
      default:
        break;
      }
      print_booltree(ctptr->node.right, indent + 1);
      break;

    case b_not:
    case cmp_ex:
      if (tree_debug)
        Rprintf("operator (id = %d) is unary\n", ctptr->node.op_id);
      Rprintf("\n");
      print_indent(indent);

      switch(ctptr->node.op_id) {
      case b_not:
        Rprintf("!\n");
        break;
      case cmp_ex:
        Rprintf("?\n");
        break;
      default:
        Rprintf("ILLEGAL OP: %d\n", ctptr->node.op_id);
        break;
      }

      print_booltree(ctptr->node.left, indent + 1);
      break;

    default:
      if (tree_debug)
        Rprintf("operator (id = %d) is unknown\n", ctptr->node.op_id);
      break;
    }
    break;

  case cnode:
    Rprintf("constant %d\n", ctptr->constnode.val);
    break;

  case id_list:
    if (ctptr->idlist.label)
      Rprintf("%smembership of %s.%s value in ",
             ctptr->idlist.negated ? "non-" : "",
             ctptr->idlist.label->name,
             ctptr->idlist.attr->any.name);
    else
      Rprintf("%smembership of %s value in ",
             ctptr->idlist.negated ? "non-" : "",
             ctptr->idlist.attr->any.name);
    for (i = 0; i < ctptr->idlist.nr_items; i++)
      Rprintf("%d ", ctptr->idlist.items[i]);
    Rprintf("\n");
    break;

  case var_ref:
    Rprintf("Variable reference to %s\n", ctptr->varref.varName);
    break;

  case func:
    Rprintf("\n");
    print_indent(indent);

    if (ctptr->func.predef >= 0)
      Rprintf("%s(", builtin_function[ctptr->func.predef].name);
    else {
      assert(ctptr->func.dynattr);
      Rprintf("%s(", ctptr->func.dynattr->any.name);
    }

    for (arg = ctptr->func.args; arg; arg = arg->next) {
      print_booltree(arg->param, indent+1);
      if (arg->next)
        Rprintf(", ");
    }
    Rprintf(")\n");

    break;

  case sbound:
    assert("Not reached" && 0);
    break;

  case pa_ref:
    Rprintf("\n");
    print_indent(indent);

    if (ctptr->pa_ref.label)
      Rprintf("%s.", ctptr->pa_ref.label->name);

    if (ctptr->pa_ref.attr)
      Rprintf("%s", ctptr->pa_ref.attr->any.name);
    else
      /* we may have label references without an attribute,
       * referring to the position only (distance)
       */
      assert(ctptr->pa_ref.label);

    break;

  case sa_ref:
    Rprintf("%s", ctptr->pa_ref.attr->any.name);
    break;

  case string_leaf:
    Rprintf("\n");
    print_indent(indent);

    switch (ctptr->leaf.pat_type) {
    case REGEXP:
      Rprintf("REGEX %s\n", ctptr->leaf.ctype.sconst);
      break;
    case NORMAL:
      Rprintf("NORMAL %s\n", ctptr->leaf.ctype.sconst);
      break;
    case CID:
      Rprintf("CID %d\n", ctptr->leaf.ctype.cidconst);
      break;
    }
    break;

  case int_leaf:
    Rprintf("\n");
    print_indent(indent);
    Rprintf("%d\n", ctptr->leaf.ctype.iconst);
    break;

  case float_leaf:
    Rprintf("\n");
    print_indent(indent);
    Rprintf("%f\n", ctptr->leaf.ctype.fconst);
    break;

  default:
    Rprintf("ILLEGAL EVAL NODE TYPE: %d\n", ctptr->type);
    break;
  }
}


/**
 * Shows the contents of the patternlist.
 *
 * (Prints to STDOUT.)
 *
 * @param eidx  Index into the global Environment array,
 *              identifying the EvalEnvironment
 *              whose patternlist is to be printed.
 */
void
show_patternlist(int eidx)
{
  int i;

  Rprintf("\n==================== Pattern List:\n\n");

  Rprintf("Size: %d\n", Environment[eidx].MaxPatIndex + 1);

  for(i = 0; i <= Environment[eidx].MaxPatIndex; i++) {
    Rprintf("Pattern #%d:\n", i);
    print_pattern(eidx, i, 0);
  }

  Rprintf("\n==================== End of Pattern List\n\n");
}



/**
 * Converts an evaluation tree to a string.
 *
 * This is done by traversing the tree in infix order,
 * calling this function recursively.
 *
 * @param etptr   The evaluation tree to convert.
 * @param length  Out paramter: length of the resulting string is placed here.
 *                Can be NULL if the length is unwanted.
 * @return        The resulting string.
 */
char *
evaltree2searchstr(Evaltree etptr, int *length)
{
  int n, p, l, min, max, remain, ldump;

  char *left, *right, *result = NULL;
  int len_l, len_r;

  if (!length)
    length = &ldump;
  *length = 0;

  if (!etptr)
    return NULL;

  if (etptr->node.type == node) {

    switch(etptr->node.op_id) {

    case re_od_concat:
    case re_oi_concat:
      assert(etptr->node.min == repeat_none);
      assert(etptr->node.min == repeat_none);

      left  = evaltree2searchstr(etptr->node.left,  &len_l);
      right = evaltree2searchstr(etptr->node.right, &len_r);
      *length = len_l + len_r + 1;
      result = (char *)cl_malloc(*length);
      snprintf(result, *length, "%s %s", left, right);
      cl_free(left);
      cl_free(right);
      break;

    case re_disj:
      assert(etptr->node.min == repeat_none);
      assert(etptr->node.min == repeat_none);

      left  = evaltree2searchstr(etptr->node.left, &len_l);
      right = evaltree2searchstr(etptr->node.right, &len_r);
      *length = len_l + len_r + 7;
      result = (char *)cl_malloc(*length);
      snprintf(result, *length, "( %s | %s )", left, right);
      cl_free(left);
      cl_free(right);
      break;

    case re_repeat:
      assert(etptr->node.min != repeat_none);

      left = evaltree2searchstr(etptr->node.left, &len_l);
      min = etptr->node.min;
      max = etptr->node.max;

      /* check the special cases first */

      if (min == 0 && max == repeat_inf) {
        *length = len_l + 5;
        result = (char *)cl_malloc(*length);
        snprintf(result, *length, "( %s )*", left);
        cl_free(left);
      }

      else if ((min == 1) && (max == repeat_inf)) {
        *length = len_l + 5;
        result = (char *)cl_malloc(*length);
        snprintf(result, *length, "( %s )+", left);
        cl_free(left);
      }

      else if ((min == 0) && (max == 1)) {
        *length = len_l + 4;
        result = (char *)cl_malloc(*length);
        snprintf(result, *length, "[ %s ]", left);
        cl_free(left);
      }

      else {
        if (max == repeat_inf)
          remain = repeat_inf;
        else
          remain = max - min;

        /* we need
         *   min * (len_l + 1) space for the minimum repetitions
         * plus
         *   if max != inf:  max - min * (len_l + 4)
         *   else:           len_l + 5
         * space for the string.
         */

        *length = min * (len_l + 1);

        if (remain == repeat_inf)
          *length = *length + len_l + 5;
        else
          *length = *length + (remain * (len_l + 4));

        result = (char *)cl_malloc(*length);

        p = 0;                /* index into result */

        /* copy the minimum repetitions  */

        for(n = 0; n < min; n++) {
          for (l = 0; left[l]; l++, p++)
            result[p] = left[l];
          result[p++] = ' ';
        }

        if (remain == repeat_inf) {
          result[p++] = '(';
          result[p++] = ' ';

          for (l = 0; left[l]; l++, p++)
            result[p] = left[l];

          result[p++] = ' ';
          result[p++] = ')';
          result[p++] = '*';

        }
        else {
          for (n = 0; n < remain; n++) {
            result[p++] = '[';
            for (l = 0; left[l]; l++, p++)
              result[p] = left[l];
            result[p++] = ' ';
          }
          for (n = 0; n < remain; n++)
            result[p++] = ']';
        }
        result[p] = '\0';
        cl_free(left);
      }
      break;
    }
  }

  else {
    /* etptr->node.type is something other than node */
    char numstr[16];
    assert(etptr->leaf.type == leaf);
    snprintf(numstr, 16, " \"%d\" ",etptr->leaf.patindex);
    result = cl_strdup(numstr);
    *length = strlen(result) + 1;
  }

  return result;
}




Constraint *
try_optimization(Constraint *tree)
{
  enum b_ops operator;

  Constraint *left;
  Constraint *right;

  if (!tree)
    return NULL;

  if (tree->type != bnode)
    return tree;

  operator = tree->node.op_id;
  left  = tree->node.left;
  right = tree->node.right;

  if (right && right->type == string_leaf) {
    if (left->type == pa_ref) {
      assert(right->leaf.pat_type != NORMAL);

      if (right->leaf.pat_type == CID && right->leaf.ctype.cidconst < 0) {
        /* we have a non-existing CID on the right. Look at the operator now. */
        free_booltree(tree);
        NEW_BNODE(tree);
        tree->type = cnode;
        tree->constnode.val = (operator == cmp_neq ? 1 : 0);
      }
    }
  }

  else if (operator == b_or) {
    Attribute *left_attr = NULL, *right_attr = NULL;
    LabelEntry left_label= NULL, right_label= NULL;
    enum bnodetype left_type = -1, right_type = -1;
    int try_opt = 1;

    /* check the type of left branch */
    if (left->type == bnode &&
        left->node.op_id == cmp_eq &&
        left->node.left->type == pa_ref &&
        left->node.right->type == string_leaf &&
        left->node.right->leaf.pat_type == CID) {
      left_attr = left->node.left->pa_ref.attr;
      left_type = pa_ref;
      left_label = left->node.left->pa_ref.label;
    }
    else if (left->type == id_list) {
      left_attr = left->idlist.attr;
      left_type = id_list;
      left_label = left->idlist.label;
      if (left->idlist.negated)
        try_opt = 0;
    }

    /* check the type of right branch */
    if (right->type == bnode &&
        right->node.op_id == cmp_eq &&
        right->node.left->type == pa_ref &&
        right->node.right->type == string_leaf &&
        right->node.right->leaf.pat_type == CID) {
      right_attr = right->node.left->pa_ref.attr;
      right_type = pa_ref;
      right_label = right->node.left->pa_ref.label;
    }

    else if (right->type == id_list) {
      right_attr = right->idlist.attr;
      right_type = id_list;
      right_label = right->idlist.label;
      if (right->idlist.negated)
        try_opt = 0;
    }

    /* we can try optimization when left and right attribute are equal */
    if (try_opt && left_attr && left_attr == right_attr && left_label == right_label) {
      Matchlist left_list;
      Matchlist right_list;
      init_matchlist(&left_list);
      init_matchlist(&right_list);

      if (left_type == id_list && right_type == id_list) {
        /* merge id lists to a single list */

        left_list.start = left->idlist.items;
        left_list.tabsize = left->idlist.nr_items;

        left->idlist.items = NULL;
        left->idlist.nr_items = 0;

        right_list.start = right->idlist.items;
        right_list.tabsize = right->idlist.nr_items;

        right->idlist.items = NULL;
        right->idlist.nr_items = 0;
      }

      else if (left_type == id_list && right_type == pa_ref) {
        /* add right id(s) to left id list */

        left_list.start = left->idlist.items;
        left_list.tabsize = left->idlist.nr_items;

        left->idlist.items = NULL;
        left->idlist.nr_items = 0;

        right_list.start = (int *)cl_malloc(1 * sizeof(int));
        right_list.tabsize = 1;
        right_list.start[0] = right->node.right->leaf.ctype.cidconst;
      }

      else if (left_type == pa_ref && right_type == id_list) {
        /* add left id(s) to right id list */

        left_list.start = right->idlist.items;
        left_list.tabsize = right->idlist.nr_items;

        right->idlist.items = NULL;
        right->idlist.nr_items = 0;

        right_list.start = (int *)cl_malloc(1 * sizeof(int));
        right_list.tabsize = 1;
        right_list.start[0] = left->node.right->leaf.ctype.cidconst;
      }

      else if (left_type == pa_ref && right_type == pa_ref) {
        /* construct a new id list where both sides are included */

        left_list.start = (int *)cl_malloc(1 * sizeof(int));
        left_list.tabsize = 1;
        left_list.start[0] = left->node.right->leaf.ctype.cidconst;

        right_list.start = (int *)cl_malloc(1 * sizeof(int));
        right_list.tabsize = 1;
        right_list.start[0] = right->node.right->leaf.ctype.cidconst;
      }

      free_booltree(tree);

      apply_setop_to_matchlist(&left_list, Union, &right_list);

      free_matchlist(&right_list);

      NEW_BNODE(tree);
      tree->type = id_list;
      tree->idlist.label = left_label;
      tree->idlist.attr = left_attr;
      tree->idlist.nr_items = left_list.tabsize;
      tree->idlist.items = left_list.start;
      tree->idlist.negated = 0;
    }
  }

  return tree;
}

