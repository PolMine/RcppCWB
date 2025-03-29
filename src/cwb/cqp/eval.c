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
#include <string.h>
#include <limits.h>
#include <math.h>

#include "../cl/cl.h"
#include "../cl/cwb-globals.h"
#include "../cl/ui-helpers.h"

#include "cqp.h"
#include "ranges.h"
#include "options.h"
#include "tree.h"
#include "symtab.h"
#include "corpmanag.h"
#include "regex2dfa.h"
#include "eval.h"
#include "builtins.h"
#include "output.h"
#include "matchlist.h"

/* this macro is used to hide chunks of the code previously avoided by an "if (0)".
 * See comment by Oli in func match_first_pattern.
 * See also section on "Easter eggs" in the tutorial. */
#define COMPILE_QUERY_OPTIMIZATION 0


#define no_match -1

#if COMPILE_QUERY_OPTIMIZATION
/** Reduction threshold */
#define RED_THRESHOLD 0.01
#endif


/*
 * GLOBAL VARIABLES FOR EVAL
 */


/** A global array of EvalEnvironment structures. Each represents the environment for a different aligned corpus. */
EvalEnvironment Environment[MAXENVIRONMENT];

/**
 * Global eval environment index; contains the index of the
 * highest currently-occupied slot within Environment.
 * @see Environment
 */
int ee_ix;

/** Global pointer, address of the "highest" cell in Environment. Used largely in parse_actions.  @see Environment  */
EEP CurEnv;

/** Global pointer, address of the cell in Environment that is being used for evaluation. Used largely in files other than parse_actions. @see Environment. */
EEP evalenv;

/* desc above of CurEnv and evalenv is still a bit vague */


/**
 * To aid in building error messages: get const
 * string with a printable name for an operation.
 */
static const char *
get_b_operator_name(enum b_ops op)
{
  switch (op) {
  case b_and:     return "and";
  case b_or:      return "or";
  case b_implies: return "implies";
  case b_not:     return "not";
  case cmp_gt:    return "greater than";
  case cmp_lt:    return "less than";
  case cmp_get:   return "greater than or equals";
  case cmp_let:   return "less than or equals";
  case cmp_eq:    return "equals";
  case cmp_neq:   return "not equals";
  case cmp_ex:    return "";
  }
  return "UNKNOWN";
}



#if COMPILE_QUERY_OPTIMIZATION
/**
 * Counts the number of token positions encompassed by all members
 * of the ->range array of the CorpusList argument.
 *
 * That is, in oher words, it tells you the size of this corpus
 * (or subcorpus).
 *
 * (Only currently used in one place, which is switched off - the query optimiser;
 * cf query_optimise in options.h)
 */
static int
nr_positions(CorpusList *cp)
{
  int nr_pos = -1, i;
  assert(cp);

  if (0 <= cp->size)
    for (nr_pos = 0, i = 0; i < cp->size; i++)
      nr_pos += cp->range[i].end - cp->range[i].start + 1;

  return nr_pos;
}
#endif

#if COMPILE_QUERY_OPTIMIZATION
/**
 * Calculates the reduction factor of a subcorpus/NQR: that is, the proportion of the mother
 * that it contains. It returns an arbitrary negative value for error.
 *
 * (Only currently used in one place, which is switched off - the query optimiser;
 * cf query_optimise in options.h)
 *
 * @param cp       A CorpusList object for a Subcorpus or query result.
 * @param nr_pos   Out param: size of the subcorpus, in tokens.
 * @return         Float: ratio of size of the Subcorpus to its mother.
 *                 Negative value for error.
 */
static float
red_factor(CorpusList *cp, int *nr_pos)
{
  Attribute *attr;
  assert(cp);

  if (!access_corpus(cp))
    return -1.0;

  if (!(attr = cl_new_attribute(cp->corpus, CWB_DEFAULT_ATT_NAME, ATT_POS)))
    return -1.0;

  if (0 > cp->mother_size)
    return -1.0;

  if (0 > (*nr_pos = nr_positions(cp)))
    return -1.0;

  return (*nr_pos + 0.0) / (cp->mother_size + 0.0);
}
#endif

/**
 * Set the appropriate values to the corpus id (given by its pointer to
 * the symbol table).
TODO what on earth does the above mean? seems not to relate to this function at all
 *
 * Assigns the range data from a matchlist object (from running a query) to a CorpusList
 * object that will represent the result as a NQR/subcorpus.
 *
 * This includes the keyword/target data if present.
 *
 * @param cp                   (should be cl) The CorpusList object for the query result we are
 *                             creating. In subquery filter mode (see below) it needs to contain the
 *                             range data for the activated NQR being filtered.
 * @param matchlist            Matchlist object array containing the actual query matches.
 * @param nr_lists             Has no effect on anything, but it will cause an error if it's greater than 1.
 * @param keep_old_ranges      Boolean. A somewhat opaquely named parameter which switches on/off
 *                             "subquery filter mode". In SQ filter mode, the query result is a subset
 *                             of the ranges in the activated query result (those which contain a
 *                             match to the subquery). In non-filter mode, the query result is the
 *                             expected set of match begin/end ranges. This implements the Keep
 *                             operator (confusingly, an exclamation mark).
 */
static void
set_corpus_matchlists(CorpusList *cp,
                      Matchlist *matchlist,
                      int nr_lists,
                      int keep_old_ranges)
{
  int i;
  int rp = 0, mp = 0; /* indexes into the range array in cp, and into the start element of the matchlist array. */

  if (keep_old_ranges) {

    /* subquery filter mode: delete the ranges which didn't lead to a match */

    /* TODO: Interaction with target @ settings */

    cl_free(cp->sortidx);

    /* keep keywords, ranges, targets from the activated query */

    while (rp < cp->size) {
      if (mp < matchlist[0].tabsize) {
        /* there is a matchlist element that might whitelist the current range; is it possibly within the current range? */
        if (matchlist[0].start[mp] >= cp->range[rp].start) {
          /* this match  MIGHT start within the current range! */
          if (matchlist[0].start[mp] > cp->range[rp].end || matchlist[0].end[mp] > cp->range[rp].end)
            /* the match is not wholly within the current range : we can safely mark this range deleted and move on. */
            cp->range[rp++].start = -1;
          else
            /* this match lies within the current range, so don't mark it for deletion; move on to next range. */
            rp++;
        }
        else
          /* match.start < range.start ; this match can't whitelist the current range, as it's not within it; try the next */
          mp++;
      }
      else
        /* mp >= matchlist[0].tabsize ; we can't find a matchlist element, so mark the range as deleted */
        cp->range[rp++].start = -1;
    }
    apply_range_set_operation(cp, RReduce, NULL, NULL);
  }
  else {

    /* DON'T use filter mode: rather than copying range data from the activated query to the new CorpusList object,
       make new range data according to the matches. */

    cl_free(cp->range);
    cl_free(cp->targets);
    cl_free(cp->keywords);
    cl_free(cp->sortidx);

    if (matchlist[0].tabsize > 0)
      cp->range = (Range *)cl_malloc(sizeof(Range) * matchlist[0].tabsize);
    cp->size = matchlist[0].tabsize;

    /* copy matches to the query result's CorpusList entry. */
    for (i = 0; i < matchlist[0].tabsize; i++) {
      cp->range[i].start = matchlist[0].start[i];
      cp->range[i].end   = matchlist[0].end[i];
    }

    /* if present, the target positions and keyword positions are moved from the matchlist to the query result. */
    if (matchlist[0].target_positions) {
      cp->targets = matchlist[0].target_positions;
      matchlist[0].target_positions = NULL;
    }
    if (matchlist[0].keyword_positions) {
      cp->keywords = matchlist[0].keyword_positions;
      matchlist[0].keyword_positions = NULL;
    }

    assert((nr_lists <= 1) && "set_corpus_matchlists(): multiple lists not supported!");
  }
}


/**
 * Gets a list of corpus positions where the given p-attribute has
 * the specified form (looked up directly in the attribute's lexicon).
 *
 * This is done by wrapping cl_str2id() and cl_idlist2cpos().
 * Positions are placed into the "start" array of the matchlist.
 *
 * Function is not NULL safe, the attribute and matchlist must be non-NULL.
 * Similarly, the matchlist object must be empty; i.e. it cannot have memory
 * allocated to its "start" array.
 *
 * @param attribute  The p-attribute to search.
 * @param wordform   The form to search for. (Should really be value_sought or similar.)
 * @param matchlist  Where to put the results.
 * @return           The size of the resulting matchlist table
 *                   (also stored in its tabsize member).
 */
static int
get_corpus_positions(Attribute *attribute, char *wordform, Matchlist *matchlist)
{
  int word_id;

  assert(attribute);
  assert(matchlist);
  assert(matchlist->start == NULL);

  word_id = cl_str2id(attribute, wordform);

  if (word_id >= 0 && cl_all_ok()) {
    /* get the positions of the id in the attribute */
    matchlist->start = cl_idlist2cpos(attribute,
                                      &word_id,
                                      1,
                                      1,
                                      &(matchlist->tabsize));
    matchlist->matches_whole_corpus = 0;
  }

  if (initial_matchlist_debug && matchlist->start != NULL && matchlist->tabsize > 0 && !silent)
    Rprintf("matched initial wordform for non-regex %s, " "%d matches\n", wordform, matchlist->tabsize);

  return matchlist->tabsize;
}


/**
 * Get corpus positions matching a regular expression on a given p-attribute.
 *
 * get_matched_corpus_positions looks in a corpus which is to be loaded for
 * a regular expression 'regstr' of a given p-attribute and returns the table
 * of matching start indices (start_table) and the tablesize (tabsize).
 *
 * @param attribute        The attribute to search on. May be NULL, in
 *                         which case DEFAULT_ATT_NAME is used.
 * @param regstr           String containing the regular expression.
 * @param regex_flags      Flags to be passed to the CL regex engine.
 * @param matchlist        Location where the list of matches will be placed.
 * @param restrictor_list  Passed to cl_idlist2cpos_oldstyle
 * @param restrictor_size  Passed to cl_idlist2cpos_oldstyle
 * @return                 The number of matches found.
 */
static int
get_matched_corpus_positions(Attribute *attribute,
                             char *regstr,
                             int regex_flags,
                             Matchlist *matchlist,
                             int *restrictor_list,
                             int restrictor_size)
{
  int *matched_form_ids, n_forms_matched, i, size, range;

  assert(matchlist);
  assert(matchlist->start == NULL);

  matchlist->is_inverted = 0;

  if (attribute == NULL)
    attribute = cl_new_attribute(evalenv->query_corpus->corpus, CWB_DEFAULT_ATT_NAME, ATT_POS);
  assert(attribute);

  size  = cl_max_cpos(attribute); /* n tokens on attribute */
  range = cl_max_id(attribute);   /* n types on attribute */

  /* changed .* / .+ optimization to .* only -- so "" will be handled correctly as an attribute value
     (will be standard in CWB 4.0, and may happen now if someone runs encode with -U "") */
  /* AH notes Aug 2019: the above comment about 4.0 has nothing to do with our present plans! */
  if (cl_str_is(regstr, ".*")) {
    if (eval_debug)
      Rprintf("get_matched_corpus_positions: .* optimization\n");

    matchlist->start = (int *)cl_malloc(sizeof(int) * size);

    /* we here produce a copy of a system corpus. TODO: optimize that with the "matches_whole_corpus"-flag. */
    for (i = 0; i < size; i++)
      matchlist->start[i] = i;
    matchlist->tabsize = size;
    matchlist->matches_whole_corpus = 1;
  }
  else {
    /* get the word ids of the forms which are matched by the regular expression  'regstr' */
    matched_form_ids = cl_regex2id(attribute,
                                   regstr,
                                   regex_flags,
                                   &n_forms_matched);

    if (n_forms_matched == range) {
      /* again, matches whole corpus. */
      cl_free(matched_form_ids);

      matchlist->start = (int *)cl_malloc(sizeof(int) * size);

      /* we here produce a copy of a system corpus. TODO: optimize that with the "matches_whole_corpus"-flag. */
      for (i = 0; i < size; i++)
        matchlist->start[i] = i;
      matchlist->tabsize = size;
      matchlist->matches_whole_corpus = 1;

    }
    else if ((matched_form_ids != NULL) && (n_forms_matched > 0)) {
      /* Some matching forms have been found, so get the position numbers in the active corpus of the word ids */
      matchlist->start = cl_idlist2cpos_oldstyle(attribute,
                                                 matched_form_ids,
                                                 n_forms_matched,
                                                 1,
                                                 &(matchlist->tabsize),
                                                 restrictor_list,
                                                 restrictor_size);
      cl_free(matched_form_ids);
    }
    else {
      /* no matching forms have been found. */
      matchlist->tabsize = 0;
      matchlist->matches_whole_corpus = 0;
    }
  } /* end case where regstr is not ".*" */

  /* finally, possibly print out a debug message */
  if (initial_matchlist_debug && matchlist->start != NULL && matchlist->tabsize > 0 && !silent)
    Rprintf("matched initial pattern for regex %s, %d matches\n", regstr, matchlist->tabsize);

  return matchlist->tabsize;
}

/*
 * helper function: locate first element in Ranges vector with specified start position by binary search,
 * returning the index of this element in the vector or -1 if exact start position does not occur
 */
static int
_find_range(Range *vec, int size, int start) {
   int lower = 0, upper = size - 1, mid;
   if (size < 1)
     return -1; /* can't find start if vector is empty */
   while (lower < upper) {
     mid = (lower + upper) / 2;
     if (vec[mid].start < start)
       lower = mid + 1; /* mid always < size - 1, so new lower remains valid */
     else
       upper = mid; /* mid is always < upper */
   }
   return (vec[lower].start == start) ? lower : -1;
}

/*
 * helper function for Region element: add range to StateQueue
 *  - sets labels and target/keyword anchors in the queued state (if applicable)
 *  - prints debug output depending on system settings
 */
static void
_add_to_queue(AVS avs, int start, int end, RefTab labels) {
  assert(avs->type == Region);
  RefTab rt = StateQueue_push(avs->region.queue, end, labels);

  if (avs->region.start_label)
    set_reftab(rt, avs->region.start_label->ref, start);
  if (avs->region.start_target == IsTarget)
    set_reftab(rt, evalenv->target_label->ref, start);
  else if (avs->region.start_target == IsKeyword)
    set_reftab(rt, evalenv->keyword_label->ref, start);

  if (avs->region.end_label)
    set_reftab(rt, avs->region.end_label->ref, end);
  if (avs->region.end_target == IsTarget)
    set_reftab(rt, evalenv->target_label->ref, end);
  else if (avs->region.end_target == IsKeyword)
    set_reftab(rt, evalenv->keyword_label->ref, end);

  if (simulate_debug) {
    Rprintf("<<%s>> ENTER added %d to queue (%d entries)\n",
            avs->region.name, end, StateQueue_length(avs->region.queue));
    if (symtab_debug)
      print_label_values(evalenv->labels, rt, start);
  }
}


/*
 * This function evaluates an AVS, i.e. an item in a finite-state query, to true or false.
 * The AVS typically represents a token expression with a Boolean constraint, but can also
 * be an XML tag, an anchor tag, or in a recent extension a Region element.
 *
 * labelrefs is the reference table for the current state (needed to set labels/targets)
 * target_labelrefs is the reference table of the target state; if the constraint is fulfilled,
 * eval_constraint() has to copy labelrefs to target_labelrefs and set labels there.
 * For the HOLD and EMIT phase of a Region element, the cpos of the FSA simulation state
 * may need to be updated by writing to *out_cpos. Note that all Region phases need to be
 * treated as zero-width elements in order for the cpos logic to be correct.
 * For other constraints, *out_cpos is unmodified (and may be set to -1 beforehand as a guard).
 */
