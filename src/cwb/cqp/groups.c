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

#include <math.h>

#include "../cl/cl.h"
#include "../cl/ui-helpers.h"

#include "options.h"
#include "html-print.h"
#include "ascii-print.h"
#include "sgml-print.h"
#include "latex-print.h"

#include "corpmanag.h"
#include "groups.h"
#include "output.h"

/** define this as 1 to prevent temporary file deletion & print additional messages */
#define GROUP_DEBUG 0

/** integer identfier special value: = anything */
#define ANY_ID -2


/** private variable in module: a global Group object. */
static Group *compare_cells_group = NULL;

/** support function for qsort() of grouping cells, controlled by global variables */
static int
compare_cells(const void *p1, const void *p2)
{
  ID_Count_Mapping *cell1 = (ID_Count_Mapping *)p1;
  ID_Count_Mapping *cell2 = (ID_Count_Mapping *)p2;

  const char *w1, *w2;
  int res, f1, f2;

  if (compare_cells_group->is_grouped) {
    /* grouped sort: order by s_freq, then source, then freq, then target */
    f1 = cell1->s_freq;
    f2 = cell2->s_freq;
    res = (f2 > f1) - (f2 < f1); /* corresponds to f2 <=> f1 in Perl */
    if (res != 0)
      return res;

    w1 = Group_id2str(compare_cells_group, cell1->s, 0);
    w2 = Group_id2str(compare_cells_group, cell2->s, 0);
    res = cl_strcmp(w1, w2);
    if (res != 0)
      return res;

    f1 = cell1->freq;
    f2 = cell2->freq;
    res = (f2 > f1) - (f2 < f1);
    if (res != 0)
      return res;

    w1 = Group_id2str(compare_cells_group, cell1->t, 1);
    w2 = Group_id2str(compare_cells_group, cell2->t, 1);
    res = cl_strcmp(w1, w2);
    return res;
  }
  else {
    /* ungrouped sort: order by freq, then source, then target */
    f1 = cell1->freq;
    f2 = cell2->freq;
    res = (f2 > f1) - (f2 < f1);
    if (res != 0)
      return res;

    w1 = Group_id2str(compare_cells_group, cell1->s, 0);
    w2 = Group_id2str(compare_cells_group, cell2->s, 0);
    res = cl_strcmp(w1, w2);
    if (res != 0)
      return res;

    w1 = Group_id2str(compare_cells_group, cell1->t, 1);
    w2 = Group_id2str(compare_cells_group, cell2->t, 1);
    res = cl_strcmp(w1, w2);
    return res;
  }
}


/**
 * Get lexicon ID of grouping element `target` (1 = target, 0 = source) for item `i` in query result set.
 *
 * Returns an integer ID to be used for frequency counts in an n-gram hash. This is either a lexicon ID
 * (if the grouping element refers to a p-attribute) or a virtual ID computed from the offset of the
 * annotation string (if the element refers to an s-attribute, relying on the fact that each unique
 * annotation string is stored only once in the s-attribute).
 *
 * If `cpos` is not NULL, the corresponding corpus position is written to `*cpos`.
 *
 * The following special cases can occur:
 *   - A: The grouping element is unused (source in a simple group statement)
 *     - returns ANY_ID = -2 (which pretty-prints as (all) in the grouping output)
 *     - corpus position is set to ANY_ID
 *   - B: The anchor point is undefined (possible for target and keyword anchors)
 *     - returns -1 (which prints as (none) in the grouping output)
 *     - corpus position is set to ANY_ID
 *   - C: The anchor point with offset (= effective cpos) is outside the valid corpus range
 *     - returns -1 (either explicitly for cpos < 0, or implicitly as returned by cl_cpos2id)
 *     - corpus position is set to effective cpos, or to -1 for _all_ negative values (to avoid confusion with ANY_ID)
 *   - D: The effective cpos is valid, but not inside a range of the referenced s-attribute
 *     - returns -1
 *     - corpus position is set to effective cpos
 *
 * @param group   Pointer to a data structure describing the group operation (and holding results).
 * @param i       The row of the query result to be accessed.
 * @param target  Whether to return information for the source element (0) or the target element (1).
 * @param cpos    If not NULL, the effective corpus position is returned in this location.
 *
 * @return        An integer ID to be used in frequency counts (see above for special cases)
 */
