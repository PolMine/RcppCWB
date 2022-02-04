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
#include <assert.h>

#include "../cl/cl.h"
#include "../cl/attributes.h"

#include "matchlist.h"
#include "output.h"
#include "eval.h"



/*
 * Functions for manipulation of matching lists
 */

/**
 * Initialise the memebrs of the given Matchlist object.
 */
void
init_matchlist(Matchlist *matchlist)
{
  matchlist->start = NULL;
  matchlist->end =   NULL;
  matchlist->target_positions =  NULL;
  matchlist->keyword_positions = NULL;
  matchlist->tabsize = 0;
  matchlist->matches_whole_corpus = 0;
  matchlist->is_inverted = 0;
}

/**
 * Prints the entire contents of a Matchlist to stdout.
 */
void
show_matchlist(Matchlist matchlist)
{
  int i;

  Rprintf("Matchlist (size: %d, %sinverted):\n", matchlist.tabsize, matchlist.is_inverted ? "" : "not ");

  for (i = 0; i < matchlist.tabsize; i++)
    Rprintf("ml[%d] = [%d, %d] @:%d @9:%d\n",
            i,
            matchlist.start[i],
            matchlist.end[i],
            matchlist.target_positions ? matchlist.target_positions[i] : -1,
            matchlist.keyword_positions ? matchlist.keyword_positions[i] : -1
            );
}

/**
 * Prints the first few elements of a matchlist to stderr.
 *
 * "First few" equals up to 1000.
 *
 * Only the start positions are printed.
 */
void
show_matchlist_firstelements(Matchlist matchlist)
{
  int i;
  int n = (matchlist.tabsize >= 1000 ? 1000 : matchlist.tabsize % 1000);

  Rprintf("the first (max 1000) elements of the matchlist (size: %d) are:\n", matchlist.tabsize);
  for (i = 0; i < n; i++)
    Rprintf("ml[%d] = [%d,...]\n", i, matchlist.start[i]);
}


/**
 * Frees all the memory used within a matchlist, and re-initialises all its variables; null-safe.
 */
void
free_matchlist(Matchlist *matchlist)
{
  if (!matchlist)
    return;
  cl_free(matchlist->start);
  cl_free(matchlist->end);
  cl_free(matchlist->target_positions);
  cl_free(matchlist->keyword_positions);
  init_matchlist(matchlist);
}

static void
_swap_items(int *v, int a, int b) {
  int tmp = v[a];
  v[a] = v[b];
  v[b] = tmp;
}
/**
 * Sort a matchlist in canonical order, i.e. by increasing start and end position.
 *
 * The sort operation is carried out in-place, modifying the function argument. Currently, a bubblesort
 * algorithm is used which doesn't need to allocate scratch memory. While it has bad worst-case complexity,
 * the typical use case (reordering matches from a single pass of a finite-state query) will only contain
 * a small number of local inversions (if any) and allows the bubblesort to stop early.
 *
 * "Special" matchlist (i.e. initial matchlist without end positions, ml->is_inverted, ml->matches_while_corpus)
 * are not allowed.
 *
 * @param ml   matchlist to be sorted (must not be initial or inverted)
 * @return     True if the sort operation was successful
 */
int
sort_matchlist(Matchlist *ml) {
  int top, i, n_changes;

  assert(ml != NULL);
  if (ml->is_inverted || ml->matches_whole_corpus) {
    cqpmessage(Error, "Can't sort an inverted or whole-corpus matchlist.");
    return False;
  }
  if (ml->tabsize == 0) {
    return True; /* trivial case */
  }
  assert(ml->start != NULL);
  if (!ml->end) {
    cqpmessage(Error, "Can't sort an initial matchlist -- not implemented yet");
    return False;
  }

  top = ml->tabsize - 1; /* last non-sorted element index */
  while (top >= 1) {
    n_changes = 0;
    for (i = 0; i < top; i++) {
      if (ml->start[i] > ml->start[i+1] ||
          (ml->start[i] == ml->start[i+1] && ml->end[i] > ml->end[i+1])) {
        /* item i should sort after item (i+1) -> swap the two items */
        _swap_items(ml->start, i, i+1);
        _swap_items(ml->end, i, i+1);
        if (ml->target_positions)
          _swap_items(ml->target_positions, i, i+1);
        if (ml->keyword_positions)
          _swap_items(ml->keyword_positions, i, i+1);
        n_changes++;
      }
    }
    if (n_changes == 0)
      break; /* terminate early if no more changes have been made to the vector */
    top--; /* top now contains the largest element from [0 .. top] -> already in correct position */
  }
  /* use this information message to debug issues with bubblesort speed */
  /* cqpmessage(Info, "Matchlist sort terminated after %d / %d loops\n", ml->tabsize - top, ml->tabsize - 1); */

  return True;
}