static Boolean
eval_constraint(AVS avs, int corppos, RefTab labelrefs, RefTab target_labelrefs, int *out_cpos)
{
  int start, end, struc, anchor, n, zero_width;
  anchor = -1;
  Boolean result = False;
  CorpusList *corpus;

  switch (avs->type) {
  case Tag:
    struc = cl_cpos2struc(avs->tag.attr, corppos);
    if (struc < 0)
      return False;
    cl_struc2cpos(avs->tag.attr, struc, &start, &end);

    /* opening tag */
    if (!avs->tag.is_closing) {
      result = (corppos == start) ;
      /* evaluate optional constraint only when at start of region */
      if (result && avs->tag.constraint) {
        char *val = cl_struc2str(avs->tag.attr, struc);
        if (val) {
          if (avs->tag.rx)
            result = cl_regex_match(avs->tag.rx, val, 0); /* pre-compiled regex available */
          else
            result = cl_streq(avs->tag.constraint, val); /* no pre-compiled regex -> match as plain string */
        }
        else
          result = False;
        if (avs->tag.negated)
          result = !result;
      }
    }
    /* closing tag */
    else {
      result = (corppos == end);
      if (strict_regions && avs->tag.right_boundary != NULL) {
        int rbound = get_reftab(labelrefs, avs->tag.right_boundary->ref, -1);
        if ((rbound < 0) || (corppos != rbound))   /* check that a within region constraint was set by an open tag, and that this is the matching close tag! */
          result = False;                          /* (so that <s> []* </s> will work and <s></s> won't match, plus some more exotic cases) */
      }
    }

    if (result) {
      dup_reftab(labelrefs, target_labelrefs);
      /* in StrictRegions mode, set corresponding label (open tag) or clear it (close tag) */
      if (strict_regions && avs->tag.right_boundary != NULL) {
        if (!avs->tag.is_closing )               /* open tag */
          set_reftab(target_labelrefs, avs->tag.right_boundary->ref, end);
        else                                     /* close tag */
          set_reftab(target_labelrefs, avs->tag.right_boundary->ref, -1);
      }
    }
    return result;

  case Anchor:
    corpus = evalenv->query_corpus;
    switch (avs->anchor.field) {
    case MatchField:
      anchor = corpus->range[evalenv->rp].start;
      break;
    case MatchEndField:
      anchor = corpus->range[evalenv->rp].end;
      break;
    case TargetField:
      assert("Internal error in eval_constraint()" && corpus->targets != NULL);
      anchor = corpus->targets[evalenv->rp];
      break;
    case KeywordField:
      assert("Internal error in eval_constraint()" && corpus->keywords != NULL);
      anchor = corpus->keywords[evalenv->rp];
      break;
    default:
      assert("Internal error in eval_constraint(): no handler for anchor." && 0);
    }
    /* we don't have to worry about whether it's an opening or closing anchor tag,
     * because corppos already is the _effective_ cpos (passed from simulate()) */
    result = (anchor >= 0 && anchor == corppos) ;

    /* don't forget to copy the reftab to the target state */
    if (result)
      dup_reftab(labelrefs, target_labelrefs);
    return result;

  case Pattern:
    /* used to reset the list of labels (kill forward references) here */
    /* Ah great, so labels can only be set by the _last_ active candidate,
       because it will erase all the labels set by earlier candidates for
       this point. However, if we keep labels pointing to the current
       cpos, we should be OK (assuming that all states progress in parallel
       along the corpus). This should enable us to get optional labels at
       least. If several candidates pass through the same labelled pattern
       at different times, though, that label will be re-set whenever
       a candidate arrives; this is simply because CQP doesn't have a full
       labels implementation. Rats.
       */
    /* if (evalenv->labels)
         reset_labellist(evalenv->labels, corppos+1); */
    /* we're resetting from corppos+1 because another active state may just
       have set a label to the current corppos! */
    /* Got some rat poison now, i.e. a new labels implementation !!! */

    /* now evaluate the pattern */
    result = eval_bool(avs->con.constraint, labelrefs, corppos);

    /* if the current constraint is labelled, set the
       corresponding label value; but _only_ if result is true,
       i.e. if the automaton actually reaches that state!
     */

    if (result) {
      dup_reftab(labelrefs, target_labelrefs); /* copy labels to target state & set label there*/
      if (avs->con.label != NULL)
        set_reftab(target_labelrefs, avs->con.label->ref, corppos);
    }

    /* return the evaluation of the bool tree of the constraint */
    return result;

  case MatchAll:
    /* if (evalenv->labels)            !!! RAT POISON !!!
       reset_labellist(evalenv->labels, corppos+1);     */
    dup_reftab(labelrefs, target_labelrefs);
    if (avs->matchall.label != NULL)
      set_reftab(target_labelrefs, avs->matchall.label->ref, corppos);
    return True;

  case Region:
    /* Region elements are the most complex case. They are implemented by three transitions
     * (the ENTER, WAIT and EMIT operations) linked to a shared queue of waiting FSA simulation states.
     * The corresponding FSA fragment is:  ENTER ( WAIT )* EMIT
     * See "Finite State Queries in CWB3", Sec. 2.5 for a general description of the algorithm used.
     *
     * Because the logic for updating the corpus position of the simulation state is fairly complex,
     * the update is passed back to the caller by setting *out_cpos. All three transitions must be
     * treated as zero-width patterns, i.e. they do not auto-advance the cpos and set matchend to cpos-1
     * if they are the last transition of a match (which can only happen for EMIT, of course).
     *
     * ENTER operation:
     *  - check whether effective_cpos is the start of a region of the s-attribute or match span of the NQR
     *  - if successful, add the state to the wait queue with cpos set to the last token of the region or span
     *  - set start/end labels and/or target markers in the RefTab of the state in the queue
     *  - take the transition, which is considered to be zero-width by the simulate() function
     *  - for a zero-width region element without wait queue, the ENTER operation only tests whether
     *    effective_cpos is the start of a suitable region or span, take the transition (zero-width)
     *    if successful, and optionally sets start label and anchor directly in target_labelref;
     *    there are no WAIT and EMIT transitions and the FSA continues directly to the next query element
     *
     * WAIT operation:
     *  - if the queue is not empty, take the loop and increment cpos (via *out_cpos because the transition is zero-width)
     *  - exception: don't increment cpos if the next _two_ states waiting in the queue have cpos <= corppos
     *  - NB: the EMIT transition is always tried after WAIT, hence it will emit the first of the two states at this time step
     *
     * EMIT operation:
     *  - if the queue is not empty and the first item has cpos <= corppos, emit a single item
     *  - to do so, copy the item's RefTab to the target state RefTab, and return its cpos + 1 via *out_cpos
     */

    switch (avs->region.opcode) {
    case AVSRegionEnter:
      zero_width = avs->region.queue ? 0 : 1; /* absence of queue indicates a zero-width region element */
      if (avs->region.nqr) {
        n = avs->region.nqr->size;
        struc = _find_range(avs->region.nqr->range, n, corppos); /* also -1 if n == 0 */
        if (struc < 0)
          return False; /* not at start of a match range */
        if (!zero_width) {
          /* add all spans starting at corppos to queue (unless zero-width) */
          while (struc < n && avs->region.nqr->range[struc].start == corppos) {
            end = avs->region.nqr->range[struc].end;
            _add_to_queue(avs, corppos, end, labelrefs);
            struc++;
          }
        }
      }
      else if (avs->region.attr) {
        struc = cl_cpos2struc(avs->region.attr, corppos);
        if (struc < 0)
          return False;
        cl_struc2cpos(avs->region.attr, struc, &start, &end);
        if (corppos != start)
          return False; /* not at start of region */
        if (!zero_width)
          _add_to_queue(avs, corppos, end, labelrefs);
      }
      else
        assert(0 && "Internal error: <<region>> element has neither s-attribute nor NQR set.");
      dup_reftab(labelrefs, target_labelrefs); /* preserve labels of the waiting state, to apply strict region checks */
      if (zero_width) {
        /* zero-width region element has to set labels here because there is no queue and no EMIT transition */
        if (avs->region.start_label)
          set_reftab(target_labelrefs, avs->region.start_label->ref, corppos);
        if (avs->region.start_target == IsTarget)
          set_reftab(target_labelrefs, evalenv->target_label->ref, corppos);
        else if (avs->region.start_target == IsKeyword)
          set_reftab(target_labelrefs, evalenv->keyword_label->ref, corppos);
      }
      return True;

    case AVSRegionWait:
      assert(avs->region.queue);
      n = StateQueue_length(avs->region.queue);
      if (n >= 2 && avs->region.queue->head->next->cpos <= corppos)
        *out_cpos = corppos; /* hold back wait state because further items need to be emitted at this cpos */
      else
        *out_cpos = corppos + 1; /* advance cpos of wait state */
      if (n <= 0)
        return False;
      if (simulate_debug)
        Rprintf("<<%s>> WAIT (%d entries in queue, next is cpos=%d)\n", avs->region.name, StateQueue_length(avs->region.queue), StateQueue_next_cpos(avs->region.queue));
      dup_reftab(labelrefs, target_labelrefs); /* preserve labels of the waiting state, to apply strict region checks */
      return True; /* take loop while there are still waiting states */

    case AVSRegionEmit:
      assert(avs->region.queue);
      end = StateQueue_next_cpos(avs->region.queue);
      if (end < 0 || end > corppos)
        return False; /* queue is empty or next state not ready to emit */
      end = StateQueue_pop(avs->region.queue, target_labelrefs); /* emit first state from queue and copy its label references to target state */
      *out_cpos = end + 1; /* advance to token after end of region in next simulation step */
      if (simulate_debug) {
        Rprintf("<<%s>> EMIT to cpos=%d from queue (%d entries remain)\n", avs->region.name, end + 1, StateQueue_length(avs->region.queue));
        /* symtab will be printed for target state anyway */
      }
      return True;
    }
    /* any unhandled opcodes or AVS types fall through to "not reached" below */
  }
  assert(0 && "Not reached");
  return False;
}




/* get the corpus position referenced by a label wrt. to the reference table rt */
/* (corppos used to be required for anchor labels, which are now removed, but it'll be useful for the 'this' label) */
static int
get_label_referenced_position(LabelEntry label, RefTab rt, int corppos)
{
  int referenced_position = -1;

  if (label) {
    referenced_position = get_reftab(rt, label->ref, corppos);
    if (eval_debug)
      Rprintf("Evaluating label %s = %d\n", label->name, referenced_position);
  }

  return referenced_position;
}



static Boolean
get_leaf_value(Constrainttree ctptr,
               RefTab rt, /* label reference table of the current simulation */
               int corppos,
               DynCallResult *dcr,
               int deliver_strings)
{
  int pos, rv;
  DynCallResult *fargs;
  ActualParamList *p;
  Boolean params_ok;

  int struc_start, struc_end;

  assert(ctptr);

  dcr->type = ATTAT_NONE;

  switch (ctptr->type) {

  case func:
    if (ctptr->func.nr_args > 0) {

      fargs = (DynCallResult *)cl_malloc(sizeof(DynCallResult) * ctptr->func.nr_args);
      if (fargs == NULL)
        return False;

      pos = 0;
      params_ok = True;
      for (p = ctptr->func.args ;
            p != NULL &&
            pos < ctptr->func.nr_args &&
            params_ok ;
           p = p->next, pos++)
        params_ok = get_leaf_value(p->param, rt, corppos, &(fargs[pos]), 1);

      /* we don't want to crash when one of the args can't be computed!) */
      assert(!params_ok || pos == ctptr->func.nr_args);

      if (params_ok) {
        if (ctptr->func.predef >= 0) {
          rv = call_predefined_function(ctptr->func.predef,
                                        fargs,
                                        pos,
                                        ctptr,
                                        dcr);
          cl_free(fargs);
          /* if the evaluation of a builtin function fails, this is usually due to a usage error;
             -> abort query evaluation to avoid hundreds or thousands of error messages (simulate Ctrl-C signal) */
          if (!rv)
            EvaluationIsRunning = 0;
          return rv;
        }
        else {
          assert(ctptr->func.dynattr);

          pos = cl_dynamic_call(ctptr->func.dynattr, dcr, fargs, ctptr->func.nr_args);
          cl_free(fargs);

          return (pos == 1 && cl_all_ok());
        }
      }
      else {
        /* parameter cannot be calculated */
        cl_free(fargs);
        return False;
      }
    }
    else
      return False;
    /* not reached: all paths return */
    break;

  case pa_ref:
    if (ctptr->pa_ref.attr == NULL) {

      /* label reference without an attribute -> returns referenced corpus position */

      assert(ctptr->pa_ref.label);

      dcr->type = ATTAT_POS;
      dcr->value.intres = get_label_referenced_position(ctptr->pa_ref.label, rt, corppos);
      if (ctptr->pa_ref.del) {
        if (eval_debug)
          Rprintf("** AUTO-DELETING LABEL %s = %d\n",
                 ctptr->pa_ref.label->name, dcr->value.intres);
        set_reftab(rt, ctptr->pa_ref.label->ref, -1);
      }
      return True;
    }
    else {
      int referenced_position;

      if (ctptr->pa_ref.label == NULL)
        referenced_position = corppos;
      else {
        referenced_position = get_label_referenced_position(ctptr->pa_ref.label, rt, corppos);
        if (ctptr->pa_ref.del) {
          if (eval_debug)
            Rprintf("** AUTO-DELETING LABEL %s = %d\n", ctptr->pa_ref.label->name, referenced_position);
          set_reftab(rt, ctptr->pa_ref.label->ref, -1);
        }

        if (referenced_position < 0) {
          dcr->type = ATTAT_NONE;
          return True;
        }
      }

      if (deliver_strings) {
        dcr->type = ATTAT_STRING;
        dcr->value.charres = cl_cpos2str(ctptr->pa_ref.attr, referenced_position);
      }
      else {
        dcr->type = ATTAT_PAREF;
        dcr->value.parefres.attr = ctptr->pa_ref.attr;

        assert(dcr->value.parefres.attr);

        dcr->value.parefres.token_id = cl_cpos2id(ctptr->pa_ref.attr, referenced_position);
      }
      return cl_all_ok();
    }
    /* not reached: both paths return */
    break;

  case sa_ref:
    if (ctptr->sa_ref.label == NULL) {
      /* bare reference to S-attribute -> old behaviour */
      dcr->type = ATTAT_INT;
      dcr->value.intres = 0;
      if (cl_cpos2struc2cpos(ctptr->sa_ref.attr, corppos, &struc_start, &struc_end)) {
        if (corppos  == struc_start)
          dcr->value.intres += 1;

        if (corppos == struc_end)
          dcr->value.intres += 2;

        /* new: make sure the value of an saref (without values) is
           non-zero (i.e. evaluates to True) as long as we are within a region */
        if ((corppos >= struc_start) && (corppos <= struc_end))
          dcr->value.intres += 4;
        /* not really necessary, since get_struc_attribute() returns 0 if we're not inside a region */
      }
      else if (cl_errno != CDA_ESTRUC) /* get_struc_attribtue() sets cl_errno=CDA_EDSTRUC if not in region */
        return False;                /* this _is_ an error */

      return True;
    }
    else {
      /* label reference to S-attribute -> return value of containing region */
      int referenced_position = get_label_referenced_position(ctptr->sa_ref.label, rt, corppos);
      if (ctptr->sa_ref.del) {
        if (eval_debug)
          Rprintf("** AUTO-DELETING LABEL %s = %d\n", ctptr->sa_ref.label->name, referenced_position);
        set_reftab(rt, ctptr->sa_ref.label->ref, -1);
      }

      if (referenced_position < 0) {
        dcr->type = ATTAT_NONE;        /* don't know what it does; copied from pa_ref */
        return True;
      }

      dcr->type = ATTAT_STRING;
      dcr->value.charres = cl_cpos2struc2str(ctptr->sa_ref.attr, referenced_position);

      /* structure_value_at_position() sets CDA_EPOSORNG if not in region */
      if (cl_errno == CDA_EPOSORNG) {
        dcr->type = ATTAT_NONE;        /* reasonable behaviour: return ATTAT_NONE if not in region */
        return True;
      }

      return cl_all_ok();
    }
    /* not reached: both paths return */
    break;

  case string_leaf:
    if (ctptr->leaf.pat_type == CID) {
      /*
       * doing CID as a type of string ref is a quick & dirty hack . We should have a cidref node
       */
      dcr->type = ATTAT_INT;
      dcr->value.intres = ctptr->leaf.ctype.cidconst;
    }
    else {
      dcr->type = ATTAT_STRING;
      dcr->value.charres = ctptr->leaf.ctype.sconst;
    }
    return True;

  case float_leaf:
    dcr->type = ATTAT_FLOAT;
    dcr->value.floatres = ctptr->leaf.ctype.fconst;
    return True;
    /* formerly there was a bug here, break instead of return, which went straight to the "not reached" assertion.
    / Didn't cuase a problem only becasue float values are so rarely used by actual users. */

  case int_leaf:
    dcr->type = ATTAT_INT;
    dcr->value.intres = ctptr->leaf.ctype.iconst;
    return True;

  default:
    cqpmessage(Error, "get_leaf_value(): Illegal node type %d\n", ctptr->type);
    EvaluationIsRunning = 0;
    return False;
  }
  assert(0 && "Not reached");
  return False;
}



/** Comparison function used when eval_bool() calls qsort(). */
static int
intcompare(const void *i, const void *j)
{
  return *(int *)i - *(int *)j;
}


/**
 * Evaluate a boolean constraint tree/node using recursion.
 *
 * @param ctptr         The constraint tree pointer.
 * @param rt            The reference table object.
 * @param curr_cpos     The current corpus position.
 */
