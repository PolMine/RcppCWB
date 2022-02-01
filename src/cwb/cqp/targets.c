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
#include <math.h>

#include "../cl/cl.h"

#include "../cl/ui-helpers.h"

#include "corpmanag.h"

#include "eval.h"
/* NB  the definition of eval_bool was changed to improve label and target handling
   all instances of eval_bool() in this file now pass NULL, i.e. a dummy reference table,
   as second argument */

#include "options.h"
#include "output.h"
#include "ranges.h"
#include "cqp.h"

#include "targets.h"

SearchStrategy
string_to_strategy(const char *s)
{
  if (!s)
    return SearchNone;

  else if (cl_streq_ci(s, "leftmost"))
    return SearchLeftmost;

  else if (cl_streq_ci(s, "rightmost"))
    return SearchRightmost;

  else if (cl_streq_ci(s, "nearest"))
    return SearchNearest;

  else if (cl_streq_ci(s, "farthest"))
    return SearchFarthest;

  else {
    cqpmessage(Warning, "Illegal search strategy specification ``%s''", s);
    return SearchNone;
  }
}


/* Handling of target, match, keyword. Tue Feb 28 16:02:03 1995 (oli) */

/* target can be any field except NoField (-> CQP dies),
   source can be NoField, which deletes the target field (unless that's match or matchend) */
/**
 * Sets one of the anchors (aka fields) of the NQR/subcorpus to the value of one of the others, optionally adding an token offset.
 * It implements the CQP commands
 *   set NQR <anchor> <anchor>[<offset>];
 *   set NQR <anchor> NULL;
 *
 * There are several special cases to be considered:
 *  - if source anchor has an undefined value (-1), the destination will either be unchanged (overwrite=0) or become undefined, too (overwrite=1)
 *  - if source anchor's offset puts it outside the valid cpos range, it is treated as undefined (and the rule above applies)
 *  - if destination is MatchField or MatchendField, the matching range is modified and may become invalid
 *    (either because the destination anchor is set to undefined or because matchend ends up to the left of match);
 *    in this case, the modification is either ignored (overwrite=0) or the match is discarded from the query (overwrite=1)
 *
 * @param corp          CorpusList entry for the subcorpus or NQR to modify.
 * @param dest          Which anchor to modify (MatchField, MatchEndField, TargetField, KeywordField).
 *                      If this is MatchField or MatchendField, the modified query will be re-sorted and some matches may be discarded.
 *                      If this is NoField, CQP dies, since that's a logic error.
 * @param source        Which anchor to get the data from. If NoField, the destination TargetField or KeywordField is deleted.
 * @param source_offset Token offset to add to the source anchor(to shift anchor to the left or right).
 * @param overwrite     Whether undefined or invalid source anchor should overwrite destination or be ignored (see above).
 *                      In other words, overwrite=0 performs a conditional update while overwrite=1 performs a forced update.
 *
 * @return              True if successful, False (with a CQP error message) otherwise.
 */