/**
 * Perform a "set operation" on the two match lists (can be initial).
 *
 * This is the main functionality provided for Matchlist object.
 * The result of the operation is written to list1, which is modified destructively and may be reallocated.
 *
 * @param list1      first or only argument of the operation, will be overwritten with the result
 * @param operation  operation to apply; only Union, Reduce and Complement are properly supported and used by other CQP code so far
 * @param list2      optional second argument of the operation
 *
 * **Here Be Dragons**
 *  - implementation of Intersection and Unique is dubious and likely buggy (and will produce wrong results for inverted matchlists)
 *  - Union, Intersection and Unique assume that the matchlists are sorted (by increasing start position, then increasing end position)
 *    and will produce incorrect results otherwise; this was not documented anywhere in the source code!
 *  - this sort order is different from the one used by `RangeSort()` (which sorts longer matches first given the same start point);
 *    it is quite unclear what the expected canonical sort order thus is for named query results, and which parts of the CQP code rely
 *    on a particular sort order
 *  - Union is used by CQP to combine results from multiple evaluation passes with different starting points (in finite-state queries);
 *    for each pass the matchlist is sorted appropriately (with start = match anchor increasing by definition, and only a single match
 *    possible for each start position, hence sorting by end = matchend anchor is irrelevant); building up the joint matchlist with Union
 *    ensures that it is also sorted correctly
 *  - if the FSA simulation is changed so that start points of matches can be out of order or there can be multiple matches with the
 *    same start point, the matchlist has to be sorted before further processing; if there could be duplicate matches (with identical
 *    start and end points) they should be discarded by an additional Uniq operation (as they might confuse other operations)
 *  - TODO: implement a suitable Sort operation and revise Uniq, or implement a join SortUniq operation, possibly as a separate function
 *    rather than yet another set operator; this should be relatively straightforward if we don't mind allocating new vectors rather than
 *    modifying the existing ones in place (at the cost of considerable memory overhead)
 *
 * **Some old notes:**
 *
 * TODO: this whole code is WRONG when one of the matchlists is inverted
 * TODO: many operations expect that the matchlist(s) is/are consistently sorted, but don't specify the expected sort order
 *
 * Fortunately, CQP only uses the operations Union, Reduce and Complement (only sensible for initial matchlists) so far, while Identity is used internally.
 * Other operations may have been intended for matching the first pattern of a finite-state query with a MU-style algorithm,
 * but clearly haven't been thought through at all.
 */