Boolean
eval_bool(Constrainttree ctptr, RefTab rt, int curr_cpos)
{
  DynCallResult lhs, rhs;
  int start, end, id;
  int referenced_corppos;

  if (ctptr)
    if (ctptr->type == bnode) {
      switch(ctptr->node.op_id) {

        /* logical and */
      case b_and:
        if (eval_debug)
          Rprintf("eval_bool: evaluate boolean and\n");
        assert(ctptr->node.left && ctptr->node.right);
        return(eval_bool(ctptr->node.left, rt, curr_cpos) && eval_bool(ctptr->node.right, rt, curr_cpos));

        /* logical or */
      case b_or:
        if (eval_debug)
          Rprintf("eval_bool: evaluate boolean or\n");
        assert(ctptr->node.left && ctptr->node.right);
        return eval_bool(ctptr->node.left, rt, curr_cpos) || eval_bool(ctptr->node.right, rt, curr_cpos);

        /* logical implication */
      case b_implies:
        if (eval_debug)
          Rprintf("eval_bool: evaluate boolean implication\n");
        assert(ctptr->node.left && ctptr->node.right);
        return eval_bool(ctptr->node.left, rt, curr_cpos) ? eval_bool(ctptr->node.right, rt, curr_cpos) : True;

        /* logical not */
      case b_not:
        if (eval_debug)
          Rprintf("eval_bool: evaluate boolean not\n");
        if (!ctptr->node.left)
          return True;
        assert(ctptr->node.right == NULL);
        return !eval_bool(ctptr->node.left, rt, curr_cpos);

        /* relational operators */
      case cmp_gt:
      case cmp_lt:
      case cmp_get:
      case cmp_let:
      case cmp_eq:
      case cmp_neq:
      case cmp_ex:
        if (eval_debug)
          Rprintf("eval_bool: evaluate comparisons\n");

        /* check presence of arguments */
        assert(ctptr->node.left &&  (ctptr->node.op_id == cmp_ex || ctptr->node.right) );

        /*
         * LHS:
         *  -- Attribute reference (type String)
         *  -- Function call       (type String or Integer)
         *  -- Qualified Label reference (type String)
         */

        switch (ctptr->node.left->type) {
        case func:
        case pa_ref:
        case sa_ref:                /* label reference to S-attribute with values */
          if (!get_leaf_value(ctptr->node.left, rt, curr_cpos, &lhs, 0))
            return False;
          break;

        default:
          cqpmessage(Error, "Illegal type (%d) of LHS argument in pattern.", ctptr->node.left->type);
          return EvaluationIsRunning = False;
        }

        /* if we only check for existence of a value, the RHS is neglected */
        if (ctptr->node.op_id == cmp_ex)
          switch (lhs.type) {
          case ATTAT_STRING:
            return (lhs.value.charres != NULL);

          case ATTAT_PAREF:
            return (lhs.value.parefres.token_id >= 0 ? 1 : 0);

          case ATTAT_FLOAT:
            return (lhs.value.floatres == 0.0 ? 0 : 1);

          case ATTAT_INT:
            return (lhs.value.intres == 0) ? 0 : 1;

          case ATTAT_POS:        /* returned by bare label references */
            return (lhs.value.intres >= 0) ? 1 : 0;

          case ATTAT_NONE:       /* undefined value, always evaluates to False */
            return False;

          default:
            cqpmessage(Error, "Illegal type (%d) of existence expression.", lhs.type);
            return EvaluationIsRunning = False;
          }

        /* otherwise, we have to compute & check the RHS */

        assert(ctptr->node.right != NULL);

        switch(ctptr->node.right->type) {
        case func:
        case pa_ref:
        case sa_ref:
        case int_leaf:
        case float_leaf:
        case string_leaf:
          if (!get_leaf_value(ctptr->node.right, rt, curr_cpos, &rhs, 0))
            return False;
          break;

        default:
          cqpmessage(Error, "Illegal type (%d) of RHS argument in pattern.", ctptr->node.right->type);
          return EvaluationIsRunning = False;
        }


        /* OK. We now evaluate the relational operator */

        /* make it easier in case of ints */
        if (lhs.type == ATTAT_POS)
          lhs.type = ATTAT_INT;
        if (rhs.type == ATTAT_POS)
          rhs.type = ATTAT_INT;

        if (lhs.type == ATTAT_NONE || rhs.type == ATTAT_NONE)
          /* cannot evaluate */
          return False;

        if (lhs.type == ATTAT_PAREF && rhs.type == ATTAT_INT) {
          if (ctptr->node.op_id == cmp_eq)
             return (lhs.value.parefres.token_id == rhs.value.intres ? 1 : 0);
          else if (ctptr->node.op_id == cmp_neq)
             return (lhs.value.parefres.token_id != rhs.value.intres ? 1 : 0);
          else {
            cqpmessage(Error, "Comparisons (>, >=, <, <=) are not allowed with p-attributes; only (=, !=) are allowed.");
            return EvaluationIsRunning = False;
          }
        }
        else if (lhs.type == ATTAT_PAREF && rhs.type == ATTAT_PAREF) {

          if (lhs.value.parefres.attr == NULL || rhs.value.parefres.attr == NULL) {
            cqpmessage(Error, "Missing p-attribute on PAREF (lhs or rhs)");
            return EvaluationIsRunning = False;
          }
          else if (lhs.value.parefres.attr == rhs.value.parefres.attr)
            switch (ctptr->node.op_id) {
            case cmp_eq:
              return lhs.value.parefres.token_id == rhs.value.parefres.token_id;
              break;
            case cmp_neq:
              return lhs.value.parefres.token_id != rhs.value.parefres.token_id;
              break;
            default:
              cqpmessage(Error, "Inequality comparisons (>, >=, <, <=) between p-attributes are not allowed.");
              return EvaluationIsRunning = False;
            }
          else {
            char *ls = cl_id2str(lhs.value.parefres.attr, lhs.value.parefres.token_id);
            char *rs = cl_id2str(rhs.value.parefres.attr, rhs.value.parefres.token_id);

            switch (ctptr->node.op_id) {
            case cmp_eq:
              return cl_streq(ls, rs);
            case cmp_neq:
              return !cl_streq(ls, rs);
            default:
              cqpmessage(Error, "Inequality comparisons (>, >=, <, <=) between p-attributes are not allowed.");
              return EvaluationIsRunning = False;
            }
          }
        }
        else if ((lhs.type == ATTAT_PAREF || lhs.type == ATTAT_STRING) && (rhs.type == ATTAT_STRING || rhs.type == ATTAT_PAREF)) {

          /* ok. we have strings or parefs on both sides. we cannot have
           * parefs both left and right, since this is handled above.
           */
          char *ls = NULL, *rs = NULL;

          if (lhs.type == ATTAT_PAREF) {
            assert(lhs.value.parefres.attr);
            ls = cl_id2str(lhs.value.parefres.attr, lhs.value.parefres.token_id);
            if ((ls == NULL) || cl_errno != CDA_OK) {
              cqpmessage(Error,
                         "Error accessing p-attribute %s (lexicon ID #%d).",
                          lhs.value.parefres.attr->any.name, lhs.value.parefres.token_id);
              return EvaluationIsRunning = False;
            }
          }
          else
            ls = lhs.value.charres;

          if (rhs.type == ATTAT_PAREF) {
            assert(rhs.value.parefres.attr);
            rs = cl_id2str(rhs.value.parefres.attr, rhs.value.parefres.token_id);
            if ((rs == NULL) || cl_errno != CDA_OK) {
              cqpmessage(Error,
                         "Error accessing p-attribute %s (lexicon ID #%d).",
                          rhs.value.parefres.attr->any.name, rhs.value.parefres.token_id);
              return EvaluationIsRunning = False;
            }
          }
          else
            rs = rhs.value.charres;

          assert(rs && ls);
          assert(ctptr->node.right);

          /* The string on the right side can be either a RegExp or a literal string.
             RegExps may only occur in string leafs, i.e. strings entered directly in the
             query. All other string values (such as parefs or sarefs, or return values
             of function calls, are literal strings */
          if (ctptr->node.right->type != string_leaf || ctptr->node.right->leaf.pat_type == NORMAL)
            /* literal string */ {
            switch (ctptr->node.op_id) {
            case cmp_eq:
              return cl_streq(ls, rs);
            case cmp_neq:
              return !cl_streq(ls, rs);
            default:
              cqpmessage(Error, "Inequality comparisons (>, >=, <, <=) are not allowed for strings.");
              return EvaluationIsRunning = False;
            }
          }
          else if (ctptr->node.right->leaf.pat_type == REGEXP) {
            /* RegExp */
            if (!(ctptr->node.op_id == cmp_eq || ctptr->node.op_id == cmp_neq)) {
              cqpmessage(Error, "Inequality comparisons (>, >=, <, <=) are not allowed for regular expressions.");
              return EvaluationIsRunning = False;
            }
            if (cl_str_is(rs, ".*"))
              /* see note about .* / .+ optimization above  */
              return (ctptr->node.op_id == cmp_eq) ? True : False;
            else
              /* perform a regular expression match of the two */
              return (ctptr->node.op_id == cmp_eq)
                      ? cl_regex_match(ctptr->node.right->leaf.rx, ls, 0)
                      : !cl_regex_match(ctptr->node.right->leaf.rx, ls, 0)
                      ;
          }
          else {
            cqpmessage(Error, "Internal error in eval_bool()<eval.c>: right->pat_type == CID???");
            return EvaluationIsRunning = False;
          }
        }

        if (lhs.type != rhs.type) {
          cqpmessage(Error, "LHS type (%d) doesn't match RHS type (%d), can't compare.", lhs.type, rhs.type);
          return EvaluationIsRunning = False;
        }

        switch (lhs.type) {
        case ATTAT_INT:
          switch (ctptr->node.op_id) {
          case cmp_gt:
            return lhs.value.intres > rhs.value.intres;

          case cmp_lt:
            return lhs.value.intres < rhs.value.intres;

          case cmp_get:
            return lhs.value.intres >= rhs.value.intres;

          case cmp_let:
            return lhs.value.intres <= rhs.value.intres;

          case cmp_eq:
            return lhs.value.intres == rhs.value.intres;

          case cmp_neq:
            return lhs.value.intres != rhs.value.intres;

          default:
            cqpmessage(Error, "Illegal numerical comparison operator (%d).", ctptr->node.op_id);
            return EvaluationIsRunning = False;
          }
          break;

        case ATTAT_FLOAT:
          switch (ctptr->node.op_id) {
          case cmp_gt:
            return lhs.value.floatres > rhs.value.floatres;

          case cmp_lt:
            return lhs.value.floatres < rhs.value.floatres;

          case cmp_get:
            return lhs.value.floatres >= rhs.value.floatres;

          case cmp_let:
            return lhs.value.floatres <= rhs.value.floatres;

          case cmp_eq:
            return lhs.value.floatres == rhs.value.floatres;

          case cmp_neq:
            return lhs.value.floatres != rhs.value.floatres;

          default:
            cqpmessage(Error, "Illegal numerical comparison operator (%d).", ctptr->node.op_id);
            return EvaluationIsRunning = False;
          }
          break;

        case ATTAT_PAREF:
        case ATTAT_STRING:
        case ATTAT_POS:
        case ATTAT_VAR:
        default:
          assert("In principle, this should be a non-reachable point. Sorry." && 0);
          break;

        }

        /* are we still here? */
        assert("In principle, this should be a non-reachable point. Sorry." && 0);
        break;

      default:
        cqpmessage(Error, "Illegal boolean operand (%d) in pattern.\n",ctptr->node.op_id);
        return EvaluationIsRunning = False;
      }
    }
    else if (ctptr->type == cnode)
      return ctptr->constnode.val != 0;

    else if (ctptr->type == id_list) {
      int res;

      if (eval_debug)
        Rprintf("eval_bool: evaluate id_list membership\n");

      assert(ctptr->idlist.attr);

      if (ctptr->idlist.label) {
        referenced_corppos = get_label_referenced_position(ctptr->idlist.label, rt, curr_cpos);
        if (ctptr->idlist.del) {
          if (eval_debug)
            Rprintf("** AUTO-DELETING LABEL %s = %d\n", ctptr->idlist.label->name, referenced_corppos);
          set_reftab(rt, ctptr->idlist.label->ref, -1);
        }
      }
      else
        referenced_corppos = curr_cpos;

      if (ctptr->idlist.nr_items <= 0)
        res = 0;                /* never member */
      else {
        if (0 <= (id = cl_cpos2id(ctptr->idlist.attr, referenced_corppos)))
          res = bsearch((char *)&id,
                        (char *)(ctptr->idlist.items),
                        ctptr->idlist.nr_items,
                        sizeof(int),
                        intcompare) == NULL ? 0 : 1;
        else
          res = 0;
      }
      return (ctptr->idlist.negated ? !res : res);
    }
    else if (ctptr->type == sbound) {
      assert("Not reached any more" && 0 && ctptr->sbound.strucattr != NULL);

      if (!cl_cpos2struc2cpos(ctptr->sbound.strucattr, curr_cpos, &start, &end) || !cl_all_ok())
        return False;
      else {
        if (ctptr->sbound.is_closing == False)
          /* opening tag */
          return (curr_cpos == start);
        else
          /* closing tag */
          return (curr_cpos == end);
      }
    }
    else {
      cqpmessage(Error,"Internal error in eval_bool()<eval.c>: Illegal node type %d.", ctptr->type);
      return EvaluationIsRunning = False;
    }
  else
    return True;

  assert(0 && "Not reached");
  return False;
}


/**
 * Alters a matchlist so that the start/end values of any match whose start point within
 * the ranges of the given subcorpus are changed to the dummy -1 value.
 *
 * @param  matchlist  Matchlist to modify.
 * @param  subcorpus  Subcorpus to compare it to.
 *                    (If some other kind of CorpusList is passed, it doesn't make sense).
 * @return            Count of matches that were overwritten as -1.
 */
static int
mark_offrange_cells(Matchlist *matchlist, CorpusList *subcorpus)
{
  /*
   * ml_ix  = index into matchlist start/end arrays.
   * sc_ix  = index into the array of range intervals in the subcorpus.
   */
  int sc_ix = 0, ml_ix = 0, n_deletions = 0;

  assert(matchlist);
  assert(subcorpus);
  assert(subcorpus->mother_size > 0);

  /* if this subcorp contains 1 interval which begins at 0 and ends at the end of its mother... */
  if (subcorpus->size == 1 && subcorpus->range[0].start == 0 && subcorpus->range[0].end == subcorpus->mother_size - 1)
      return 0;
  /* no need to do anything. All matches are by definition within range. */

  while (ml_ix < matchlist->tabsize) {
    /* if we have run out of intervals, or if this match begins before the current interval, delete it
     * (because, in either case, the match won't fall into any subsequent range interval either */
    if (sc_ix >= subcorpus->size || matchlist->start[ml_ix] < subcorpus->range[sc_ix].start) {
      matchlist->start[ml_ix] = -1;
      if (matchlist->end)
        matchlist->end[ml_ix] = -1;
      /* move on to the next match, and log the deletion. */
      n_deletions++;
      ml_ix++;
    }

    /* if the match starts after the present interval, move on to the next interval, and try again */
    else if (matchlist->start[ml_ix] > subcorpus->range[sc_ix].end)
        sc_ix++;

    /* match doesn't start before OR after the interval, ergo, matchbegin is within the interval;
     * we therefore keep this one, and move on to the next match. */
    else
      ml_ix++;
  }

  return n_deletions;
}



/**
 * Gets the inital list of matches for a query.
 *
 * NB. This function is called recursively.
 *
 * @return  False iff something has gone wrong.
 */