int
set_target(CorpusList *corp, FieldType dest, FieldType source, int source_offset, int overwrite)
{
  int i, cpos, size, n_tokens;
  cpos = -1;

  assert(dest != NoField);
  if (dest == source && source_offset == 0) {
    cqpmessage(Warning, "Anchors are identical, nothing to be done.");
    return 1;
  }

  size = corp->size;
  if (size == 0) {
    cqpmessage(Warning, "Query result is empty, nothing to be done.");
    return 1;
  }
  assert(corp->range);

  n_tokens = corp->mother_size; /* for boundary checks if source_offset != 0 */

  /* set NQR (keyword|target) NULL; --> delete keyword or target anchor */
  if (source == NoField) {
    if (dest == MatchField || dest == MatchEndField) {
      cqpmessage(Error, "Can't delete match or matchend anchor from %s\n", corp->name);
      return 0;
    }
    if (dest == TargetField)
      cl_free(corp->targets);
    else if (dest == KeywordField)
      cl_free(corp->keywords);
    else
      assert(0 && "Can't be");
  }
  /* set NQR <anchor> <anchor>[<offset>]; --> update destination anchor from source */
  else {
    if (source == TargetField && !corp->targets) {
      cqpmessage(Error, "No target anchors defined for %s\n", corp->name);
      return 0;
    }
    if (source == KeywordField && !corp->keywords) {
      cqpmessage(Error, "No keyword anchors defined for %s\n", corp->name);
      return 0;
    }
    /* having handled all possible error conditions, we can now safely allocate anchor vectors if needed */
    if (dest == TargetField && !corp->targets) {
      corp->targets = (int *)cl_malloc(size * sizeof(int));
      for (i = 0; i < size; i++)
        corp->targets[i] = -1; /* initialise to undefined values */
    }
    if (dest == KeywordField && !corp->keywords) {
      corp->keywords = (int *)cl_malloc(size * sizeof(int));
      for (i = 0; i < size; i++)
        corp->keywords[i] = -1;
    }

    /* In a textbook case of premature optimisation, the previous implementation had 16 different versions of the loop below,
     * for each possible combination of source and destination anchor. This was done to avoid case distinctions in the inner
     * loop and even replace it with a memcopy() in some cases. The optimisation was a completely unnecessary complication of
     * the source code because
     * 1) the set target command is invoked only occasionally, and even with a suboptimal implementation it will be much faster
     *    than the original query, requiring only a single linear pass through the result set;
     * 2) more importantly, if-clauses within the inner loop always take the same branch (based on source and dest), which the
     *    CPU's branch prediction will learn quickly, reducing the overhead from the case distinctions to be quite irrelevant.
     * Hence, the new implementation uses a single outer loop over the result set and handles the different source and destination
     * anchors within the loop, thus avoiding any code duplication and making the algorithm much more transparent.
     */
    for (i = 0; i < size; i++) {
      /* get the source anchor for this match */
      if (source == MatchField)
        cpos = corp->range[i].start;
      else if (source == MatchEndField)
        cpos = corp->range[i].end;
      else if (source == TargetField)
        cpos = corp->targets[i];
      else if (source == KeywordField)
        cpos = corp->keywords[i];
      else
        assert(0 && "Can't be");

      /* add the offset and check for range overflow */
      if (source_offset != 0 && cpos >= 0) {
        cpos += source_offset;
        if (cpos < 0 || cpos >= n_tokens)
          cpos = -1; /* if the anchor is shifted outside the corpus, it becomes undefined */
      }

      /* assign to target or keyword */
      if (dest == TargetField) {
        if (overwrite || cpos >= 0)
          corp->targets[i] = cpos;
      }
      else if (dest == KeywordField) {
        if (overwrite || cpos >= 0)
          corp->keywords[i] = cpos;
      }
      else if (dest == MatchField) {
        if (cpos >= 0 && cpos <= corp->range[i].end)
          corp->range[i].start = cpos;
        else if (overwrite)
          corp->range[i].start = -1; /* mark for deletion if overwriting makes match invalid */
      }
      else if (dest == MatchEndField) {
        if (cpos >= 0 && cpos >= corp->range[i].start)
          corp->range[i].end = cpos;
        else if (overwrite)
          corp->range[i].start = -1; /* note that we have to set the match anchor to -1 to mark for deletion */
      }
      else
        assert(0 && "Can't be");
    }

    /* re-sort corpus and discard invalid matching spans if match or matchend anchor was modified */
    if (dest == MatchField || dest == MatchEndField) {
      apply_range_set_operation(corp, RReduce, NULL, NULL);
      RangeSort(corp, 0);
    }
  }

  touch_corpus(corp);
  return 1;
}