int
apply_setop_to_matchlist(Matchlist *list1, MLSetOp operation, Matchlist *list2)
{
  int i, j, k, t, ins;
  Matchlist tmp;
  Attribute *attr;

  switch (operation) {

  case Union:

    /*
     * -------------------- UNION
     */

    /*
     * TODO: optimize in case  (list1->matches_whole_corpus && list2->matches_whole_corpus)
     */

    if (list2->start == NULL) {

      if (list2->is_inverted)
        /* l2 is empty, but inverted, so the result is the whole corpus,
         * as in l2. */
        return apply_setop_to_matchlist(list1, Identity, list2);
      else
        /* the result is list1, so just return */
        return 1;
    }

    else if (list1->start == NULL) {

      if (list1->is_inverted)
        /* empty, but inverted --> whole corpus, list1 */
        return 1;
      else
        /* the result is in list2, so return a copy */
        return apply_setop_to_matchlist(list1, Identity, list2);
    }

    else if (list1->is_inverted && list2->is_inverted) {

      /* union of 2 inverted lists is the inverted intersection */
      list1->is_inverted = 0;
      list2->is_inverted = 0;
      apply_setop_to_matchlist(list1, Intersection, list2);
      list1->is_inverted = 1;

    }
    else {

      if (list1->is_inverted) {
        list1->is_inverted = 0;
        apply_setop_to_matchlist(list1, Complement, NULL);
      }
      if (list2->is_inverted) {
        list2->is_inverted = 0;
        apply_setop_to_matchlist(list2, Complement, NULL);
      }

      tmp.tabsize = list1->tabsize + list2->tabsize;

      tmp.start = (int *)cl_malloc(sizeof(int) * tmp.tabsize);

      if (list1->end && list2->end)
        tmp.end   = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      else
        tmp.end = NULL;

      if (list1->target_positions && list2->target_positions)
        tmp.target_positions = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      else
        tmp.target_positions = NULL;
      if (list1->keyword_positions && list2->keyword_positions)
        tmp.keyword_positions = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      else
        tmp.keyword_positions = NULL;


      i = 0;                        /* the position in list1 */
      j = 0;                        /* the position in list2 */
      k = 0;                        /* the insertion point in the result list `tmp' */


      while ((i < list1->tabsize) || (j < list2->tabsize)) {

        /* only one condition satisfied per loop. */
        if ((i < list1->tabsize) && (list1->start[i] == -1))
          i++;
        else if ((j < list2->tabsize) && (list2->start[j] == -1))
          j++;
        else if ((j >= list2->tabsize) || ((i < list1->tabsize) && (list1->start[i] < list2->start[j]))) {

          /* copy (remaining) item from list1 */
          tmp.start[k] = list1->start[i];

          if (tmp.end)
            tmp.end[k] = list1->end[i];

          if (tmp.target_positions)
            tmp.target_positions[k] = list1->target_positions[i];
          if (tmp.keyword_positions)
            tmp.keyword_positions[k] = list1->keyword_positions[i];

          k++;
          i++;

        }
        else if ((i >= list1->tabsize) || ((j < list2->tabsize) && (list1->start[i] > list2->start[j]))) {

          /* copy (remaining) item from list2 */
          tmp.start[k] = list2->start[j];

          if (tmp.end)
            tmp.end[k] = list2->end[j];

          if (tmp.target_positions)
            tmp.target_positions[k] = list2->target_positions[j];
          if (tmp.keyword_positions)
            tmp.keyword_positions[k] = list2->keyword_positions[j];

          k++;
          j++;

        }
        else {
          /* both start positions are identical. Now check whether the end
           * positions are also the same => the ranges are identical and
           * the duplicate is to be eliminated.
           */
          tmp.start[k] = list1->start[i];

          if ((tmp.end == NULL) || (list1->end[i] == list2->end[j])) {

            /* real duplicate, copy once */
            if (tmp.end)
              tmp.end[k]   = list1->end[i];

            if (tmp.target_positions)
              tmp.target_positions[k]  = list1->target_positions[i];
            if (tmp.keyword_positions)
              tmp.keyword_positions[k] = list1->keyword_positions[i];

            i++;
            j++;

          }
          else {

            /* we have existing, non-equal end positions. copy the smaller one. */
            if (list1->end[i] < list2->end[j]) {
              tmp.end[k]   = list1->end[i];

              if (tmp.target_positions)
                tmp.target_positions[k] = list1->target_positions[i];
              if (tmp.keyword_positions)
                tmp.keyword_positions[k] = list1->keyword_positions[i];
              i++;
            }
            else {
              tmp.end[k]   = list2->end[j];

              if (tmp.target_positions)
                tmp.target_positions[k] = list2->target_positions[j];
              if (tmp.keyword_positions)
                tmp.keyword_positions[k] = list2->keyword_positions[j];
              j++;
            }

          }
          k++;
        }
      } /* endwhile */
      assert(k <= tmp.tabsize);

      /* we did not eliminate any duplicates if k==tmp.tabsize.
       * So, in that case, we do not have to bother with reallocs.
       */

      if (k < tmp.tabsize) {
        tmp.start = (int *)cl_realloc((char *)tmp.start, sizeof(int) * k);
        if (tmp.end)
          tmp.end = (int *)cl_realloc((char *)tmp.end, sizeof(int) * k);
        if (tmp.target_positions)
          tmp.target_positions = (int *)cl_realloc((char *)tmp.target_positions, sizeof(int) * k);
        if (tmp.keyword_positions)
          tmp.keyword_positions = (int *)cl_realloc((char *)tmp.keyword_positions, sizeof(int) * k);
      }

      cl_free(list1->start);
      cl_free(list1->end);
      cl_free(list1->target_positions);
      cl_free(list1->keyword_positions);

      list1->start = tmp.start;
      tmp.start = NULL;
      list1->end   = tmp.end;
      tmp.end = NULL;
      list1->target_positions  = tmp.target_positions;
      tmp.target_positions = NULL;
      list1->keyword_positions = tmp.keyword_positions;
      tmp.keyword_positions = NULL;
      list1->tabsize = k;
      list1->matches_whole_corpus = 0;
      list1->is_inverted = 0;
    }

    break;

  case Intersection:

    /*
     * -------------------- INTERSECTION
     */

    if (0 == list1->tabsize && list1->is_inverted)
      /* l1 matches whole corpus, so intersection is equal to l2 */
      return apply_setop_to_matchlist(list1, Identity, list2);

    else if (0 == list2->tabsize && list2->is_inverted)
      /* l2 matches whole corpus, so intersection is equal to l1 */
      return 1;

    else if (0 == list1->tabsize || 0 == list2->tabsize) {
      /*
       * one of the two is empty AND NOT INVERTED.
       * so the intersection is also empty.
       */
      cl_free(list1->start);
      cl_free(list1->end);
      cl_free(list1->target_positions);
      cl_free(list1->keyword_positions);
      list1->tabsize = 0;
      list1->matches_whole_corpus = 0;
      list1->is_inverted = 0;
    }

    else if (list1->is_inverted && list2->is_inverted) {
      /*
       * intersection of 2 inverted lists is the inverted union
       */
      list1->is_inverted = 0;
      list2->is_inverted = 0;
      apply_setop_to_matchlist(list1, Union, list2);
      list1->is_inverted = 1;
    }

    else {
      /*
       * Two non-empty lists. ONE or both may be inverted.
       * We have to do some work then
       */
      if (list1->is_inverted)
        tmp.tabsize = list2->tabsize;
      else if (list2->is_inverted)
        tmp.tabsize = list1->tabsize;
      else
        tmp.tabsize = MIN(list1->tabsize, list2->tabsize);

      tmp.start = (int *)cl_malloc(sizeof(int) * tmp.tabsize);

      if (list1->end && list2->end)
        tmp.end = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      else
        tmp.end = NULL;

      if (list1->target_positions && list2->target_positions)
        tmp.target_positions = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      else
        tmp.target_positions = NULL;
      if (list1->keyword_positions && list2->keyword_positions)
        tmp.keyword_positions = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      else
        tmp.keyword_positions = NULL;

      i = 0;                        /* the position in list1 */
      j = 0;                        /* the position in list2 */
      k = 0;                        /* the insertion point in the result list */

      while (i < list1->tabsize && j < list2->tabsize) {
        if (list1->start[i] < list2->start[j])
          i++;

        else if (list1->start[i] > list2->start[j])
          j++;

        else {
          /* both start positions are identical. Now check whether the end
           * positions are also the same => the ranges are identical and
           * one version is to be copied.
           */
          if (!tmp.end || list1->end[i] == list2->end[j]) {
            /*
             * real duplicate, copy once
             */
            tmp.start[k] = list1->start[i];
            if (tmp.end)
              tmp.end[k]   = list1->end[i];
            if (tmp.target_positions)
              tmp.target_positions[k]   = list1->target_positions[i];
            if (tmp.keyword_positions)
              tmp.keyword_positions[k]   = list1->keyword_positions[i];
            i++;
            j++;
            k++;
          }
          else {
            /*
             * we have existing, non-equal end positions. Advance on
             * list with the smaller element.
             */
            if (list1->end[i] < list2->end[j])
              i++;
            else
              j++;
          }
        }
      }

      assert(k <= tmp.tabsize);

      if (k == 0) {
        /* we did not copy anything. result is empty. */
        cl_free(tmp.start);
        tmp.start = NULL;
        cl_free(tmp.end);
        tmp.end = NULL;
        cl_free(tmp.target_positions);
        tmp.target_positions = NULL;
        cl_free(tmp.keyword_positions);
        tmp.keyword_positions = NULL;
      }
      else if (k < tmp.tabsize) {
        /*
         * we did not eliminate any duplicates if k==tmp.tabsize.
         * So, in that case, we do not have to bother with reallocs.
         */
        tmp.start = (int *)cl_realloc((char *)tmp.start, sizeof(int) * k);
        if (tmp.end)
          tmp.end = (int *)cl_realloc((char *)tmp.end, sizeof(int) * k);
        if (tmp.target_positions)
          tmp.target_positions = (int *)cl_realloc((char *)tmp.target_positions, sizeof(int) * k);
        if (tmp.keyword_positions)
          tmp.keyword_positions = (int *)cl_realloc((char *)tmp.keyword_positions, sizeof(int) * k);
      }

      cl_free(list1->start);
      cl_free(list1->end);
      cl_free(list1->target_positions);
      cl_free(list1->keyword_positions);

      list1->start = tmp.start;
      tmp.start = NULL;
      list1->end = tmp.end;
      tmp.end = NULL;

      list1->target_positions   = tmp.target_positions;
      tmp.target_positions = NULL;
      list1->keyword_positions  = tmp.keyword_positions;
      tmp.keyword_positions = NULL;

      list1->tabsize = k;
      list1->matches_whole_corpus = 0;
      list1->is_inverted = 0;
    }

    break;

  case Complement:

    /*
     * -------------------- COMPLEMENT
     * in that case. ML2 should be empty. We suppose it is.
     */

    /* what the hell is the complement of a non-initial matchlist?
     * I simply do not know. so do it only for initial ones. */
    if (list1->end) {
      Rprintf("Can't calculate complement for non-initial matchlist.\n");
      return 0;
    }

    /* we could always make the complement by toggling the inversion flag,
     * but we only do that in case the list is inverted, otherwise we would
     * need another function to physically make the complement
     */
    if (list1->is_inverted) {
      list1->is_inverted = 0;
      return 1;
    }

    if (!evalenv) {
      Rprintf("Can't calculate complement with NULL eval env\n");
      return 0;
    }

    if (!evalenv->query_corpus) {
      Rprintf("Can't calculate complement with NULL query_corpus.\n");
      return 0;
    }

    if (!access_corpus(evalenv->query_corpus)) {
      Rprintf("Complement: can't access current corpus.\n");
      return 0;
    }

    /* OK. The tests went by. Now, the size of the new ML is the size of the corpus MINUS the size of the current matchlist. */
    if (!(attr = cl_new_attribute(evalenv->query_corpus->corpus, CWB_DEFAULT_ATT_NAME, ATT_POS))) {
      Rprintf("Complement: can't find %s attribute of current corpus\n", CWB_DEFAULT_ATT_NAME);
      return 0;
    }

    i = cl_max_cpos(attr);
    if (cl_errno != CDA_OK) {
      Rprintf("Complement: can't get attribute size\n");
      return 0;
    }

    tmp.tabsize = i - list1->tabsize;

    if (tmp.tabsize == 0) {
      /* Best case. Result is empty. */
      cl_free(list1->start);
      cl_free(list1->end);
      cl_free(list1->target_positions);
      cl_free(list1->keyword_positions);
      list1->matches_whole_corpus = 0;
      list1->tabsize = 0;
      list1->is_inverted = 0;
    }
    else if (tmp.tabsize == i) {
      /*
       * Worst case. Result is a copy of the corpus.
       *
       * TODO: This is not true if we have -1 elements in the source list.
       */
      cl_free(list1->start);
      cl_free(list1->end);
      cl_free(list1->target_positions);
      cl_free(list1->keyword_positions);
      list1->start = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      list1->tabsize = tmp.tabsize;
      list1->matches_whole_corpus = 1;
      list1->is_inverted = 0;
      for (i = 0; i < tmp.tabsize; i++)
        list1->start[i] = i;
    }
    else {
      /*
       * in between.
       */
      tmp.start = (int *)cl_malloc(sizeof(int) * tmp.tabsize);
      tmp.end = NULL;
      tmp.target_positions = NULL;
      tmp.keyword_positions = NULL;
      tmp.matches_whole_corpus = 0;

      j = 0;                        /* index in source list */
      t = 0;                        /* index in target list */
      for (k = 0; k < i; k++) {
        if ((j >= list1->tabsize) || (k < list1->start[j]))
          tmp.start[t++] = k;
        else if (k == list1->start[j])
          j++;
        else /* (k > list1->start[j]) */
          assert("Error in Complement calculation routine" && 0);
      }
      assert(t == tmp.tabsize);

      cl_free(list1->start);
      cl_free(list1->end);
      cl_free(list1->target_positions);
      cl_free(list1->keyword_positions);
      list1->start = tmp.start;
      tmp.start = NULL;
      list1->end   = tmp.end;
      tmp.end = NULL;
      list1->tabsize = tmp.tabsize;
      list1->matches_whole_corpus = 0;
      list1->is_inverted = 0;
    }

    break;

  case Identity:
    /*
     * -------------------- IDENTITY
     * create a copy of ML2 into ML1
     */
    free_matchlist(list1);

    list1->tabsize = list2->tabsize;
    list1->matches_whole_corpus = list2->matches_whole_corpus;
    list1->is_inverted = list2->is_inverted;

    if (list2->start) {
      list1->start = (int *)cl_malloc(sizeof(int) * list2->tabsize);
      memcpy((char *)list1->start, (char *)list2->start, sizeof(int) * list2->tabsize);
    }
    if (list2->end) {
      list1->end = (int *)cl_malloc(sizeof(int) * list2->tabsize);
      memcpy((char *)list1->end, (char *)list2->end, sizeof(int) * list2->tabsize);
    }
    if (list2->target_positions) {
      list1->target_positions = (int *)cl_malloc(sizeof(int) * list2->tabsize);
      memcpy((char *)list1->target_positions,
             (char *)list2->target_positions, sizeof(int) * list2->tabsize);
    }
    if (list2->keyword_positions) {
      list1->keyword_positions = (int *)cl_malloc(sizeof(int) * list2->tabsize);
      memcpy((char *)list1->keyword_positions,
             (char *)list2->keyword_positions, sizeof(int) * list2->tabsize);
    }

    break;

  case Uniq:
    /*
     * -------------------- UNIQ
     * create a unique version of ML1, working destructively on list1
     */
    if (list1->start && (list1->tabsize > 0)) {
      ins = 0;                        /* the insertion point */

      if (list1->end)
        for (i = 0; i < list1->tabsize; i++) {
          if ( ins == 0 || (list1->start[i] != list1->start[ins-1] || list1->end[i] != list1->end[ins-1]) ) {
            /*
             * copy the data from the current position down to the insertion point.
             */
            list1->start[ins] = list1->start[i];
            list1->end[ins]   = list1->end[i];
            if (list1->target_positions)
              list1->target_positions[ins]   = list1->target_positions[i];
            if (list1->keyword_positions)
              list1->keyword_positions[ins]  = list1->keyword_positions[i];
            ins++;
          }
        }
      else
        for (i = 0; i < list1->tabsize; i++) {
          if (ins == 0 || list1->start[i] != list1->start[ins-1]) {
            /*
             * copy the data from the current position down to the insertion point.
             */
            list1->start[ins] = list1->start[i];
            if (list1->target_positions)
              list1->target_positions[ins]   = list1->target_positions[i];
            if (list1->keyword_positions)
              list1->keyword_positions[ins]  = list1->keyword_positions[i];
            ins++;
          }
        }

      if (ins != list1->tabsize) {
        /*
         * no elements were deleted from the list when ins==tabsize. So we do not have to do anything then.
         * Otherwise, the list was used destructively. Free up used space.
         */
        list1->start = (int *)cl_realloc(list1->start, sizeof(int) * ins);
        if (list1->end)
          list1->end = (int *)cl_realloc(list1->end,   sizeof(int) * ins);
        if (list1->target_positions)
          list1->target_positions = (int *)cl_realloc(list1->target_positions,   sizeof(int) * ins);
        if (list1->keyword_positions)
          list1->keyword_positions = (int *)cl_realloc(list1->keyword_positions, sizeof(int) * ins);
        list1->tabsize = ins;
        list1->matches_whole_corpus = 0;
        list1->is_inverted = 0;
      }
    }

    break;

  case Reduce:
    /*
     * -------------------- REDUCE
     */
    if ((list1->start) && (list1->tabsize > 0)) {
      /* for the sake of efficiency, we distinguish here between
       * initial matchlists and non-initial matchlists. Two almost
       * identical loops are performed, but we do the test for initial
       * mls instead of inside the loop here
       */
      ins = 0;

      if (list1->end)
        for (i = 0; i < list1->tabsize; i++) {
          if (list1->start[i] != -1) {
            /* copy the data from the current position down to the insertion point. */
            if (i != ins) {
              list1->start[ins] = list1->start[i];
              list1->end[ins]   = list1->end[i];
              if (list1->target_positions)
                list1->target_positions[ins]   = list1->target_positions[i];
              if (list1->keyword_positions)
                list1->keyword_positions[ins]  = list1->keyword_positions[i];
            }
            ins++;
          }
        }
      else
        for (i = 0; i < list1->tabsize; i++) {
          if (list1->start[i] != -1) {
            /* copy the data from the current position down to the insertion point. */
            if (i != ins)
              list1->start[ins] = list1->start[i];
            if (list1->target_positions)
              list1->target_positions[ins]   = list1->target_positions[i];
            if (list1->keyword_positions)
              list1->keyword_positions[ins]  = list1->keyword_positions[i];
            ins++;
          }
        }

      if (ins == 0) {
        /* all elements have been deleted; free memory */
        cl_free(list1->start);
        cl_free(list1->end);
        cl_free(list1->target_positions);
        cl_free(list1->keyword_positions);
        list1->tabsize = 0;
        list1->matches_whole_corpus = 0;
        list1->is_inverted = 0;
      }
      else if (ins != list1->tabsize) {
        /*
         * no elements were deleted from the list when ins==tabsize.
         * So we do not have to do anything then.
         * Otherwise, the list was used destructively. Free up used space.
         */
        list1->start = (int *)cl_realloc(list1->start, sizeof(int) * ins);
        if (list1->end)
          list1->end = (int *)cl_realloc(list1->end,   sizeof(int) * ins);
        if (list1->target_positions)
          list1->target_positions = (int *)cl_realloc(list1->target_positions, sizeof(int) * ins);
        if (list1->keyword_positions)
          list1->keyword_positions = (int *)cl_realloc(list1->keyword_positions, sizeof(int) * ins);
        list1->tabsize = ins;
        list1->matches_whole_corpus = 0;
        list1->is_inverted = 0;
      }
    }

    break;

  default:
    assert("Illegal operator in apply_setop_to_matchlist" && 0);
    return 0;
  }

  return 1;
}