static Boolean
calculate_initial_matchlist_1(Constrainttree ctptr, Matchlist *matchlist, CorpusList *corpus)
{
  int i;
  Matchlist left, right;

  /* do NOT use free_matchlist here! */

  init_matchlist(&left);
  init_matchlist(&right);

  if (ctptr) {

    if (ctptr->type == bnode) {
      switch(ctptr->node.op_id) {

      case b_and:                /* logical and */

        if (eval_debug)
          Rprintf("calc_initial_ml: boolean and\n");

        assert(ctptr->node.left && ctptr->node.right);

        /* just the beginnings of an implementation for the b_and operator (by oli);
           never mind, the entire <eval.c> code will have to be rewritten at some point
           TODO */
#ifdef INITIAL_MATCH_BY_MU

        if (calculate_initial_matchlist_1(ctptr->node.left, &left, corpus) && calculate_initial_matchlist_1(ctptr->node.right, &right, corpus)) {
          apply_setop_to_matchlist(&left, Intersection, &right);
          free_matchlist(&right);

          matchlist->start = left.start;
          matchlist->end   = left.end;
          matchlist->tabsize = left.tabsize;
          matchlist->matches_whole_corpus = 0;
          matchlist->is_inverted = 0;

          return True;
        }
        else {
          free_matchlist(matchlist)
          free_matchlist(&left);
          free_matchlist(&right);
          return False;
        }
#else
        /* this is the old code. */
        if (calculate_initial_matchlist_1(ctptr->node.left, &left, corpus)) {
          /* We have b_and. So try to eval the right tree for each
           * position yielded by the left tree. */

          for (i = 0; i < left.tabsize; i++) {
            if (!EvaluationIsRunning)
              break;
            if (left.start[i] >= 0 &&
                !eval_bool(ctptr->node.right, NULL, left.start[i]))
              /* we're ignoring labels at the moment, so we pass NULL as reftab */
              left.start[i] = -1;
          }

          if (!apply_setop_to_matchlist(&left, Reduce, NULL))
            return False;

          matchlist->start = left.start;
          matchlist->end   = left.end;
          matchlist->tabsize = left.tabsize;
          matchlist->matches_whole_corpus = 0;
          return True;
        }
        else {
          free_matchlist(matchlist);
          free_matchlist(&left);
          free_matchlist(&right);
          return False;
        }
#endif
        break;

      case b_or:                /* logical or */

        if (eval_debug)
          Rprintf("calc_initial_ml: boolean or\n");

        assert(ctptr->node.left && ctptr->node.right);

        if (calculate_initial_matchlist_1(ctptr->node.left, &left, corpus) && calculate_initial_matchlist_1(ctptr->node.right, &right, corpus)) {

          if (left.is_inverted)
            if (!apply_setop_to_matchlist(&left, Complement, NULL))
              return False;
          if (right.is_inverted)
            if (!apply_setop_to_matchlist(&right, Complement, NULL))
               return False;

          if (!apply_setop_to_matchlist(&left, Union, &right))
            return False;

          free_matchlist(&right);

          matchlist->start = left.start;
          matchlist->end   = left.end;
          matchlist->tabsize = left.tabsize;
          matchlist->matches_whole_corpus = left.matches_whole_corpus;
          matchlist->is_inverted = 0;

          return True;
        }
        else {
          free_matchlist(matchlist);
          free_matchlist(&left);
          free_matchlist(&right);
          return False;
        }
        break;

      case b_implies:                /* logical implication (not optimised in query initial position) */

        /* for the moment, this is just a ridiculous dummy implementation; replace when tables are ready */
        /* matchlist should be initialised and empty, hence invert it to get list of all corpus positions */
        if (!apply_setop_to_matchlist(matchlist, Complement, NULL))
          return False;

        mark_offrange_cells(matchlist, corpus);
        for (i = 0; i < matchlist->tabsize; i++) {
          if (!EvaluationIsRunning) {
            free_matchlist(matchlist);
            return False;
          }
          if (matchlist->start[i] >= 0 && !eval_bool(ctptr, NULL, matchlist->start[i]))
            /* we're ignoring labels at the moment, so we pass NULL as reftab */
            matchlist->start[i] = -1;
        }

        if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
          return False;

        return True;
        break;

      case b_not:                /* logical negation */

        if (eval_debug)
          Rprintf("calc_initial_ml: boolean not\n");

        assert(ctptr->node.left);

        if (calculate_initial_matchlist_1(ctptr->node.left, matchlist, corpus)) {
          if (!apply_setop_to_matchlist(matchlist, Complement, NULL))
            return False;

          if (mark_offrange_cells(matchlist, corpus))
            if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
              return False;

          return True;
        }
        else {
          free_matchlist(matchlist);
          return False;
        }

        break;

        /* relational operators */
      case cmp_gt:
      case cmp_lt:
      case cmp_get:
      case cmp_let:
      case cmp_eq:
      case cmp_neq:
      case cmp_ex:
        if (eval_debug)
          Rprintf("calc_initial_ml: evaluate comparison [%s]\n", get_b_operator_name(ctptr->node.op_id));

        /* check argument types */
        assert(ctptr->node.left && (ctptr->node.op_id == cmp_ex || ctptr->node.right) );

        /* on the left, there can be
         *   func
         *   pa_ref
         * on the right,
         *   string_leaf
         *   int_leaf
         * may occur additionally (why no constants on the lhs?).
         */

        switch (ctptr->node.left->type) {

        case func:
        case sa_ref:
          /* for the moment, this is just a ridiculous dummy implementation; replace when tables are ready */
          /* matchlist should be initialised and empty, hence invert it to get list of all corpus positions */
          if (!apply_setop_to_matchlist(matchlist, Complement, NULL))
            return False;

          mark_offrange_cells(matchlist, corpus);
          for (i = 0; i < matchlist->tabsize; i++) {
            if (!EvaluationIsRunning) {
              free_matchlist(matchlist);
              return False;
            }
            if (matchlist->start[i] >= 0 && !eval_bool(ctptr, NULL, matchlist->start[i]))
              /* we're ignoring labels at the moment, so we pass NULL as reftab */
              matchlist->start[i] = -1;
          }

          if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
            return False;
          return True;

        case pa_ref:
          if (ctptr->node.left->pa_ref.label) {
            /*
             * In principle, this shouldn't happen because no labels are defined in query initial position.
             * Exceptions are the special labels match (which is undefined at this point) and _ ("this", which refers to the current corpus position).
             * Implementing a work-around for the redundant [_.word = "..."] query seems pointless, but directly locating a specific corpus
             * position with [ _ = 1042 ] is really useful as a special case.
             */

            /* special case code for [ _ = <cpos> ] */
            if (cl_str_is(ctptr->node.left->pa_ref.label->name, "_") && ctptr->node.right && ctptr->node.right->type == int_leaf) {
              if (ctptr->node.op_id != cmp_eq) {
                cqpmessage(Error, "Only [ _ = <cpos> ] is allowed in query-initial position, no inequality comparisons (!=, >, >=, <, <=)");
                return False;
              }
              matchlist->start = (int *) cl_malloc(sizeof(int)); /* generate matchlist with specified cpos */
              matchlist->start[0] = ctptr->node.right->leaf.ctype.iconst;
              matchlist->tabsize = 1;
              matchlist->is_inverted = False;
              matchlist->matches_whole_corpus = False;
              return True;
            }

            /* all other label references are invalid */
            cqpmessage(Error, "Reference to label '%s' not allowed in query initial position.", ctptr->node.left->pa_ref.label->name);
            return False;
          }
          else {
            /*
             * we have a non-labelled pa_ref on the left;
             * should have a string atom on the right
             */

            if (!ctptr->node.right || ctptr->node.right->type != string_leaf) {
              /* for the moment, this is just a ridiculous dummy implementation; replace when tables are ready */
              /* matchlist should be initialised and empty, hence invert it to get list of all corpus positions */
              if (!apply_setop_to_matchlist(matchlist, Complement, NULL))
                return False; /* usually an out-of-memory error, which can happen easily here */

              mark_offrange_cells(matchlist, corpus);
              for (i = 0; i < matchlist->tabsize; i++) {
                if (!EvaluationIsRunning) {
                  free_matchlist(matchlist);
                  return False;
                }

                if (matchlist->start[i] >= 0 &&
                    !eval_bool(ctptr, NULL, matchlist->start[i]))
                  /* we're ignoring labels at the moment, so we pass NULL as reftab */
                  matchlist->start[i] = -1;
              }
              if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
                return False;
              return True;
            }

            /* the code below this point only works for == and != comparisons */
            if ( !(ctptr->node.op_id == cmp_eq || ctptr->node.op_id == cmp_neq) ) {
              cqpmessage(Error, "Inequality comparisons (>, >=, <, <=) are not allowed for p-attributes.");
              return False;
            }

            switch (ctptr->node.right->leaf.pat_type) {
            case REGEXP:
              /* check whether we have a ".*" on the right -- in this case
               * there is nothing to do (matched by everything)
               */
              if (cl_str_is(ctptr->node.right->leaf.ctype.sconst, ".*")) {
                if (ctptr->node.op_id == cmp_neq) {
                  /* every word is != ".*", so just return an empty match list */
                  free_matchlist(matchlist);
                  return True;
                }
                else {
                  /* return a copy of the corpus (expensive, but what shall we do)? */
                  get_matched_corpus_positions(ctptr->node.left->pa_ref.attr,
                                               ".*",
                                               0,
                                               matchlist,
                                               (int *)corpus->range,
                                               corpus->size);
                  if (mark_offrange_cells(matchlist, corpus))
                    if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
                      return False;
                  return True;
                }
              }
              else
                 get_matched_corpus_positions(ctptr->node.left->pa_ref.attr,
                                              ctptr->node.right->leaf.ctype.sconst,
                                              ctptr->node.right->leaf.canon,
                                              matchlist,
                                              (int *)corpus->range,
                                              corpus->size);
              break;

            case NORMAL:
              /* get postions where the LHS att == the RHS string const */
              get_corpus_positions(ctptr->node.left->pa_ref.attr, ctptr->node.right->leaf.ctype.sconst, matchlist);
              break;

            case CID:
              matchlist->start = cl_id2cpos_oldstyle(ctptr->node.left->pa_ref.attr,
                                                     ctptr->node.right->leaf.ctype.cidconst,
                                                     &(matchlist->tabsize),
                                                     (int *)corpus->range,
                                                     corpus->size);
              matchlist->matches_whole_corpus = 0;
              matchlist->is_inverted = 0;
              matchlist->end = NULL;
              break;

            default:
              cqpmessage(Error, "Unknown pattern type (%d) on RHS of comparison operator.", ctptr->node.right->leaf.pat_type);
              return False;
              break;
            }

            if (mark_offrange_cells(matchlist, corpus))
              if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
                return False;

            if (ctptr->node.op_id == cmp_neq)
              if (!apply_setop_to_matchlist(matchlist, Complement, NULL))
                return False;
                /* usually an out-of-memory error */
          }
          return True;

        default:
          cqpmessage(Error, "Wrong node type (%d) on LHS of comparison operator.", ctptr->node.left->type);
          break;

        }   /* switch (ctptr->node.left->type) ... */
        break;

      default:
        assert("Internal error in calculate_initial_matchlist_1(): Unknown comparison operator." && 0);
        break;

      }     /* endswitch (ctptr->node.op_id) ... */
    } /* endif (ctptr->type == bnode) */
    else if (ctptr->type == cnode) {

      if (ctptr->constnode.val == 0) {
        matchlist->start = NULL;
        matchlist->end   = NULL;
        matchlist->tabsize = 0;
        matchlist->matches_whole_corpus = 0;
        matchlist->is_inverted = 0;
      }
      else {
        get_matched_corpus_positions(ctptr->node.left->pa_ref.attr,
                                     ".*",
                                     0,
                                     matchlist,
                                     (int *)corpus->range,
                                     corpus->size);
        if (mark_offrange_cells(matchlist, corpus))
          if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
            return False;
      }

      return True;
    }
    else if (ctptr->type == id_list) {

      if (ctptr->idlist.label == NULL) {
        if (ctptr->idlist.nr_items > 0) {
          assert(ctptr->idlist.attr);

          matchlist->start = cl_idlist2cpos(ctptr->idlist.attr,
                                            ctptr->idlist.items,
                                            ctptr->idlist.nr_items,
                                            1, /* sort: yes */
                                            &(matchlist->tabsize));
          matchlist->end = NULL;
          matchlist->matches_whole_corpus = 0;
          matchlist->is_inverted = 0;
        }
        else {
          matchlist->start = NULL;
          matchlist->end   = NULL;
          matchlist->tabsize = 0;
          matchlist->matches_whole_corpus = 0;
          matchlist->is_inverted = 0;
        }

        if (ctptr->idlist.negated)
          if (!apply_setop_to_matchlist(matchlist, Complement, NULL))
            return False;

        if (mark_offrange_cells(matchlist, corpus))
          if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
            return False;

        return True;
      }
      else {
        cqpmessage(Error, "Reference to label '%s' not allowed in query initial position.", ctptr->node.left->idlist.label->name);
        return False;
      }    /* if (ctptr->idlist.label == NULL) ... else ... */
    }

    else {
      cqpmessage(Error, "Internal error in calculate_initial_matchlist_1()<eval.c>: Illegal node type %d.\n", ctptr->type);
      return False;
    }   /* if (ctptr->type == bnode) ... else if ...  */
  } /* endif ctptr */
  else
    /* if ctptr is NULL */
    return True;

  assert("Internal error in calculate_initial_matchlist1(): went over the edge." && 0);
  return 0;
}

/**
 * Wrapper around calculate_initial_matchlist_1, qv.
 *
 * @see calculate_initial_matchlist_1
 */
static Boolean
calculate_initial_matchlist(Constrainttree ctptr, Matchlist *matchlist, CorpusList *corpus)
{
  Boolean res = calculate_initial_matchlist_1(ctptr, matchlist, corpus);

  /* i.e. if calling the main function worked, and a matchlist was created */
  if (res && matchlist) {
    if (matchlist->is_inverted) {
      matchlist->is_inverted = 0;
      res = apply_setop_to_matchlist(matchlist, Complement, NULL);
    }
    if (res && mark_offrange_cells(matchlist, corpus))
      res = apply_setop_to_matchlist(matchlist, Reduce, NULL);
  }

  return res;
}




/**
 * try to match the given word form pattern and return success.
 */