int
evaluate_target(CorpusList *corp,          /* the corpus */
                FieldType t_id,            /* the field to set */
                FieldType base,            /* where to start the search */
                int inclusive,             /* including or excluding the base */
                SearchStrategy strategy,   /* disambiguation rule: which item */
                Constrainttree constr,     /* the constraint */
                enum ctxtdir direction,    /* context direction */
                int units,                 /* number of units */
                char *attr_name)           /* name of unit */
{
  Attribute *attr;
  int *table;
  Context context;
  int i, line, lbound, rbound;
  int excl_start, excl_end;
  int nr_evals;
  int percentage, new_percentage; /* for ProgressBar */

  /* ------------------------------------------------------------ */

  assert(corp);

  /* consistency check */
  assert(t_id == TargetField || t_id == KeywordField || t_id == MatchField || t_id == MatchEndField);

  if (!constr) {
    cqpmessage(Error, "Constraint pattern missing in 'set target' command.");
    return 0;
  }

  if (corp->size <= 0) {
    cqpmessage(Error, "Corpus is empty.");
    return 0;
  }

  /* check whether the base field specification is ok */
  switch(base) {

  case MatchField:
  case MatchEndField:
    if (corp->range == NULL) {
      cqpmessage(Error, "No ranges for start of search");
      return 0;
    }
    break;

  case TargetField:
    if (corp->targets == NULL) {
      cqpmessage(Error, "Can't start from base TARGET, none defined");
      return 0;
    }
    break;

  case KeywordField:
    if (corp->keywords == NULL) {
      cqpmessage(Error, "Can't start from base KEYWORD, none defined");
      return 0;
    }
    break;

  default:
    cqpmessage(Error, "Illegal base field (#%d) in 'set target' command.", base);
    return 0;
  }

  if (units <= 0) {
    cqpmessage(Error, "Invalid search space (%d units) in 'set target' command.", units);
    return 0;
  }

  /* THIS SHOULD BE UNNECESSARY, BECAUSE THE GRAMMAR MAKES SURE THE SUBCORPUS EXISTS & IS LOADED */
  /*   if (!access_corpus(corp)) { */
  /*     cqpmessage(Error, "Can't access named query %s.", corp->name); */
  /*     return 0; */
  /*   } */

  context.size = units;
  context.direction = direction;

  if (cl_streq_ci(attr_name, "word") || cl_streq_ci(attr_name, "words")) {
    attr = cl_new_attribute(corp->corpus, CWB_DEFAULT_ATT_NAME, ATT_POS);
    context.space_type = word;
    context.attrib = NULL;
  }
  else {
    attr = cl_new_attribute(corp->corpus, attr_name, ATT_STRUC);
    context.space_type = structure;
    context.attrib = attr;
  }

  if (attr == NULL) {
    cqpmessage(Error, "Can't find attribute %s.%s", corp->mother_name, attr_name);
    return 0;
  }

  if (progress_bar) {
    progress_bar_clear_line();
    progress_bar_message(1, 1, "    preparing");
  }

  table = (int *)cl_calloc(corp->size, sizeof(int));

  EvaluationIsRunning = 1;
  nr_evals = 0;
  percentage = -1;

  for (line = 0; line < corp->size && EvaluationIsRunning; line++) {

    if (progress_bar) {
      new_percentage = floor(0.5 + (100.0 * line) / corp->size);
      if (new_percentage > percentage) {
        percentage = new_percentage;
        progress_bar_percentage(0, 0, percentage);
      }
    }

    table[line] = -1;

    switch(base) {

    case MatchField:
      excl_start = corp->range[line].start;
      excl_end   = corp->range[line].end;

      if ((corp->range[line].start == corp->range[line].end) || inclusive) {

        if (!calculate_ranges(corp,
                              corp->range[line].start, context,
                              &lbound, &rbound)) {

          Rprintf("Can't compute boundaries for range #%d", line);
          lbound = rbound = -1;
        }
      }
      else {
        int dummy;

        if (!calculate_ranges(corp,
                             corp->range[line].start, context,
                             &lbound, &dummy)) {

          Rprintf("Can't compute left search space boundary match #%d", line);
          lbound = rbound = -1;
        }
        else if (!calculate_ranges(corp,
                                  corp->range[line].end, context,
                                  &dummy, &rbound)) {

          Rprintf("Can't compute right search space boundary match #%d", line);
          lbound = rbound = -1;
        }
      }
      break;

    case MatchEndField:
      excl_start = excl_end = corp->range[line].end;

      if (excl_start >= 0) {
        if (!calculate_ranges(corp,
                              corp->range[line].end, context,
                              &lbound, &rbound)) {

          Rprintf("Can't compute search space boundaries for match #%d", line);
          lbound = rbound = -1;
        }
      }
      else
        lbound = rbound = -1;
      break;

    case TargetField:
      excl_start = excl_end = corp->targets[line];

      if (excl_start >= 0) {
        if (!calculate_ranges(corp,
                              corp->targets[line], context,
                              &lbound, &rbound)) {

          Rprintf("Can't compute search space boundaries for match #%d", line);
          lbound = rbound = -1;
        }
      }
      else
        lbound = rbound = -1;
      break;

    case KeywordField:
      excl_start = excl_end = corp->keywords[line];

      if (excl_start >= 0) {
        if (!calculate_ranges(corp,
                              corp->keywords[line], context,
                              &lbound, &rbound)) {

          Rprintf("Can't compute search space boundaries for match #%d", line);
          lbound = rbound = -1;
        }
      }
      else
        lbound = rbound = -1;
      break;

    default:
      assert(0 && "Can't be");
      return 0;
    }

    if (lbound >= 0 && rbound >= 0) {
      int dist, maxdist;

      if (direction == ctxtdir_left) {
        rbound = excl_start;
        if (strategy == SearchNearest)
          strategy = SearchRightmost;
        else if (strategy == SearchFarthest)
          strategy = SearchLeftmost;
      }
      else if (direction == ctxtdir_right) {
        lbound = excl_start;
        if (strategy == SearchNearest)
          strategy = SearchLeftmost;
        else if (strategy == SearchFarthest)
          strategy = SearchRightmost;
      }

      switch (strategy) {

      case SearchFarthest:
        maxdist = MAX(excl_start - lbound, rbound - excl_start);
        assert(maxdist >= 0);

        for (dist = maxdist; dist >= 0; dist--) {

          i = excl_start - dist;
          if (i >= lbound && (inclusive || i < excl_start) )
            if (eval_bool(constr, NULL, i)) {
              table[line] = i;
              break;
            }

          i = excl_start + dist;
          if (i <= rbound && (inclusive || i > excl_end))
            if (eval_bool(constr, NULL, i)) {
              table[line] = i;
              break;
            }

          if (++nr_evals == 1000) {
            CheckForInterrupts();
            nr_evals = 0;
          }
        }
        break;

      case SearchNearest:
        maxdist = MAX(excl_start - lbound, rbound - excl_start);
        assert(maxdist >= 0);

        for (dist = 0; dist <= maxdist; dist++) {

          i = excl_start - dist;

          if (i >= lbound && (inclusive || (i < excl_start)))
            if (eval_bool(constr, NULL, i)) {
              table[line] = i;
              break;
            }

          i = excl_start + dist;

          if (i <= rbound && (inclusive || (i > excl_end)))
            if (eval_bool(constr, NULL, i)) {
              table[line] = i;
              break;
            }

          if (++nr_evals == 1000) {
            CheckForInterrupts();
            nr_evals = 0;
          }
        }
        break;

      case SearchLeftmost:
        for (i = lbound; i <= rbound; i++)
          if (inclusive || (i < excl_start) || (i > excl_end)) {
            if (eval_bool(constr, NULL, i)) {
              table[line] = i;
              break;
            }

            if (++nr_evals == 1000) {
              CheckForInterrupts();
              nr_evals = 0;
            }
          }
        break;

      case SearchRightmost:
        for (i = rbound; i >= lbound; i--)
          if (inclusive || (i < excl_start) || (i > excl_end)) {
            if (eval_bool(constr, NULL, i)) {
              table[line] = i;
              break;
            }

            if (++nr_evals == 1000) {
              CheckForInterrupts();
              nr_evals = 0;
            }
          }
        break;

      default:
        break;
      }
    }
  }

  if (progress_bar)
    progress_bar_message(1, 1, "  cleaning up");

  switch (t_id) {
  case MatchField:
    for (i = 0; i < corp->size; i++) {
      if (table[i] >= 0)
        corp->range[i].start = table[i];
      if (corp->range[i].start > corp->range[i].end)
        corp->range[i].start = corp->range[i].end;
    }
    cl_free(table);
    break;

  case MatchEndField:
    for (i = 0; i < corp->size; i++) {
      if (table[i] >= 0)
        corp->range[i].end = table[i];
      if (corp->range[i].end < corp->range[i].start)
        corp->range[i].end = corp->range[i].start;
    }
    cl_free(table);
    break;

  case TargetField:
    cl_free(corp->targets);
    corp->targets = table;
    break;

  case KeywordField:
    cl_free(corp->keywords);
    corp->keywords = table;
    break;

  default:
    assert(0 && "Can't be");
    break;
  }

  if (progress_bar)
    progress_bar_clear_line();

  /* re-sort corpus if match regions were modified */
  if (t_id == MatchField || t_id == MatchEndField)
    RangeSort(corp, 0);

  touch_corpus(corp);

  if (!EvaluationIsRunning) {
    cqpmessage(Warning, "Evaluation interruted: results may be incomplete.");
    if (which_app == cqp)
      install_signal_handler();
  }
  EvaluationIsRunning = 0;

  return 1;
}