static int
get_group_id(Group *group, int i, int target, int *cpos)
{
  CorpusList *cl  = group->my_corpus;
  int field_type  = (target ? group->target_field     : group->source_field);
  int offset      = (target ? group->target_offset    : group->source_offset);
  Attribute *attr = (target ? group->target_attribute : group->source_attribute);
  int is_struc    = (target ? group->target_is_struc  : group->source_is_struc);
  char *base      = (target ? group->target_base      : group->source_base);
  int pos, id;
  pos = -1;

  switch (field_type) {
  case KeywordField:
    pos = cl->keywords[i];
    break;
  case TargetField:
    pos = cl->targets[i];
    break;
  case MatchField:
    pos = cl->range[i].start;
    break;
  case MatchEndField:
    pos = cl->range[i].end;
    break;
  case NoField:
    /* special case A (there is no source field): ID=ANY_ID, *cpos=ANY_ID */
    if (cpos) *cpos = ANY_ID;
    return ANY_ID;
    break;
  default:
    assert(0 && "get_group_id: reached unreachable code");
    break;
  }
  if (pos < 0) {
    /* special case B (anchor point is undefined): ID=-1, *cpos=ANY_ID */
    if (cpos) *cpos = ANY_ID;
    return -1;
  }

  pos += offset; /* compute effective cpos */
  if (pos < 0) {
    /* special case C1 (effective cpos negative): ID=-1, *cpos=-1 */
    if (cpos) *cpos = -1;
    id = -1;
  }
  else {
    /* in all other cases, we return the effective cpos (which may be beyond end of corpus) */
    if (cpos) *cpos = pos;
    /* special cases C2 and D are handled implicitly below: if CL function returns an error, set ID=-1 */
    if (is_struc) {
      char *str = cl_cpos2struc2str(attr, pos);
      id = ( str ? (str - base) : -1 );
    }
    else {
      /* includes special case C2, for which cl_cpos2id returns -1 */
      id = cl_cpos2id(attr, pos);
      if (id < 0)
        id = -1;
    }
  }
  return id;
}

const char *
Group_id2str(Group *group, int id, int target) {
  Attribute *attr = (target) ? group->target_attribute : group->source_attribute;
  int is_struc = (target) ? group->target_is_struc : group->source_is_struc;
  char *base = (target) ? group->target_base : group->source_base;
  if (id == ANY_ID)
    return "(all)";
  else if (id < 0)
    return "(none)";
  else if (is_struc)
    return base+id;             /* keep fingers crossed ... */
  else
    return (const char *)cl_id2str(attr, id);
}