static Boolean
match_first_pattern(AVS pattern, Matchlist *matchlist, CorpusList *corpus)
{
  int nr_strucs, nr_ok, ok, i, k, n, start, end, cpos;
  cpos = -1;
  Range* matches;
  Bitfield bf;
  char *val;
#if COMPILE_QUERY_OPTIMIZATION
  float red;
  int nr_pos;
#endif

  assert(pattern);

  switch (pattern->type) {

  case Tag:
    assert(pattern->tag.attr != NULL);

    nr_strucs = cl_max_struc(pattern->tag.attr);
    if (nr_strucs <= 0) {        /* CL considers 0 regions a missing data error, but we won't be that strict */
      matchlist->tabsize = 0;        /* should be initialised to that, but make sure we report 0 matches  */
      return True;
    }

    /* if there is a constraint, match annotated strings first */
    bf = create_bitfield(nr_strucs); /* always use bitfield (the memory overhead is acceptable) */
    if (pattern->tag.constraint) {
      clear_all_bits(bf);
      nr_ok = 0;
      for (i = 0; (i < nr_strucs) && (EvaluationIsRunning); i++) {
        val = cl_struc2str(pattern->tag.attr, i);
        if (val) {
          if (pattern->tag.rx)
            ok = cl_regex_match(pattern->tag.rx, val, 0);
          else
            ok = (0 == strcmp(pattern->tag.constraint, val));
          if (pattern->tag.negated)
            ok = !ok;
        }
        else
          ok = 0;
        if (ok) {
          set_bit(bf, i);
          nr_ok++;
        }
      }
      if (!EvaluationIsRunning)
        nr_ok = 0;                /* user abort -> stop query execution */
    }
    else {
      set_all_bits(bf);                /* no constraint -> all regions are possible start points */
      nr_ok = nr_strucs;
    }

    if (nr_ok <= 0) {                /* no matches -> return empty matchlist */
      destroy_bitfield(&bf);
      matchlist->tabsize = 0;
      return True;
    }
    else {
      /* compute the initial matchlist according to the flags in bf */
      matchlist->start = (int *)cl_malloc(sizeof(int) * nr_ok);
      matchlist->end = NULL;
      matchlist->matches_whole_corpus = 0;

      k = 0;
      for (i = 0; i < nr_strucs; i++) {
        if (get_bit(bf, i)) {
          if (!cl_struc2cpos(pattern->tag.attr, i, &start, &end)) {
            destroy_bitfield(&bf);
            cl_free(matchlist->start);
            return False;
          }
          matchlist->start[k++] = (pattern->tag.is_closing) ? (end + 1) : start;
          /* NB: it's (end+1) for a closing tag, since the tag refers to the token at cpos-1 */
        }
      }
    }
    destroy_bitfield(&bf);
    matchlist->tabsize = nr_ok;

    return True;
    /* endcase Tag */

  case Anchor:
    /* first, check some error conditions */
    if (0 == corpus->size || NULL == corpus->range) {
      cqpmessage(Error, "Subquery on empty corpus. Not evaluated.");
      return False;
    }
    if (pattern->anchor.field == TargetField && NULL == corpus->targets) {
      cqpmessage(Error, "No <target> anchors found in query corpus.");
      return False;
    }
    if (pattern->anchor.field == KeywordField && NULL == corpus->keywords) {
      cqpmessage(Error, "No <keyword> anchors found in query corpus.");
      return False;
    }

    /* allocate matchlist with maximal size required */
    matchlist->start = (int *)cl_malloc(sizeof(int) * corpus->size);
    matchlist->end = NULL;
    matchlist->matches_whole_corpus = 0;
    matchlist->tabsize = corpus->size;

    /* now go through all ranges in the query corpus and copy the corresponding anchor position into the matchlist */
    for (i = 0; i < corpus->size; i++) {
      switch (pattern->anchor.field) {
      case MatchField:
        cpos = corpus->range[i].start;
        break;
      case MatchEndField:
        cpos = corpus->range[i].end;
        break;
      case TargetField:
        cpos = corpus->targets[i];
        break;
      case KeywordField:
        cpos = corpus->keywords[i];
        break;
      default:
        assert("Internal Error" && 0);
      }
      if (pattern->anchor.is_closing && cpos >= 0)
        cpos++;                        /* </target> anchor refers to point after target token etc. */
      matchlist->start[i] = cpos;
    }

    /* remove 'undefined' target or keyword anchors */
    if (!apply_setop_to_matchlist(matchlist, Reduce, NULL))
      return False;

    return True;
    /* endcase Anchor */

  case Pattern:
    /* no need to set labels here, since this is done in the following NFA simulation */

#if COMPILE_QUERY_OPTIMIZATION
    /* NB. this is one of two effects that 'query_optimize=true' has. the other is to switch on optimization in the CL. */
    if (query_optimize &&
        (red = red_factor(corpus, &nr_pos)) >= 0.0 && red < RED_THRESHOLD) {

      /*
       * THIS IS CURRENTLY DISABLED
       * ----------------------------------------------------------------------
       *
       * The evaluator optimization expects the ``initial matchlist''
       * to be exact, that is, there mustn't be any CPs in it which don't
       * reflect to valid start states in the automaton.
       *
       * Fri Mar 17 11:12:47 1995 (oli)
       */

      if (!silent)
        Rprintf("QOpt: %f (pos %d)\n", red, nr_pos);

      matchlist->start = (int *)cl_malloc(sizeof(int) * nr_pos);
      matchlist->end = NULL;
      matchlist->matches_whole_corpus = 0;
      if (matchlist->start == NULL)
        return False;
      matchlist->tabsize = nr_pos;

      k = 0;
      for (i = 0; i < corpus->size; i++)
        for (start = corpus->range[i].start; start <= corpus->range[i].end; start++) {
          assert(k < nr_pos);
          matchlist->start[k++] = start;
        }

      assert(k == nr_pos);

      if (!silent)
        Rprintf("QOpt: copied ranges\n");

      return k == nr_pos;
    }
    else
      return calculate_initial_matchlist(pattern->con.constraint, matchlist, corpus);
      /* this is what always happens at present : do it without the query optimisations (see oli's comment above) */
#else
    return calculate_initial_matchlist(pattern->con.constraint, matchlist, corpus);
#endif
    /* endcase Pattern */

  case MatchAll:
    get_matched_corpus_positions(NULL, ".*", 0, matchlist, (int *)corpus->range, corpus->size);
    return True;

  case Region:
    /* If the initial transition is the ENTER operation of a Region element, potential start points for the FSA are
     * the start tokens of all regions of the corresponding s-attribute or all matching ranges of the corresponding NQR.
     * Note that the FSA simulation must not skip the initial transition in this case so that the range can be added
     * to the wait queue.
     */
    if (pattern->region.opcode != AVSRegionEnter) {
      cqpmessage(Error, "internal error -- WAIT or EMIT operation of <<%s>> cannot be initial transition", pattern->region.name);
      return False;
    }
    if (pattern->region.nqr) {
      /* de-duplicate start positions so we don't run simulation multiple times for the same initial position */
      n = pattern->region.nqr->size;
      matches = pattern->region.nqr->range;
      nr_ok = 0; /* number of unique start positions (NB: ranges are sorted by start position) */
      for (i = 0, cpos = -1; i < n; i++) {
        if (matches[i].start != cpos)
          nr_ok++;
        cpos = matches[i].start;
      }
      matchlist->start = cl_malloc(sizeof(int) * nr_ok);
      matchlist->end = NULL;
      matchlist->matches_whole_corpus = 0;
      matchlist->tabsize = nr_ok;
      for (i = 0, cpos = -1, k = 0; i < n; i++) {
        if (matches[i].start != cpos)
          matchlist->start[k++] = matches[i].start;
        cpos = matches[i].start;
      }
      return True;
    }
    else if (pattern->region.attr) {
      n = cl_max_struc(pattern->region.attr);
      matchlist->start = cl_malloc(sizeof(int) * n);
      matchlist->end = NULL;
      matchlist->matches_whole_corpus = 0;
      matchlist->tabsize = n;
      for (i = 0; i < n; i++)
        cl_struc2cpos(pattern->region.attr, i, &(matchlist->start[i]), &k);
      return True;
    }
    else
      assert(0 && "Internal error: <<region>> element has neither s-attribute nor NQR set.");

    return True;
    /* any unhandled opcodes or AVS types fall through to "not reached" below */
  }

  assert(0 && "Not reached");
  return False;
}



/**
 * Simulate the DFA in the global evalenv.
 *
 * ("Simulate", in technical Comp Sci terminology on finite state machines, basically means "run" -
 * since CQP is, technically, only *simulating* the abstract machine described by the DFA!)
 *
 * Returns True if the matching ranges may be out-of-order and need to be sorted.
 */