/**
 * @param cl            the corpus
 * @param the_field     the field to scan
 * @param constr
 * @return
 */
int
evaluate_subset(CorpusList *cl, FieldType the_field, Constrainttree constr)
{
  int line, position;
  int percentage, new_percentage; /* for ProgressBar */

  assert(cl && constr);
  assert(cl->type == SUB || cl->type == TEMP);

  percentage = -1;

  for (EvaluationIsRunning = 1, line = 0; line < cl->size && EvaluationIsRunning; line++) {

    if (progress_bar) {
      new_percentage = floor(0.5 + (100.0 * line) / cl->size);
      if (new_percentage > percentage) {
        percentage = new_percentage;
        progress_bar_percentage(0, 0, percentage);
      }
    }

    switch (the_field) {

    case MatchField:
      position = cl->range[line].start;
      break;

    case MatchEndField:
      position = cl->range[line].end;
      break;

    case KeywordField:
      assert(cl->keywords);
      position = cl->keywords[line];
      break;

    case TargetField:
      assert(cl->targets);
      position = cl->targets[line];
      break;

    case NoField:
    default:
      position = -1;
      break;
    }

    if (position < 0 || (!eval_bool(constr, NULL, position))) {
      cl->range[line].start = -1;
      cl->range[line].end   = -1;
    }
  }

  /* if interrupted, delete part of temporary query result which hasn't been filtered;
     so that the result is incomplete but at least contains only correct matches */
  while (line < cl->size) {
    cl->range[line].start = -1;
    cl->range[line].end   = -1;
    line++;
  }

  if (!EvaluationIsRunning) {
    cqpmessage(Warning, "Evaluation interruted: results may be incomplete.");
    if (which_app == cqp)
      install_signal_handler();
  }
  EvaluationIsRunning = 0;

  if (progress_bar)
    progress_bar_message(0, 0, "  cleaning up");

  apply_range_set_operation(cl, RReduce, NULL, NULL);

  return 1;
}