static Group *
ComputeGroupInternally(Group *group)
{
  cl_ngram_hash pairs, groups; /* frequency counts for (source, target) pairs and groups (= source ID) */
  cl_ngram_hash_entry item;
  Attribute *within = group->within_attribute; /* NULL = token frequencies / s-attribute handle = document frequencies */

  int i, *payload;
  int ids_s_t[2]; /* pair of source and target IDs */
  size_t nr_nodes;
  int percentage, new_percentage; /* for ProgressBar */
  int size = group->my_corpus->size;
  int cpos0, cpos1, struc0, struc1; /* effective cpos of grouping element, and corresponding s-attribute region for df counts */
  int do_within = (within) ? 1 : 0; /* = extra payload size needed for pairs */

  if (progress_bar)
    progress_bar_clear_line();
  percentage = -1;

  pairs  = cl_new_ngram_hash(2, 0, do_within);
  groups = cl_new_ngram_hash(1, 0, 0);

  EvaluationIsRunning = 1;

  for (i = 0; i < size; i++) {
    if (!EvaluationIsRunning)
      break;
      /* user abort (Ctrl-C) is the only way that var can change in this loop. */

    if (progress_bar) {
      new_percentage = floor(0.5 + (100.0 * i) / size);
      if (new_percentage > percentage)
        progress_bar_percentage(1, 2, (percentage = new_percentage) );
    }

    ids_s_t[0] = get_group_id(group, i, 0, &cpos0);   /* source ID */
    ids_s_t[1] = get_group_id(group, i, 1, &cpos1);   /* target ID */
    if (!do_within) {
      cl_ngram_hash_add(pairs,  ids_s_t, 1);        /* count frequency of (source, target) pair */
      cl_ngram_hash_add(groups, ids_s_t, 1);        /* frequency counts for groups (source) */
    }
    else {
      /* compute document frequencies based on <within> regions */
      if (cpos0 == ANY_ID)
        cpos0 = cpos1; /* if there is no source element or its anchor is undefined -> focus on target element */
      else if (cpos1 == ANY_ID)
        cpos1 = cpos0; /* if target element anchor is undefined -> focus on source element */
      if (cpos0 >= 0 && cpos1 >= 0) {
        /* if both anchors are undefined or if the effective cpos of one of them is out of bounds,
         * the pair cannot be assigned to a <within> region and is silently discarded */
        struc0 = cl_cpos2struc(within, cpos0);
        struc1 = cl_cpos2struc(within, cpos1);
        if (struc0 >= 0 && struc1 >= 0 && struc0 == struc1) {
          /* if either effective cpos is out of bounds or not in a <within> region, the pair is silently discarded;
           * the same happens if they are in different <within> regions */
          item = cl_ngram_hash_add(pairs, ids_s_t, 0); /* find (possibly new) entry for this ID pair */
          payload = cl_ngram_hash_payload(pairs, item, NULL); /* initialised to -1 for new entry */
          if (struc0 > *payload) {
            item->freq++;                              /* first occurrence in this <within> region -> count and update payload */
            *payload = struc0;
            cl_ngram_hash_add(groups, ids_s_t, 1);     /* increase total count for the group = sum over all pairs */
          }
          else if (struc0 < *payload) {
            /* anchor points are out of order in query result (can happen for target and keyword);
             * our approach cannot deal with this situation (would substantially increase time and/or memory complexity),
             * so abort the grouping operation with an error message */
            cqpmessage(Error, "Anchor points are out of order for group ... within (aborted).");
            EvaluationIsRunning = 0;
          }
        }
      }
    }
  }

  if (EvaluationIsRunning) {
    if (progress_bar)
      progress_bar_message(2, 2, " cutoff freq.");

    /* extract vector of pairs above the specified frequency threshold */
    group->count_cells = (ID_Count_Mapping *)cl_malloc(cl_ngram_hash_size(pairs) * sizeof(ID_Count_Mapping));
    nr_nodes = 0;
    cl_ngram_hash_iterator_reset(pairs);
    while (NULL != (item = cl_ngram_hash_iterator_next(pairs))) {
      if (item->freq >= group->cutoff_frequency) {
        group->count_cells[nr_nodes].s      = item->ngram[0];
        group->count_cells[nr_nodes].t      = item->ngram[1];
        group->count_cells[nr_nodes].freq   = item->freq;
        group->count_cells[nr_nodes].s_freq = cl_ngram_hash_freq(groups, item->ngram);
        nr_nodes++;
      }
    }

    /* free unused memory if frequency threshold has filtered out items */
    if (nr_nodes < cl_ngram_hash_size(pairs))
      group->count_cells = (ID_Count_Mapping *)cl_realloc(group->count_cells, (nr_nodes * sizeof(ID_Count_Mapping)));
    group->nr_cells = nr_nodes;

    if (progress_bar)
      progress_bar_message(2, 2, " sorting rslt");

    /* now sort entries by decreasing frequency, breaking ties in cl_strcmp() order */
    compare_cells_group = group;
    qsort(group->count_cells, nr_nodes, sizeof(ID_Count_Mapping), compare_cells);

    if (progress_bar) {
      progress_bar_percentage(2, 2, 100); /* so total percentage runs up to 100% */
      progress_bar_clear_line();
    }

  }
  else {
    cqpmessage(Warning, "Group operation aborted.");
    if (which_app == cqp) install_signal_handler();
    free_group(&group);         /* sets return value to NULL to indicate failure */
  }
  EvaluationIsRunning = 0;

  /* free memory */
  cl_delete_ngram_hash(pairs);
  cl_delete_ngram_hash(groups);

  return group;
}

/**
 * This function is used for passing Groups to external tools for grouping.
 * The Group object is modified in situ. The return is that Group again.
 */