static int
simulate(Matchlist *matchlist,
         int *cut,
         int *state_vector,
         int *target_vector,
         RefTab *reftab_vector,
         RefTab *reftab_target_vector,
         int start_transition)
{
  int p, cpos, effective_cpos;
  int strict_regions_ok, lookahead_constraint, zero_width_pattern;
  int target_state, transition_valid;
  int state,  /* used for index into param state_vector */
    boundary, b1, b2,
    running_states,
    winner, winner_start,  /* end and start cpos of successful match */
    my_target, my_keyword, /* target and keyword anchors of successful match */
    this_is_a_winner;      /* whether FSA simulation has reached a final state */
  int *help;
  RefTab *help2;
  AVStructure *condition;
  int nr_transitions = 0;
  int percentage, new_percentage; /* for ProgressBar option */
  int infinite_loop = 0; /* detect infinite loop if state vector doesn't change for a certain number of time steps */
  int has_match_selector = /* whether we need to extract a sub-span of the match */
      evalenv->match_selector.begin || evalenv->match_selector.begin_offset ||
      evalenv->match_selector.end || evalenv->match_selector.end_offset;

  assert(evalenv->query_corpus);
  assert(evalenv->query_corpus->size > 0);
  assert(evalenv->query_corpus->range);
  assert(matchlist);
  assert(matchlist->start);
  assert(matchlist->end);

  /*
   * state 0 must neither be final nor error
   */

  assert(!evalenv->dfa.Final[0] && evalenv->dfa.E_State != 0);

  if (evalenv->query_corpus->size == 0 || evalenv->query_corpus->range == NULL)
    /* i.e. if the query corpus doesn't exist... free match list, go straight to end of func. */
    free_matchlist(matchlist);

  else {
    /* whole of rest of function */

    int r_ix = 0; /* index into the range array; indicates our current range */
    int i = 0;  /* index into  array of match start cpos */

    assert(state_vector);
    assert(target_vector);
    assert(reftab_vector);
    assert(reftab_target_vector);

    percentage = -1;

    while ((i < matchlist->tabsize) && ((*cut) != 0) && EvaluationIsRunning) {
      if (progress_bar && !evalenv->aligned) {
        new_percentage = floor(0.5 + (100.0 * i) / matchlist->tabsize);
        if (new_percentage > percentage) {
          percentage = new_percentage;
          progress_bar_percentage(0, 0, percentage);
        }
      }
      /*
       * find the appropriate range
       * three cases:
       * 1 start point smaller than range beginning
       *   we should have considered that start point before.
       *   so we cannot have a match in this case.
       *   action: assign -1 to matchlist->start and increment i
       * 2 start point within range (ok)
       *   action: simulate automaton
       * 3 start point beyond range end
       *   we have to check whether a later range will contain the
       *   starting point.
       *   action: increment rp.
       */

      my_target = -1;
      my_keyword = -1;

      if (simulate_debug)
        Rprintf("Looking at matchlist element %d (cpos %d)\n  range[rp=%d]=[%d,%d]\n",
                i, matchlist->start[i],
                r_ix,
                r_ix < evalenv->query_corpus->size ? evalenv->query_corpus->range[r_ix].start : -1,
                r_ix < evalenv->query_corpus->size ? evalenv->query_corpus->range[r_ix].end   : -1);

      if (r_ix >= evalenv->query_corpus->size || matchlist->start[i] < evalenv->query_corpus->range[r_ix].start) {
        /* case 1 - no match possible
         */
        matchlist->start[i] = -1;
        i++;
      }

      else if (matchlist->start[i] > evalenv->query_corpus->range[r_ix].end)
        /* case 3 -  no match in this range.
         * check for a later one (but keep the current matchlist element)
         */
        r_ix++;

      else {

        /* case 2 - simulate automaton */

        /* determine maximal right boundary for this match:
         * boundary = MIN() of
         *  - boundary given by "within" clause (defaults to hard_boundary, if there is no "within" clause)
         *  - right boundary of current range in the query_corpus (for subqueries)
         */
        b1 = calculate_rightboundary(evalenv->query_corpus, matchlist->start[i], evalenv->search_context);
        b2 = evalenv->query_corpus->range[r_ix].end;
        boundary = MIN(b1, b2);

        if (simulate_debug)
          Rprintf("Starting NFA simulation. Max bound is %d\n", boundary);

        if (boundary == -1) {
          /* no match here, since not within selected boundary. */
          matchlist->start[i] = -1;
          if (simulate_debug)
            Rprintf("  ... not within selected boundary\n");
        }
        else {
          int first_transition_traversed, j;

          /*
           * set up some 'global' variables in evalenv (which subroutines may need to use)
           */
          evalenv->rp = r_ix;        /* current range (in subquery); used to evaluate Anchor constraints */

          /*
           * all states are inactive / reset label references
           */
          for (state = 0; state < evalenv->dfa.Max_States; state++) {
            state_vector[state] = -1;
            reset_reftab(reftab_vector[state]);
          }

          /*
           * activate initial state and set the special "match" label
           */
          state_vector[0] = matchlist->start[i];
          set_reftab(reftab_vector[0], evalenv->match_label->ref, matchlist->start[i]);

          running_states = 1;  /* the number of currently active states */
          winner = -1;         /* the end position of the winning (final) state (or range extracted by a match selector) */
          winner_start = matchlist->start[i]; /* corresponding start position (may be modified by a match selector) */

          first_transition_traversed = 0; /* the first transition was not yet traversed */

          /*
           * make sure that wait queues for any Region elements in the query are empty
           * (might need to discard remaining items from last simulation run, if match was found with non-empty queue)
           */

          for (j = 0; j <= evalenv->MaxPatIndex; j++) {
            if (evalenv->patternlist[j].type == Region &&
                evalenv->patternlist[j].region.opcode == AVSRegionEnter &&
                evalenv->patternlist[j].region.queue) /* skip for a zero-width region element without queue */
                StateQueue_clear(evalenv->patternlist[j].region.queue); /* clearing multiple times wouldn't hurt, but want to avoid two extra function calls */
          }

          /* bail out on the first winner, unless matching_strategy == longest match
             (in longest_match strategy, wait until we don't have any running states left) */
          while ( (winner < 0 || evalenv->matching_strategy == longest_match) && running_states > 0 ) {

            /*
             * the core of the whole simulation
             */
            if (simulate_debug) {
              Rprintf("\nActive states: [ ");
              for (state = 0; state < evalenv->dfa.Max_States; state++) {
                if (state_vector[state] >= 0)
                  Rprintf("s%d=%d ", state, state_vector[state]);
              }
              Rprintf("]\n");
            }

            /* first, clear the list of target states */
            for (state = 0; state < evalenv->dfa.Max_States; state++)
              target_vector[state] = -1;
            /* no need to reset the target reftab, since only reftab associated with active states will be considered */

            for (state = 0 ;
                 state < evalenv->dfa.Max_States &&
                   (winner < 0 || evalenv->matching_strategy == longest_match) &&  /* abort when we've found a winner, unless strategy is longest match */
                   running_states > 0 ;                      /* no remaining active states -> simulation finished */
                 state++) {
              cpos = state_vector[state];
              /* A state always refers to a point between two consecutive tokens.
               * cpos = state_vector[state] is the corpus position of the second token.
               * Hence, in a query, patterns and open tags refer to the second token (at cpos),
               * where as closing tags refer to the first token (at cpos-1)
               */

              if (simulate_debug && cpos >= 0) {
                Rprintf("  state %d, cpos %d...\n", state, cpos);
                if (symtab_debug)
                  print_label_values(evalenv->labels, reftab_vector[state], cpos);
              }

              /* Transitions from this state are allowed if:
               * - cpos >= 0         -->  state is active
               * - cpos <= boundary  -->  within right boundary for this match
               * !! if the next query element is a closing tag, we have to substitue cpos-1 for cpos in this condition !!
               * --> we'll test (cpos >= 0) right now and the other condition separately for each possible transition
               */

              if (cpos >= 0) {        /* active state */
                running_states--; /* this state becomes inactive; it will spawn new active states if there are valid transitions */

                /* cycle through all possible transitions from this state; in shortest_match mode, stop evaluation as soon as there is a winner */
                for (p = 0 ; p < evalenv->dfa.Max_Input && (winner < 0 || evalenv->matching_strategy == longest_match) ; p++) {
                  cpos = state_vector[state]; /* may be modified by transition to set next cpos correctly */

                  /* the target state (that is, the state we reach after the transition) */
                  target_state = evalenv->dfa.TransTable[state][p];

                  /* if we reach a non-error state with this transition */
                  if (target_state != evalenv->dfa.E_State) {

                    /* condition is the AVStructure ("pattern") associated with this transition */
                    condition = &(evalenv->patternlist[p]);

                    /* check whether the associated condition is a lookahead constraint pattern */
                    lookahead_constraint =
                      ((condition->type == MatchAll) && (condition->matchall.lookahead)) ||
                      ((condition->type == Pattern) && (condition->con.lookahead));

                    /* tags, anchors, lookahead constraints, and all phases of a region element are zero-width patterns */
                    zero_width_pattern =
                      (condition->type == Tag)     || (condition->type == Anchor) ||
                      (condition->type == Region ) || lookahead_constraint;

                    /* check if we're still in range (i.e. effective_cpos <= boundary; except for lookahead constraint) */
                    effective_cpos = cpos;
                    /* a closing tag or anchor point refers to effective_cpos = cpos-1 */
                    if ( (condition->type == Tag && condition->tag.is_closing) || (condition->type == Anchor && condition->anchor.is_closing) )
                      effective_cpos--;

                    /* In StrictRegions mode, we have to check all constraints imposed by region boundaries now. */
                    strict_regions_ok = 1;
                    if (strict_regions) {
                      int flags = LAB_RDAT | LAB_DEFINED | LAB_USED; /* 'active' labels in rdat namespace */
                      LabelEntry rbound_label = symbol_table_new_iterator(evalenv->labels, flags);
                      while (rbound_label != NULL) {
                        int rbound = get_reftab(reftab_vector[state], rbound_label->ref, -1);
                        if ((rbound >= 0) && (effective_cpos > rbound))
                          strict_regions_ok = 0; /* a within region constraint has been violated */
                        rbound_label = symbol_table_iterator(rbound_label, flags);
                      }
                    }

                    /* if we're in range, evaluate the constraint and activate target state iff true */
                    if ( ((effective_cpos <= boundary) || (lookahead_constraint && (effective_cpos == boundary+1))) && strict_regions_ok ) {

                      /* IF this is the first transition we pass from the start state,
                       * AND it is the one for which we built the initial matchlist,
                       * we can save a little time if we don't evaluate the condition again;
                       * if (start_transition >= 0) we can also safely ignore all other
                       * transitions from the start state because they will crop up in another
                       * initial matchlist (start_transition < 0 happens in "aligned" queries, for instance).
                       *
                       * !! THIS MESSES UP THE SHORTEST MATCH STRATEGY !!  ->  results have to be cleaned up after the query
                       *
                       * There is one exception: if the first transition is (the ENTER phase of) a Region element,
                       * we must not skip it because of its side-effects (adding a FSA state to the wait list in
                       * order to be emitted at the end of the region). Otherwise, we would have to duplicate the
                       * side-effects here as is done for Pattern elements below (but much more code for Region).
                       * However, we can still safely skip the Region element if it's not the current start transition.
                       */
                      if (state == 0 && start_transition >= 0 && first_transition_traversed == 0 &&
                          !(start_transition == p && condition->type == Region)) {
                        transition_valid = (start_transition == p) ? 1 : 0;

                        /* if this first transition is valid, we need to copy the reftab
                         * and set the optional label associated with this transition (otherwise done by eval_constraint()) */
                        if (transition_valid) {
                          LabelEntry label;

                          /* copy the state's reftab to the target state (done by eval_constrain() in the other cases) */
                          dup_reftab(reftab_vector[state], reftab_target_vector[target_state]);
                          if (condition->type == Pattern)
                            label = condition->con.label;
                          else if (condition->type == MatchAll)
                            label = condition->matchall.label;
                          else
                            label = NULL;

                          if (label != NULL)
                            set_reftab(reftab_target_vector[target_state], label->ref, effective_cpos); /* see below */

                          /* if the skipped first pattern was an open tag, we have to set the corresponding label in StrictRegions mode here */
                          if (strict_regions && (condition->type == Tag)) {
                            int start, end;
                            if ( !condition->tag.is_closing && condition->tag.right_boundary != NULL && cl_cpos2struc2cpos(condition->tag.attr, effective_cpos, &start, &end) )
                              set_reftab(reftab_target_vector[target_state], condition->tag.right_boundary->ref, end);
                          }
                        }
                      }
                      /* OTHERWISE we evaluate the condition associated with the transition
                       * (NB this condition refers to the effective_cpos!)
                       */
                      else {       /* if ((state == start_state) && ... ) ...  */
                        transition_valid = eval_constraint(condition, effective_cpos, reftab_vector[state], reftab_target_vector[target_state], &cpos);
                        /* Note that a Region element will update &cpos to:
                         *  - HOLD phase: cpos + 1 (normally) or cpos (if further states are to be emitted at the same cpos)
                         *  - EMIT phase: end_of_region + 1 (which becomes the next cpos of the emitted state, but is
                         *                adjusted to matchend == end_of_region if it the FSA has reached a target state)
                         * This logic only works correctly if Region elements are treated as zero-width patterns, which
                         * implies that the ENTER phase doesn't advance the cpos and can correctly emit a single-token region
                         * in the next simulation step (so the total delay for a Region element is one time step).
                         */
                      }

                      /* now set target / keyword for this transition */
                      if (transition_valid) {
                        target_nature pattern_is_targeted; /* 0 = not marked, 1 = marked as target, 2 = marked as keyword */
                        if (condition->type == Pattern)
                          pattern_is_targeted = condition->con.is_target;
                        else if (condition->type == MatchAll)
                          pattern_is_targeted = condition->matchall.is_target;
                        else
                          pattern_is_targeted = IsNotTarget;

                        if (pattern_is_targeted == IsTarget)
                          /* the special "target" label */
                          set_reftab(reftab_target_vector[target_state], evalenv->target_label->ref, effective_cpos);
                          /* since only patterns can be targeted, this is == cpos at the moment, but why not change it? */

                        if (pattern_is_targeted == IsKeyword)
                          /* the special "keyword" label */
                          set_reftab(reftab_target_vector[target_state], evalenv->keyword_label->ref, effective_cpos);
                      }

                      /* now, finally, check if we have a winner, i.e.
                       *   - the target state is a final state
                       *   - the optional global constraint is fulfilled
                       * and then advance to the next token (unless transition was associated with a tag or anchor point)
                       */
                      if (transition_valid) {
                        nr_transitions++;
                        if (nr_transitions >= 20000) {
                          CheckForInterrupts();
                          nr_transitions = 0;
                        }

                        if (simulate_debug) {
                          Rprintf("Transition %d --%d-> %d  (pattern %d TRUE at cpos=%d)\n", state, p, target_state, p, effective_cpos);
                          if (symtab_debug)
                            print_label_values(evalenv->labels, reftab_target_vector[target_state], effective_cpos);
                        }

                        /* check for winner */
                        this_is_a_winner = 0;
                        if (evalenv->dfa.Final[target_state]) {
                          if (evalenv->gconstraint == NULL)
                            this_is_a_winner = 1;
                          else {
                            int matchend_cpos = zero_width_pattern ? cpos - 1 : cpos;
                            /* set special "matchend" label before the global constraint is evaluated */
                            set_reftab(reftab_target_vector[target_state], evalenv->matchend_label->ref, matchend_cpos);
                            /* evaluate global constraint with current cpos set to -1 (undef) */
                            if (eval_bool(evalenv->gconstraint, reftab_target_vector[target_state], -1))
                              this_is_a_winner = 1;

                            /* delete "matchend" label in case we continue the simulation (longest match mode) */
                            set_reftab(reftab_target_vector[target_state], evalenv->matchend_label->ref, -1);
                          }
                        }

                        if (this_is_a_winner) {
                          if (simulate_debug)
                            Rprintf("Winning cpos found at %d\n", cpos);

                          /* remember the last token (cpos) of this winner & its target position (if set) */
                          winner = zero_width_pattern ?  cpos - 1 : cpos;
                          /* for zero-width elements, the last token of the match is the token _before_ the current corpus position
                           * [NB: for closing tags and anchors we could have used the effective cpos, but not for open tags and lookahead constraints]
                           */

                          /* If there is a match selector, adjust the start and end cpos of the match accordingly,
                           * setting winner_start = -1 to indicate an invalid match (matchend < match or out of corpus range). */
                          if (has_match_selector) {
                            /* NB: always set winner_start so winner_start = -1 can be overwritten by a later valid match with matching_strategy == longest_match */
                            if (evalenv->match_selector.begin)
                              winner_start = get_reftab(reftab_target_vector[target_state], evalenv->match_selector.begin->ref, -1);
                            else
                              winner_start = matchlist->start[i];
                            if (winner_start >= 0)
                              winner_start += evalenv->match_selector.begin_offset;
                            if (evalenv->match_selector.end)
                              winner = get_reftab(reftab_target_vector[target_state], evalenv->match_selector.end->ref, -1);
                            /* else keep the end cpos set above */
                            if (winner >= 0)
                              winner += evalenv->match_selector.end_offset;
                            if (winner < 0 || winner >= evalenv->query_corpus->mother_size ||
                                winner_start < 0 || winner_start >= evalenv->query_corpus->mother_size ||
                                winner < winner_start) {
                              winner_start = -1; /* extracted range is invalid -> mark as deleted */
                              winner = 0; /* so that we still exit the simulation unless matching_strategy == longest_match */
                            }
                          }

                          if (evalenv->has_target_indicator)
                            my_target = get_reftab(reftab_target_vector[target_state], evalenv->target_label->ref, -1);
                          if (evalenv->has_keyword_indicator)
                            my_keyword = get_reftab(reftab_target_vector[target_state], evalenv->keyword_label->ref, -1);

                          /* NB If (matching_strategy == longest_match) later winners will overwrite
                           * the <winner>, <winner_start>, <my_target> and <my_keyword> variables */
                        }

                        /* if our matching strategy is longest_match, we have to activate the target state,
                         * so our winner can expand to a longer match in queries like ''"ADJA" "NN"+;'' */
                        if (!this_is_a_winner || evalenv->matching_strategy == longest_match) {
                          /* for zero-width elements, don't increment corpus position (think of "<s><np> ... </np></s>" for instance) */
                          if (zero_width_pattern)
                            target_vector[target_state] = cpos;  /* this is NOT the effective cpos, otherwise we'd go backwards! */
                          else
                            target_vector[target_state] = cpos + 1;
                        }

                      } /* if (transition_valid) ...           [check for winners] */

                    }   /* if (effective_cpos <= boundary) ... */

                  }     /* if (target_state != evalenv->dfa.E_State) ... */

                }       /* for (p = 0; ... ; p++) ...          [loop over all transitions from this state] */

              }         /* if (cpos >= 0) ...                  [active state] */

            }           /* for (state = 0; ... ; state++) ...  [loop over all states] */

            /* we have now traversed all possible transitions from all active states
             * -> if this is the first simulation cycle, we have now done our first transitions */
            first_transition_traversed = 1;

            /* if we haven't found a winner, or we're looking for other winners,
               check if there are still any active states */
            if (winner < 0 || evalenv->matching_strategy == longest_match) {
              running_states = 0;
              for (state = 0; state < evalenv->dfa.Max_States; state++)
                if (target_vector[state] >= 0)
                  running_states++;

              /* while we're at it, swap the current state & target vectors and reftabs for the next simulation cycle */
              help = state_vector;
              state_vector = target_vector;
              target_vector = help;

              help2 = reftab_vector;
              reftab_vector = reftab_target_vector;
              reftab_target_vector = help2;
            }

            /*
               Some nonsensical CQP queries involving XML tags or zero-width assertions (such as ``<s> *'') can lead to a FSA with
               eps-cycles (i.e. a cycle that returns to the same state without consuming a token) and hence to an infinite loop in the simulation.
               The hallmark of such an infinite loop should be that the state_vector converges to a stable state (a "fixed point").
               Here, we check for this situation (state_vector == target_vector) and abort FSA simulation if it is caught in an infinite loop.
               For now, an emphatic error message is printed (asking users to file a bug report if this code should abort a valid query) and
               query execution is stopped immediately.  Once we are reasonably certain that the infinite loop test works correctly, we might
               also simply consider setting the current run to "no match" and continue with the next starting point in the list.
            */
            for (state = 0; state < evalenv->dfa.Max_States; state++)
              if (state_vector[state] != target_vector[state])
                break;
            if (state >= evalenv->dfa.Max_States)
              infinite_loop++;   /* state vector unchanged: potential infinite loop */
            else
              infinite_loop = 0; /* state vector changed: reset infinite loop counter */

            if (infinite_loop >= 42) {
              /* state_vector hasn't changed for several simulation steps => caught in infinite loop
               * NB: An unchanged state vector is a valid (if unlikely) possibility if a Region element
               *     emits multiple states with the same end point (because the wait state is held back, too)
               *     an nothing else happens in the FSA. Therefore we now wait for several time steps until
               *     we break out of the infinite loop; this is extremely unlikely in a valid situation.
               * */
              cqpmessage(Error,
                         "Infinite loop detected: did you quantify over a zero-width element (XML tag or lookahead)?\n"
                         "\tIf you are reasonably sure that your query is valid, please contact the CWB development team and file a bug report!\n"
                         "\tQuery execution aborted.");
              running_states = 0; /* this should get us safely out of the inner loop ... */
              EvaluationIsRunning = 0; /* ... and the outer loop */
            }

          }    /* while (((winner < 0) || ... ) && (running_states > 0)) ...  */

          /* if we returned here, we either have a winning
           * corpus position or no running states any more. */
          if (simulate_debug)
            Rprintf("NFA sim terminated. Winner %d, running states %d\n", winner, running_states);

          /* queries like "</s>" will return empty matches -> ignore those (set to no match)
           * (NB this doesn't happen for open tags, since "<s>" == "<s> []") */
          if (winner >= 0 && winner >= winner_start) {
            if (*cut > 0)
              *cut = *cut - 1;
            matchlist->end[i] = winner;
            matchlist->start[i] = winner_start;
          }
          else
            matchlist->start[i] = -1;
        }

        if (matchlist->target_positions) {
          if (matchlist->start[i] >= 0)
            matchlist->target_positions[i] = my_target;
          else
            matchlist->target_positions[i] = -1;
        }
        if (matchlist->keyword_positions) {
          if (matchlist->start[i] >= 0)
            matchlist->keyword_positions[i] = my_keyword;
          else
            matchlist->keyword_positions[i] = -1;
        }

        i++;
        /* move to the next regarded matchlist element */

      } /* case 2: simulate automaton */

    }   /* while ((i < matchlist->tabsize) && ... ) ...  [simulate automaton for current matchlist] */

    /* if we left the execution prematurely, i.e. because !EvaluationIsRunning (due to interrupt or eval error) */
    while (i < matchlist->tabsize)
      matchlist->start[i++] = -1;

  }     /* end of the big "else" (unless evalenv->query_corpus->size == 0) */

  return has_match_selector; /* if a match selector was applied, results may need to be re-sorted */
}


static int
check_alignment_constraints(Matchlist *ml)
{
  int mlp, env_ix, i;
  int as, ae, dum1, dum2, dum3;
  EEP tmp;

  int *state_vector;
  int *target_vector;
  RefTab *reftab_vector;
  RefTab *reftab_target_vector;

  Matchlist matchlist;

  if (ee_ix > 0) {
    /*
     * we have alignments
     */
    EvaluationIsRunning = 1;

    init_matchlist(&matchlist);

    for (env_ix = 1; env_ix <= ee_ix; env_ix++) {
      assert(Environment[env_ix].aligned);
      tmp = evalenv;
      evalenv = &(Environment[env_ix]);

      state_vector  = (int *)cl_malloc(sizeof(int) * Environment[env_ix].dfa.Max_States);
      target_vector = (int *)cl_malloc(sizeof(int) * Environment[env_ix].dfa.Max_States);
      reftab_vector = (RefTab *) cl_malloc(sizeof(RefTab) * evalenv->dfa.Max_States);
      reftab_target_vector = (RefTab *) cl_malloc(sizeof(RefTab) * evalenv->dfa.Max_States);

      /* init reference table for current evalenv */
      for (i = 0; i < evalenv->dfa.Max_States; i++) {
        reftab_vector[i] = new_reftab(evalenv->labels);
        reftab_target_vector[i] = new_reftab(evalenv->labels);
      }

      for (mlp = 0; mlp < ml->tabsize; mlp++) {
        if (ml->start[mlp] != no_match) {
          int alg1, alg2;
          if (0 > (alg1 = cl_cpos2alg(Environment[env_ix].aligned, ml->start[mlp])))
            ml->start[mlp] = no_match;
          else if (!cl_alg2cpos(Environment[env_ix].aligned, alg1, &dum1, &dum2, &as, &dum3) || !cl_all_ok())
            ml->start[mlp] = no_match;
          else if (0 > (alg2 = cl_cpos2alg(Environment[env_ix].aligned, ml->end[mlp])))
            ml->start[mlp] = no_match;
          else if (!cl_alg2cpos(Environment[env_ix].aligned,
                                alg2, &dum1, &dum2, &dum3, &ae) || (cl_errno != CDA_OK))
            ml->start[mlp] = no_match;
          else if ((ae < as)  || (ae < 0) || (as < 0))
            ml->start[mlp] = no_match;
          else {
            /* construct initial matchlist by assuming
             * that every position in the aligned range
             * may be an initial matchpoint
             */
            matchlist.tabsize = ae - as + 1;
            matchlist.start = (int *)cl_malloc(sizeof(int) * matchlist.tabsize);
            matchlist.end   = (int *)cl_malloc(sizeof(int) * matchlist.tabsize);

            for (i = as; i <= ae; i++) {
              matchlist.start[i - as] = i;
              matchlist.end[i - as] = i;
            }

            dum1 = 1; /* run aligned query with cut 1; -> will be decremented and stop FSA simulation if match is found */

            /* don't reset label references here, because it shouldn't really be necessary (it's done in simulate() ) */
            simulate(&matchlist, &dum1,
                     state_vector, target_vector,
                     reftab_vector, reftab_target_vector,
                     -1);

            /* now dum1 == 0 if an aligned match was found, and dum1 == 1 if no match was found */
            if (dum1 != evalenv->negated)
              ml->start[mlp] = no_match;

            free_matchlist(&matchlist);
          }
        }
      }
      cl_free(state_vector);
      cl_free(target_vector);
      for (i = 0; i < evalenv->dfa.Max_States; i++) {
        delete_reftab(reftab_vector[i]);
        delete_reftab(reftab_target_vector[i]);
      }
      cl_free(reftab_vector);
      cl_free(reftab_target_vector);

      evalenv = tmp;
    }

    if (!EvaluationIsRunning) {
      cqpmessage(Info, "Evaluation interruted: results may be incomplete.");
      if (which_app == cqp)
        install_signal_handler();
    }
  }

  return 0;
}

/**
 * Run the DFA that performs a standard CQP query (i.e. based on token-level regular expression).

 *
 * @param envidx            Index into the global Environment array, specifiying which EvalEnvironment to use.
 *                          The EvalEnvironment provides, inter alia, the corpus to search and the DFA itself.
 * @param cut               The number specified as part of the "cut N" syntax. Any negative number = no limit.
 * @param keep_old_ranges   Boolean. Passed through to set_corpus_matchlists.
 */
static void
run_standard_query_dfa(int envidx, int cut, int keep_old_ranges)
{
  int p, maxresult, state, i, need_sort;
  Matchlist matchlist;
  Matchlist total_matchlist;

  int *state_vector;            /* currently active states are marked with corresponding cpos */
  int *target_vector;           /* target states when simulating transition */
  RefTab *reftab_vector;        /* the reference tables corresponding to the state vector */
  RefTab *reftab_target_vector; /* the reference tables corresponding to the target vector */

  int allocate_target_space, allocate_keyword_space;

  /* We can avoid wasting memory if the first transition of the FSA is
     deterministic, because there's no need to collect matches in total_matchlist then. */
  int FirstTransitionIsDeterministic;
  int trans_count = 0, current_transition = 0;

  assert(envidx <= ee_ix);        /* envidx == 0, actually ...  check_alignment_constraint EXPLICITLY assumes
                                   that everything else is an alignment constraint! */
  evalenv = &Environment[envidx];

  /* Apparently Max_Input is the maximal number of transitions that appears in the
     FSA, so we have Max_Input transitions from every state and the unused ones go
     to dfa.E_State (error state?) and aren't evaluated. To cut a long story short,
     we'll have to count how many transitions from the start state we'll have to
     follow. Heck. */
  /* this loop is essentially the same as the main loop below */
  for (p = 0; p < evalenv->dfa.Max_Input; p++)
    if (evalenv->dfa.TransTable[0][p] != evalenv->dfa.E_State)
      trans_count++;
  FirstTransitionIsDeterministic = (trans_count == 1);

  init_matchlist(&matchlist);
  if (!FirstTransitionIsDeterministic)
    init_matchlist(&total_matchlist);

  allocate_target_space = evalenv->has_target_indicator;
  allocate_keyword_space = evalenv->has_keyword_indicator;

  assert(evalenv->query_corpus);

  if (evalenv->dfa.Final[0]) {
    cqpmessage(Error, "Query matches empty string, evaluation aborted (otherwise whole corpus would be matched)\n");
    set_corpus_matchlists(evalenv->query_corpus,
                          &matchlist, /* total_matchlist may be uninitialised */
                          1,
                          0);
    free_matchlist(&matchlist);
  }
  else if (0 ==evalenv->query_corpus->size) {
    cqpmessage(Info, "Query corpus is empty (and so is the result).");
    free_matchlist(&matchlist);
  }
  else {
    /* allocate the state and reference table vectors here, so that this has
     * not to be done in every simulate iteration
     */
    state_vector = (int *)cl_malloc(sizeof(int) * evalenv->dfa.Max_States);
    target_vector = (int *)cl_malloc(sizeof(int) * evalenv->dfa.Max_States);
    reftab_vector = (RefTab *) cl_malloc(sizeof(RefTab) * evalenv->dfa.Max_States);
    reftab_target_vector = (RefTab *) cl_malloc(sizeof(RefTab) * evalenv->dfa.Max_States);
    /* init reference table for current evalenv */
    for (i = 0; i < evalenv->dfa.Max_States; i++) {
      reftab_vector[i] = new_reftab(evalenv->labels);
      reftab_target_vector[i] = new_reftab(evalenv->labels);
    }

    EvaluationIsRunning = 1;

    /* first transition loop (loops over all possible initial patterns) */
    for (p = 0; p < evalenv->dfa.Max_Input && EvaluationIsRunning; p++)
      if ((state = evalenv->dfa.TransTable[0][p]) != evalenv->dfa.E_State) {
        current_transition++;        /* counts how many of the trans_count initial transitions we have evaluated */
        if (progress_bar && !evalenv->aligned) {
          progress_bar_clear_line();
          progress_bar_message(current_transition, trans_count, "    preparing");
        }

        if (evalenv->labels) {
          for (i = 0; i < evalenv->dfa.Max_States; i++) {
            reset_reftab(reftab_vector[i]);        /* reset all label references */
            reset_reftab(reftab_target_vector[i]); /* shouldn't be necessary, just to make sure */
          }
        }

        /* match the initial pattern. */
        if (match_first_pattern(&(evalenv->patternlist[p]), &matchlist, evalenv->query_corpus)) {
          if (initial_matchlist_debug) {
            Rprintf("After initial matching for transition %d: ", p);
            show_matchlist_firstelements(matchlist);
            print_symbol_table(evalenv->labels);
          }

          if (matchlist.tabsize > 0) {
            matchlist.end = (int *)cl_malloc(sizeof(int) * matchlist.tabsize);
            memcpy(matchlist.end, matchlist.start, sizeof(int) * matchlist.tabsize);

            if (allocate_target_space) {
              matchlist.target_positions = (int *)cl_malloc(sizeof(int) * matchlist.tabsize);
              for (i = 0; i < matchlist.tabsize; i++)
                matchlist.target_positions[i] = -1;
            }
            if (allocate_keyword_space) {
              matchlist.keyword_positions = (int *)cl_malloc(sizeof(int) * matchlist.tabsize);
              for (i = 0; i < matchlist.tabsize; i++)
                matchlist.keyword_positions[i] = -1;
            }

            /* If 'cut <n>' is specified, try to get <n> matches from every initial pattern;
             * then reduce to a total of <n> matches after sorting (this happens in do_StandardQuery<parse_actions.c>).
             * Exception: aligned queries will remove an unpredictable number of matches below, so we must not apply a cut here and instead rely on the final reduction.
             * This means that aligned queries will always run to completion even if cut is specified, but the inefficiency can't be helped with the current design.
             */

            /* ee_ix > 0 iff there are alignment constraints */
            maxresult = (cut <= 0 || ee_ix > 0) ? -1 : cut;

            need_sort = simulate(&matchlist, &maxresult,
                                 state_vector, target_vector,
                                 reftab_vector, reftab_target_vector,
                                 p);

            if (initial_matchlist_debug) {
              Rprintf("After simulation for transition %d:\n ", p);
              show_matchlist(matchlist);
            }

            if (progress_bar && !evalenv->aligned)
              progress_bar_message(current_transition, trans_count, "merging reslt");

            /* reduce matchlist for this pass */
            apply_setop_to_matchlist(&matchlist, Reduce, NULL);

            /* re-sort the matchlist in canonical order if match selector was applied;
             * has to be performed after reduce so we don't waste a lot of time bubblesorting all
             * discarded items to the beginning of the matchlist */
            if (need_sort) {
              if (!sort_matchlist(&matchlist))
                EvaluationIsRunning = 0; /* indicates there was a problem */
              else
                /* remove duplicates created in this pass (might show up in final result with matching_strategy == traditional) */
                apply_setop_to_matchlist(&matchlist, Uniq, NULL);
            }

            /* collect the matches (unless there's only one transition) */
            if (!FirstTransitionIsDeterministic)
              apply_setop_to_matchlist(&total_matchlist, Union, &matchlist);

            if (initial_matchlist_debug && (!FirstTransitionIsDeterministic)) {
              Rprintf("Complete Matchlist after simulating transition %d: \n", p);
              show_matchlist(total_matchlist);
            }
          }
        }
        else if (EvaluationIsRunning) {
          cqpmessage(Error, "Problems while computing initial matchlist for pattern %d. Aborted.\n", p);
          EvaluationIsRunning = 0;
        }

        if (!FirstTransitionIsDeterministic)
          free_matchlist(&matchlist);

        if (progress_bar && !evalenv->aligned)
          progress_bar_clear_line();
      }        /* end of loop over all initial transitions */

    /* if there's only one transition, the total matchlist is the same as the matchlist
       of that transition, so we just copy it (note that we didn't initialize total_matchlist
       in that case */
    if (FirstTransitionIsDeterministic)
      total_matchlist = matchlist;

    if (initial_matchlist_debug) {
      Rprintf("After total simulation:\n");
      show_matchlist(total_matchlist);
    }

    if (!EvaluationIsRunning) {
      cqpmessage(Warning, "Evaluation interrupted: results will be incomplete.");
      if (which_app == cqp)
        install_signal_handler();
    }

    check_alignment_constraints(&total_matchlist);

    EvaluationIsRunning = 0;

    /* may need to reduce again after checking alignment constraints */
    apply_setop_to_matchlist(&total_matchlist, Reduce, NULL);

    if (initial_matchlist_debug) {
      Rprintf("after final reducing\n");
      show_matchlist(total_matchlist);
    }

    set_corpus_matchlists(evalenv->query_corpus, &total_matchlist, 1, keep_old_ranges);
    free_matchlist(&total_matchlist);

    cl_free(state_vector);
    cl_free(target_vector);
    for (i = 0; i < evalenv->dfa.Max_States; i++) {
      delete_reftab(reftab_vector[i]);
      delete_reftab(reftab_target_vector[i]);
    }
    cl_free(reftab_vector);
    cl_free(reftab_target_vector);
  }
}



/**
 * Does a "meet" operation within MU queries.
 *
 * NB: list1 will be modified in place, list2 remains unchanged and should be deallocated by the caller

 * @param  negated   Boolean, true iff the "meet" operation contains the negation operator "not".
 */
static int
meet_mu(Matchlist *list1, Matchlist *list2, int lw, int rw, Attribute *struc, int negated)
{
  int start, end, found_match;
  int corpus_size = evalenv->query_corpus->mother_size; /* corpus size needed for boundary checks below */

  if (list1->tabsize == 0 || list2->tabsize == 0) {
    /* If one of the two lists is empty, so is their intersection and we're done. */
    cl_free(list1->start);
    cl_free(list1->end);
    list1->tabsize = 0;
    list1->matches_whole_corpus = 0;
  }
  else {
    /* Implementation modified to give consistent "filtering" semantics (SE, 2017-07-01)
     *  - result of (meet A B <win>) are those items of A for which at least one item of B occurs within <win>
     *  - the same item of B can satisfy this constraint for multiple items of A, which is achieved simply by not "consuming" this item
     *  - because both match lists are ordered, the filter can be applied efficiently in a single forward pass
     *  - new consistent behaviour is now documented in the CQP Query Language Tutorial
     */

    /* since we're filtering list1, we can simply upcopy items that pass the filter */
    int i = 0;                /* index in list1 */
    int j = 0;                /* index in list2 */
    int k = 0;                /* insertion point in list1 as result list */

    while ((i < list1->tabsize) && (j < list2->tabsize)) {
      /* check whether this item from A can be matched against an item from B in the window [start, end] */
      if (struc) {
        /* s-attribute context: find region containing current point in A, otherwise there can be no match here */
        if (!cl_cpos2struc2cpos(struc, list1->start[i], &start, &end) || !cl_all_ok()) {
          if (negated)
            list1->start[k++] = list1->start[i++]; /* negated constraint: upcopy match to insertion point within A */
          else
            i++; /* discard current point in A */
          continue;
        }
      }
      else {
        /* numeric context: compute start and end as offsets from current corpus position in A
         *  - in principle, no boundary checks would be needed because we will only compare valid positions (from B) with the window
         *  - however, we need to check for integer overflow in case we are dealing with a very large corpus
         *    (or with an idiot who thinks it's funny to specify a context window of -2147483647 +2147483647 in his query)
         *  - we need to distinguish between two cases: whether the boundary specifies a (i) minimum or (ii) a maximum distance
         *  - case (i) occurs for a start offset lw > 0 (e.g. 2 5) and for an end offset rw < 0 (e.g. -4 -2)
         *  - if a minimum distance boundary (i) falls outside the corpus, we cannot possibly find a match -> skip and continue
         *  - a maximum distance boundary (ii) can simply be clamped to the range of valid corpus positions
         *  - because of these case distinctions, the boundary checks are fairly expensive, but cannot be avoided without 64bit ints
         */
        start = cl_cpos_offset(list1->start[i], lw, corpus_size, lw <= 0); /* clamp to corpus if lw specifies a maximum distance */
        end   = cl_cpos_offset(list1->start[i], rw, corpus_size, rw >= 0); /* clamp if rw specifies a maximum distance */

        /* if a minimum distance is outside the corpus, there can be no match here */
        if (start < 0 || end < 0) { /* i.e. error code CDA_EPOSORNG has been returned */
          if (negated)
            list1->start[k++] = list1->start[i++]; /* negated constraint: upcopy match to insertion point within A */
          else
            i++; /* discard current point in A */
          continue;
        }
      }

      /* [start, end] is now a valid cpos range (which may be empty for end < start) and we try to find an item from B in this window */
      /* skip items of B before start of current context window */
      while (j < list2->tabsize && list2->start[j] < start)
        j++;
      /* note that we never have to move backwards in list2 because the context windows will be strictly increasing */

      /* now check for a match within the context window unless we have already reached the end of B */

      if (j < list2->tabsize && list2->start[j] <= end) {
        /* verify that this is a valid match, as it should be */
        assert((start <= list2->start[j]) && (list2->start[j] <= end));
        found_match = 1;
      }
      else
        found_match = 0;

      /* if we found a match... or, in negated mode, if we DIDN'T find one ... */
//      if ( (found_match && !negated) || (!found_match && negated) ) {
      if ( found_match == (negated ? 0 : 1) )
        /* ... upcopy match to insertion point within A */
        list1->start[k++] = list1->start[i++];
      else
        /* discard current point in A */
        i++;

      assert(k <= list1->tabsize && k <= i);      /* make sure that the upcopy works correctly */
    }
    /* end of loop filtering A against B */

    if (k == 0)
      /* the result is empty, so free list1 */
      cl_free(list1->start);

    else if (k < list1->tabsize)
      /* reallocate vector if the number of matches has been reduced */
      list1->start = (int *)cl_realloc(list1->start, sizeof(int) * k);

    list1->tabsize = k;
    list1->matches_whole_corpus = 0; /* should already be the case */
  }
  /* end of case where neither of the two matchlists is empty */

  return 1;
}



/** return is boolean (all OK?  true => yes, false => no) */
static int
eval_mu_tree(Evaltree et, Matchlist* ml)
{
  Matchlist arg2;
  int ok;

  if (!et)
    return 0;
  if (!EvaluationIsRunning)
    return 0; /* so user can interrupt query */

  if (et->type == meet_union) {
    switch (et->cooc.op_id) {

    case cooc_meet:
      init_matchlist(&arg2);
      if (! eval_mu_tree(et->cooc.left, ml))
        return 0;
      if (! eval_mu_tree(et->cooc.right, &arg2)) {
        free_matchlist(&arg2);
        return 0;
      }
      ok = meet_mu(ml, &arg2, et->cooc.lw, et->cooc.rw, et->cooc.struc, et->cooc.negated);
      free_matchlist(&arg2);
      return ok;

    case cooc_union:
      init_matchlist(&arg2);
      if (! eval_mu_tree(et->cooc.left, ml))
        return 0;
      if (! eval_mu_tree(et->cooc.right, &arg2)) {
        free_matchlist(&arg2);
        return 0;
      }
      apply_setop_to_matchlist(ml, Union, &arg2);
      free_matchlist(&arg2);
      return 1;

    default:
      assert("Illegal node type in cooc" && 0);
      break;
    }
  }

  else if (et->type == leaf) {
    assert(CurEnv);
    ok = calculate_initial_matchlist(evalenv->patternlist[et->leaf.patindex].con.constraint, ml, evalenv->query_corpus);
    return ok && EvaluationIsRunning; /* aborts evaluation on user interrupt */
  }

  assert("Illegal node type in MU Evaluation Tree" && 0);
  return 0;
}




/**
 * Run a standard CQP query.
 *
 * This function wraps round run_standard_query_dfa (the only other thing it does is enforce the hard_cut limit).
 *
 * @see   hard_cut
 * @see   run_standard_query_dfa
 * @param cut                    Integer value for "cut" command, clamped to hard_cut if one is set.
 * @param keep_old_ranges        Boolean, passed to equivalent parameter to run_standard_query_dfa()
 */
void
cqp_run_query(int cut, int keep_old_ranges)
{
  /* can't run a query if we don't have an Environment! */
  if (ee_ix >= 0) {
    if (hard_cut > 0)
      if (hard_cut < cut)
        cut = hard_cut;
    run_standard_query_dfa(0, cut, keep_old_ranges);
  }
}



/**
 * Run a CQP Meet-Union query.
 *
 * @param keep_old_ranges   Boolean
 * @param cut_value
 */
void
cqp_run_mu_query(int keep_old_ranges, int cut_value)
{
  Matchlist matchlist;

  init_matchlist(&matchlist);
  evalenv = &Environment[0];
  assert(evalenv->query_corpus);

  EvaluationIsRunning = 1;
  if (!eval_mu_tree(evalenv->evaltree, &matchlist)) {
    cqpmessage(Error, "Evaluation of MU query has failed (or been interrupted by user)");
    free_matchlist(&matchlist);        /* automatically initialises to empty match list */
  }

  if (matchlist.tabsize > 0) {
    mark_offrange_cells(&matchlist, evalenv->query_corpus);
    apply_setop_to_matchlist(&matchlist, Reduce, NULL);

    if (cut_value > 0 && matchlist.tabsize > cut_value) {
      int i;
      for (i = cut_value; i < matchlist.tabsize; i++)
        matchlist.start[i] = -1;
      apply_setop_to_matchlist(&matchlist, Reduce, NULL);
    }

    matchlist.end = (int *)cl_malloc(sizeof(int) * matchlist.tabsize);
    memcpy(matchlist.end, matchlist.start, sizeof(int) * matchlist.tabsize);
  }
  else
    assert(matchlist.start == NULL);

  set_corpus_matchlists(evalenv->query_corpus, &matchlist, 1, keep_old_ranges);

  /* the above call empties out the matchlist, but does not free the local variable's memory */
  free_matchlist(&matchlist);
}