static Group *
ComputeGroupExternally(Group *group)
{
  int i;
  int size = group->my_corpus->size;
  int cutoff_freq = group->cutoff_frequency;

  char temporary_name[TEMP_FILENAME_BUFSIZE];
  FILE *pipe;
  char sort_call[CL_MAX_LINE_LENGTH];

  if (group->within_attribute) {
    cqpmessage(Error, "group ... within; not supported for external grouping.");
    return group;
  }

  for (i = 0; i < size; i++)
    Rprintf("%d %d\n", get_group_id(group, i, 0, NULL), get_group_id(group, i, 1, NULL)); /* (source ID, target ID) */

  /* construct sort call */
  sprintf(sort_call, ExternalGroupCommand, temporary_name);
#if GROUP_DEBUG
  Rprintf("Running grouping sort: \n\t%s\n", sort_call);
#endif
  if (!(pipe = popen(sort_call, "r"))) {
    perror("Failure opening grouping pipe");
    cqpmessage(Warning, "Can't open grouping pipe:\n%s\nDisable external grouping with 'set ExternalGroup off;'", sort_call);
  }
  else {
    int freq, p1, p2, tokens;
#define GROUP_REALLOC 1024

    while (3 == (tokens = fscanf(pipe, "%d%d%d", &freq, &p1, &p2))) {
      if (freq > cutoff_freq) {
        if (0 == (group->nr_cells % GROUP_REALLOC)) {
          if (!group->count_cells)
            group->count_cells = (ID_Count_Mapping *)cl_malloc(GROUP_REALLOC * sizeof(ID_Count_Mapping));
          else
            group->count_cells = (ID_Count_Mapping *)cl_realloc(group->count_cells, (GROUP_REALLOC + group->nr_cells) * sizeof(ID_Count_Mapping));
          assert(group->count_cells);
        }

        group->count_cells[group->nr_cells].s = p1;
        group->count_cells[group->nr_cells].t = p2;
        group->count_cells[group->nr_cells].freq = freq;
        group->count_cells[group->nr_cells].s_freq = 0;

        group->nr_cells = group->nr_cells + 1;
      }
    }

    if (tokens != EOF)
      Rprintf("Warning: could not reach EOF of temporary file!\n");
    pclose(pipe);
  }

#if GROUP_DEBUG
  Rprintf("Keeping temporary file %s -- delete manually\n", temporary_name);
#else
  if (0 != unlink(temporary_name)) {
    perror(temporary_name);
    Rprintf("Can't remove temporary file %s -- \n\tI will continue, but you should remove that file.\n", temporary_name);
  }
#endif

  return group;
}


Group *compute_grouping(CorpusList *cl,
                        FieldType source_field,
                        int source_offset,
                        const char *source_attr_name,
                        FieldType target_field,
                        int target_offset,
                        const char *target_attr_name,
                        int cutoff_freq,
                        int is_grouped,
                        const char *within) /* name of s-attribute for document frequency counts (or NULL) */
{
  Group *group;
  Attribute *source_attr, *target_attr, *within_attr;
  int source_is_struc = 0, target_is_struc = 0;
  char *source_base = NULL, *target_base = NULL;

  if (!cl || !cl->corpus) {
    cqpmessage(Warning, "Grouping:\nCan't access corpus.");
    return NULL;
  }

  if (cl->size == 0 || cl->range == NULL) {
    cqpmessage(Warning, "Corpus %s is empty, no grouping possible", cl->name);
    return NULL;
  }

  if ((source_attr_name == NULL) && (source_field == NoField))
    source_attr = NULL;
  else {
    if (!(source_attr = cl_new_attribute(cl->corpus, source_attr_name, ATT_POS))) {
      source_attr = cl_new_attribute(cl->corpus, source_attr_name, ATT_STRUC);
      source_is_struc = 1;
    }
    if (!source_attr) {
      cqpmessage(Error, "Can't find attribute ``%s'' for named query %s", source_attr_name, cl->name);
      return NULL;
    }
    if (source_is_struc) {
      if (cl_struc_values(source_attr)) {
        source_base = cl_struc2str(source_attr, 0); /* should be beginning of the attribute's lexicon */
        assert(source_base && "Internal error. Please don't use s-attributes in group command.");
      }
      else {
        cqpmessage(Error, "No annotated values for s-attribute ``%s'' in named query %s", source_attr_name, cl->name);
        return NULL;
      }
    }

    switch (source_field) {
    case KeywordField:
      if (!cl->keywords) {
        cqpmessage(Error, "No keyword anchors defined for %s", cl->name);
        return NULL;
      }
      break;

    case TargetField:
      if (!cl->targets) {
        cqpmessage(Error, "No target anchors defined for %s", cl->name);
        return NULL;
      }
      break;

    case MatchField:
    case MatchEndField:
      assert(cl->range && cl->size > 0);
      break;

    case NoField:
    default:
      cqpmessage(Error, "Illegal second anchor in group command");
      return NULL;
      break;
    }
  }

  if (!(target_attr = cl_new_attribute(cl->corpus, target_attr_name, ATT_POS))) {
      target_attr = cl_new_attribute(cl->corpus, target_attr_name, ATT_STRUC);
      target_is_struc = 1;
  }
  if (!target_attr) {
    cqpmessage(Error, "Can't find attribute ``%s'' for named query %s", target_attr_name, cl->name);
    return NULL;
  }
  if (target_is_struc) {
    if (cl_struc_values(target_attr)) {
      target_base = cl_struc2str(target_attr, 0); /* should be beginning of the attribute's lexicon */
      assert(target_base && "Internal error. Please don't use s-attributes in group command.");
    }
    else {
      cqpmessage(Error, "No annotated values for s-attribute ``%s'' in named query %s", target_attr_name, cl->name);
      return NULL;
    }
  }

  switch (target_field) {
  case KeywordField:
    if (!cl->keywords) {
      cqpmessage(Error, "No keyword anchors defined for %s", cl->name);
      return NULL;
    }
    break;

  case TargetField:
    if (!cl->targets) {
      cqpmessage(Error, "No target anchors defined for %s", cl->name);
      return NULL;
    }
    break;

  case MatchField:
  case MatchEndField:
    assert(cl->range && cl->size > 0);
    break;

  case NoField:
  default:
    cqpmessage(Error, "Illegal anchor in group command");
    return NULL;
  }

  if (within) {
    within_attr = cl_new_attribute(cl->corpus, within, ATT_STRUC);
    if (!within_attr) {
      cqpmessage(Error, "Can't find attribute ``%s'' for group ... within %s on named query %s", within, within, cl->name);
      return NULL;
    }
  }
  else
    within_attr = NULL;

  /* set up Group object */
  group = (Group *) cl_malloc(sizeof(Group));
  group->my_corpus = cl;
  group->source_attribute = source_attr;
  group->source_offset = source_offset;
  group->source_is_struc = source_is_struc;
  group->source_base = source_base;
  group->source_field = source_field;
  group->target_attribute = target_attr;
  group->target_offset = target_offset;
  group->target_is_struc = target_is_struc;
  group->target_base = target_base;
  group->target_field = target_field;
  group->nr_cells = 0;
  group->count_cells = NULL;
  group->cutoff_frequency = cutoff_freq;
  group->is_grouped = is_grouped;
  group->within_attribute = within_attr;

  /* External grouping shouldn't exist; backwards compatibility demands it be preserved */
  if (UseExternalGroup && !insecure && !(source_is_struc || target_is_struc || is_grouped))
    /* external grouping doesn't support s-attributes and grouped counts */
    return ComputeGroupExternally(group); /* modifies Group object in place and returns pointer */
  else
    return ComputeGroupInternally(group);
}


/** Frees the Group object pointed to by the argument. */
void
free_group(Group **group)
{
  cl_free((*group)->count_cells);
  cl_free(*group);
}


/** Prints a group in a style determined by GlobalPrintMode, to an output stream determined by argument rd (which gets both opened and closed here). */
void
print_group(Group *group, struct Redir *dst)
{
  if (group && open_rd_output_stream(dst, group->my_corpus->corpus->charset)) {
    switch (GlobalPrintMode) {

    case PrintSGML:
      sgml_print_group(group, dst->stream);
      break;

    case PrintHTML:
      html_print_group(group, dst->stream);
      break;

    case PrintLATEX:
      latex_print_group(group, dst->stream);
      break;

    case PrintASCII:
      ascii_print_group(group, dst->stream);
      break;

    default:
      cqpmessage(Error, "Unknown print mode");
      break;
    }
    close_rd_output_stream(dst);
  }
}