/**
 * Run a CQP Tabular query.
 */
void
cqp_run_tab_query()
{
  Evaltree col;
  int nr_columns, i, this_col, smallest_col;
  int n_res, max_res, running;

  Matchlist *lists, result;
  int *positions;
  Evaltree *constraints;

  /* ------------------------------------------------------------ */

  evalenv = &Environment[0];

  assert(evalenv->query_corpus);

  nr_columns = 0;
  for (col = evalenv->evaltree; col; col = col->tab_el.next) {
    assert(col->type == tabular);
    if (evalenv->patternlist[col->tab_el.patindex].type != Pattern) {
      cqpmessage(Error, "matchall [] (or another token pattern matching the entire corpus) is not allowed in TAB query (column #%d)\n", nr_columns + 1);
      init_matchlist(&result);
      set_corpus_matchlists(evalenv->query_corpus, &result, 1, 0); /* return empty result set */
      return;
    }
    nr_columns++;
  }
  assert(nr_columns > 0);

  /* allocate matchlists for all TAB columns, a vector of list offsets, and a list of constraint trees */
  lists = (Matchlist *)cl_calloc(nr_columns, sizeof(Matchlist));
  positions = (int *)cl_calloc(nr_columns, sizeof(int));
  constraints = (Evaltree *)cl_calloc(nr_columns, sizeof(Evaltree));

  EvaluationIsRunning = 1; /* so user can interrupt the TAB query, and because calculate_initial_matchlist() fails to evaluate & operator otherwise */
  running = 1; /* whether we're still evaluating */

  /* compute matchlists for all column constraints in the TAB query */
  i = 0;
  smallest_col = 0;
  for (col = evalenv->evaltree; col; col = col->tab_el.next) {
    constraints[i] = col;
    init_matchlist(&lists[i]);
    if (!EvaluationIsRunning)
      running = 0;
    if (running) {
      if (!calculate_initial_matchlist(evalenv->patternlist[col->tab_el.patindex].con.constraint, &lists[i], evalenv->query_corpus)) {
        free_matchlist(&lists[i]); /* discard this matchlist in case of error or user abort */
        running = 0;
      }
    }
    /* useful for debugging:
     * Rprintf("TAB pattern #%d: %d hits %s %s\n", i + 1, lists[i].tabsize,
     *   (lists[i].is_inverted) ? "(inverted)" : "", (lists[i].matches_whole_corpus) ? "(whole corpus)" : "");
     */

    if (lists[smallest_col].tabsize > lists[i].tabsize)
      smallest_col = i;
    i++;
  }
  max_res = lists[smallest_col].tabsize; /* = 0 if there was an error or user abort */

  init_matchlist(&result);

  if (max_res > 0) {
    /*
     * A simple greedy algorithm:
     *  - for each start position (column 0)
     *  - find nearest item from next column within distance range
     *  - if successful, fix this item and proceed to next column
     *  - if greedy algorithm doesn't find a match, the start position is discarded (even if it might match with different assignment)
     *  - the original algorithm used to "consume" the items from all columns that participate in a match (so they are no longer available for subsequent matches);
     *    this produces inconsistent results that are hard to predict, similar to the original implementation of MU queries
     *    (consider e.g. the sequence A1 .. A2 .. B1 .. C1 .. B2 .. C2: the algorithm would match A1-B1-C1 and A2-B2-C2, but without A1 it would match A2-B1-C1)
     *  - new algorithm does not to consume items from columns >= 1, so it now predictably matches A1-B1-C1 and A2-B1-C1
     *  - similar to the standard matching strategy of regular queries, nested matches are discarded, returning only the "early match" A1-B1-C1 in the example
     *  - as a consequence, each item from any column cannot participate in more than one of the final matches and the result set is bounded by the shortest column
     *  - TAB queries do not respect the matching strategy setting because this makes little sense without implementing a complete combinatorial search
     */

    /* allocate result matchlist (for up to max_res matches) */
    result.start = (int *)cl_malloc(sizeof(int) * max_res);
    result.end = (int *)cl_malloc(sizeof(int) * max_res);
    n_res = 0; /* also serves as pointer into the result matchlist */

    while (positions[0] < lists[0].tabsize && running) {
      int next_col, this_pos, next_pos, l_pos, r_pos, boundary;
      /* The original implementation of TAB queries completely ignored the optional "within" constraint (which defaults to hard_boundary tokens).
       * In order to evaluate the query correctly, we must first determine a right boundary for the complete TAB match, which will also be used
       * to cut off unlimited distances (repeat_inf) between TAB columns.
       */
      if (0 > (boundary = calculate_rightboundary(evalenv->query_corpus, lists[0].start[positions[0]], evalenv->search_context))) {
        /* can't get a match here (because of a "within <s-att>" constraint);
         * note that we cannot rely on falling through in the first iteration of the for loop in case there is just a single column
         * (e.g. "TAB [] within head;" would fail to apply the "within" constraint)
         */
        positions[0]++;
        continue;
      }

      /* iterate over pairs of adjacent columns, scanning for a greedy match within specified distance */
      next_pos = -1;
      for (next_col = 1; next_col < nr_columns; next_col++) {
        this_col = next_col - 1;

        /* offset in matchlist for current column */
        this_pos = lists[this_col].start[positions[this_col]];

        /* valid range for a matching cpos in the next column */
        l_pos = cl_cpos_offset(this_pos, constraints[next_col]->tab_el.min_dist, boundary + 1, 0); /* NB: set virtual corpus size to boundary + 1 */
        if (l_pos < 0)
          break; /* beyond search boundary, no match possible */

        if (repeat_inf == constraints[next_col]->tab_el.max_dist)
          r_pos = cl_cpos_offset(this_pos, hard_boundary, boundary + 1, 1);
        else
          r_pos = cl_cpos_offset(this_pos, constraints[next_col]->tab_el.max_dist, boundary + 1, 1);

        /* scan next column for a potential match (with cpos >= l_pos) */
        while (positions[next_col] < lists[next_col].tabsize && (next_pos = lists[next_col].start[positions[next_col]]) < l_pos)
          positions[next_col]++;

        /* no potential match found or not in range (i.e. !(next_pos <= r_pos)) */
        if (positions[next_col] >= lists[next_col].tabsize || next_pos > r_pos)
          break;
      }

      if (next_col >= nr_columns) {
        /* we have found a greedy match: copy it to the result matchlist */
        l_pos = lists[0].start[positions[0]]; /* start of match = cpos of first column */
        r_pos = lists[nr_columns - 1].start[positions[nr_columns - 1]]; /* end of match = cpos of last column */
        /* discard nested matches; the only possible case is that r_pos == result.end[n_res - 1] */
        if (n_res == 0 || r_pos > result.end[n_res - 1]) {
          assert(n_res < max_res);
          result.start[n_res] = l_pos;
          result.end[n_res] = r_pos;
          n_res++;
        }
      }

      positions[0]++;
      if (!EvaluationIsRunning)
        running = 0;
    }

    /* finalize the result matchlist */
    if (n_res > 0 && running) {
      if (n_res < max_res) {
        /* shorten vectors if necessary */
        result.start = (int *)cl_realloc(result.start, sizeof(int) * n_res);
        result.end = (int *)cl_realloc(result.end, sizeof(int) * n_res);
      }
      result.tabsize = n_res;

      /* delete offrange cells if we are in a subcorpus */
      if (mark_offrange_cells(&result, evalenv->query_corpus) > 0)
        apply_setop_to_matchlist(&result, Reduce, NULL);
    }
    else {
      /* no matches: return empty matchlist */
      cl_free(result.start);
      cl_free(result.end);
      result.tabsize = 0;
    }

  } /* otherwise max_res == 0 and result has already been initialized as an empty matchlist */

  if (!running)
    cqpmessage(Error, "Evaluation of TAB query has failed (or been interrupted by user)");

  set_corpus_matchlists(evalenv->query_corpus, &result, 1, 0);

  /* cleanup */
  cl_free(positions);
  cl_free(constraints);
  for (i = 0; i < nr_columns; i++)
    free_matchlist(&lists[i]);
  cl_free(lists);
  free_matchlist(&result);
}



/* ---------------------------------Functions for working with the global array of EvalEnvironments. */

/**
 * Sets up a new environment in the global array, which then becomes the
 *
 * (better name would be "add_environment" or "push_new_environment")
 *
 * The next slot upwards is used (and ee_ix is incremented).
 *
 * @see     ee_ix
 * @see     Environment
 * @return  True for all OK, false for an error (overflow of MAXENVIRONMENT).
 */
int
next_environment(void)
{
  if (ee_ix >= MAXENVIRONMENT) {
    Rprintf("No more environments for evaluation (max %d exceeded)\n", MAXENVIRONMENT);
    return 0; /* set a cqp error value? */
  }

  ee_ix++;

  Environment[ee_ix].query_corpus   = NULL;
  Environment[ee_ix].labels         = new_symbol_table();

  Environment[ee_ix].MaxPatIndex    = -1;

  Environment[ee_ix].gconstraint    = NULL;
  Environment[ee_ix].evaltree       = NULL;

  Environment[ee_ix].has_target_indicator = 0;
  Environment[ee_ix].target_label   = NULL;
  Environment[ee_ix].has_keyword_indicator = 0;
  Environment[ee_ix].keyword_label  = NULL;

  Environment[ee_ix].match_label    = NULL;
  Environment[ee_ix].matchend_label = NULL;

  init_dfa(&Environment[ee_ix].dfa);

  Environment[ee_ix].search_context.direction = ctxtdir_leftright;
  Environment[ee_ix].search_context.space_type = word;
  Environment[ee_ix].search_context.attrib = NULL;
  Environment[ee_ix].search_context.size = 0;
  Environment[ee_ix].negated = 0;
  Environment[ee_ix].matching_strategy = matching_strategy; /* initialize from current global setting */

  CurEnv = &Environment[ee_ix];

  return 1;
}


/**
 * Frees an evaluation environment.
 *
 * The environment must be one currently occupied within the global array.
 *
 * @see            Environment
 * @see            eep
 * @param thisenv  The eval environment to free (index in global array).
 * @return         Boolean: true if the deletion went OK;
 *                 false if the environment to be freed was
 *                 not occupied (will print an error message).
 */
int
free_environment(int thisenv)
{
  int i;

  if (thisenv < 0 || thisenv > ee_ix) {
    Rprintf("Environment %d is not occupied\n", thisenv);
    return 0;
  }
  else {
    Environment[thisenv].query_corpus = NULL;
    delete_symbol_table(Environment[thisenv].labels);
    Environment[thisenv].labels = NULL;

    for (i = 0; i <= Environment[thisenv].MaxPatIndex; i++) {
      switch (Environment[thisenv].patternlist[i].type) {

      case Pattern:
        free_booltree(Environment[thisenv].patternlist[i].con.constraint);
        Environment[thisenv].patternlist[i].con.constraint = NULL;
        Environment[thisenv].patternlist[i].con.label = NULL;
        Environment[thisenv].patternlist[i].con.is_target = IsNotTarget;
        Environment[thisenv].patternlist[i].con.lookahead = False;
        break;

      case Tag:
        Environment[thisenv].patternlist[i].tag.attr = NULL;
        Environment[thisenv].patternlist[i].tag.right_boundary = NULL;
        cl_free(Environment[thisenv].patternlist[i].tag.constraint);
        Environment[thisenv].patternlist[i].tag.flags = 0;
        cl_delete_regex(Environment[thisenv].patternlist[i].tag.rx);
        Environment[thisenv].patternlist[i].tag.rx = NULL;
        break;

      case Anchor:
        Environment[thisenv].patternlist[i].anchor.field = NoField;
        break;

      case MatchAll:
        Environment[thisenv].patternlist[i].matchall.label = NULL;
        Environment[thisenv].patternlist[i].matchall.is_target = IsNotTarget;
        Environment[thisenv].patternlist[i].matchall.lookahead = False;
        break;

      case Region:
        if (Environment[thisenv].patternlist[i].region.opcode == AVSRegionEnter)
          StateQueue_delete(&(Environment[thisenv].patternlist[i].region.queue)); /* weak reference in other Hold and Emit ops */
        Environment[thisenv].patternlist[i].region.opcode = 0;
        cl_free(Environment[thisenv].patternlist[i].region.name);
        Environment[thisenv].patternlist[i].region.attr = NULL;
        Environment[thisenv].patternlist[i].region.nqr = NULL;
        Environment[thisenv].patternlist[i].region.start_label = NULL;
        Environment[thisenv].patternlist[i].region.start_target = IsNotTarget;
        Environment[thisenv].patternlist[i].region.end_label = NULL;
        Environment[thisenv].patternlist[i].region.end_target = IsNotTarget;
        break;

      default:
        assert("Illegal AVS type in pattern list of ee" && 0);
        break;
      }
    }

    Environment[thisenv].MaxPatIndex = -1;

    free_booltree(Environment[thisenv].gconstraint);
    Environment[thisenv].gconstraint = NULL;
    free_evaltree(&Environment[thisenv].evaltree);
    free_dfa(&Environment[thisenv].dfa);

    Environment[thisenv].search_context.direction = ctxtdir_leftright;
    Environment[thisenv].search_context.space_type = word;
    Environment[thisenv].search_context.attrib = NULL;
    Environment[thisenv].search_context.size = 0;
    Environment[thisenv].has_target_indicator = 0;

    Environment[thisenv].match_selector.begin        = NULL;
    Environment[thisenv].match_selector.begin_offset = 0;
    Environment[thisenv].match_selector.end          = NULL;
    Environment[thisenv].match_selector.end_offset   = 0;

    return 1;
  }
}

/**
 * Prints the contents of an EvalEnvironment object to STDOUT.
 *
 * Which bits of information are printed depends on which of a group of
 * debugging-variables are set to true.
 *
 * The EvalEnvironment to print is specified as an index into the global
 * array (Environment).
 *
 * @see Environment
 * @param thisenv  Index into Environment indicating which EvalEnvironment
 *                 should be displayed.
 */
void
show_environment(int thisenv)
{
  if (thisenv < 0 || thisenv > ee_ix)
    Rprintf("Environment %d not used\n", thisenv);

  else if (show_compdfa || show_evaltree || show_gconstraints || show_patlist) {
    /* Note, at least one of the above debugging-variables must be true, or there is nothing to print! */

    Rprintf("\n ================= ENVIRONMENT #%d ===============\n\n", thisenv);
    Rprintf("Has %starget indicator.\n", Environment[thisenv].has_target_indicator ? "" : "no ");
    Rprintf("Has %skeyword indicator.\n", Environment[thisenv].has_keyword_indicator ? "" : "no ");

    if (show_compdfa) {
      Rprintf("\n==================== DFA:\n\n");
      show_complete_dfa(Environment[thisenv].dfa);
    }

    if (show_evaltree) {
      Rprintf("\n==================== Evaluation Tree:\n\n");
      print_evaltree(thisenv, Environment[thisenv].evaltree, 0);
    }

    if (show_gconstraints) {
      Rprintf("\n==================== Global Constraints:\n\n");
      print_booltree(Environment[thisenv].gconstraint, 0);
    }

    if (show_patlist)
      show_patternlist(thisenv);

    if (Environment[thisenv].match_selector.begin ||
        Environment[thisenv].match_selector.begin_offset ||
        Environment[thisenv].match_selector.end ||
        Environment[thisenv].match_selector.end_offset) {
      Rprintf("\n==================== Match Selector:\n\n%s[%d] ... %s[%d]\n",
             Environment[thisenv].match_selector.begin ? Environment[thisenv].match_selector.begin->name : "match",
             Environment[thisenv].match_selector.begin_offset,
             Environment[thisenv].match_selector.end? Environment[thisenv].match_selector.end->name : "matchend",
             Environment[thisenv].match_selector.end_offset);
    }

    Rprintf("\n ================= END ENVIRONMENT #%d =============\n", thisenv);
#ifndef R_PACKAGE
    fflush(stdout);
#endif
  }
}

/**
 * Frees all eval environments in the global array, and sets the ee_ix index to -1
 */
void
free_all_environments(void)
{
  int i;
  for (i = 0; i <= ee_ix; i++) {
    if (!free_environment(i)) {
      Rprintf("Problems while free()ing environment %d\n", i);
      break;
    }
  }
  ee_ix = -1;
}
