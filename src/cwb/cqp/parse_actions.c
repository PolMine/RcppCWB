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

#include "parse_actions.h"

#include <stdlib.h>
#include <sys/time.h>
#ifndef __MINGW__
#include <sys/resource.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>

#include "../cl/globals.h"
#include "../cl/special-chars.h"
#include "../cl/attributes.h"
#include "../cl/ui-helpers.h"

#include "cqp.h"
#include "options.h"
#include "ranges.h"
#include "symtab.h"
#include "treemacros.h"
#include "tree.h"
#include "eval.h"
#include "corpmanag.h"
#include "regex2dfa.h"
#include "builtins.h"
#include "groups.h"
#include "targets.h"
#include "attlist.h"
#include "concordance.h"
#include "output.h"
#include "print-modes.h"
#include "variables.h"

/* ======================================== GLOBAL PARSER VARIABLES */

/**
 * A boolean; indicates whether the code trees for query execution
 * should actually be built or not when the query input is parsed.
 *
 * When it is false, many actions simply have no effect, because they are
 * set to only actually do anything "if (generate_code)".
 *
 * It's only referenced in this file and in the Yacc grammar.
 *
 * Responses to errors usually involve switching this off - since when
 * there's been an error, the query won't ever run, so no point malloc'ing
 * massive object trees.
 */
int generate_code;

/**
 * A boolean; seems to be about whether or not we are within a global constraint.
 * It's switched on in the SearchPattern parser rule when it gets to RegWordfExpr
 * and then switched off when it gets to GlobalConstraint.
 *
 * It can also be turned off by do_Query / prepare_Query.
 *
 * It's referenced as a test within do_IDReference.
 */
int within_gc;

/** Type of last corpus yielding command */
CYCtype last_cyc;

/** The corpus (or subcorpus) which is "active" in the sense that the query will be executed within it. */
CorpusList *query_corpus = NULL;

/** Used for preserving former values of query_corpus (@see query_corpus), so it can be reset to a former value). */
CorpusList *old_query_corpus = NULL;

int catch_unknown_ids = 0;


/**
 * This is used by the parser in response to CQP's "expand" operator,
 * which incorporates context around the query hit into the match itself.
 * Functions involved in carrying this out utilise info stored here by the parser.
 */
Context expansion;

/** Buffer for storing regex strings. As it says on the tin. */
char regex_string[CL_MAX_LINE_LENGTH];
/** index into the regex string buffer, storing a current position. @ see regex_string */
int regex_string_pos;


/* ======================================== non-exported function declarations */

static char *convert_pattern_for_feature_set(char *s);



/* ======================================== PARSER ACTIONS */

/* ======================================== Syntax rule: line -> command */

/**
 * Add a line of CQP input to the history file.
 *
 * Supports parser rule: line -> command
 *
 * The line that is added comes from QueryBuffer; the file it is
 * written to is that named in cqp_history_file.
 *
 * @see QueryBuffer
 * @see cqp_history_file
 */
void
addHistoryLine(void)
{
  FILE *dst;
  if (cqp_history_file != NULL    &&
      cqp_history_file[0] != '\0' &&
      write_history_file          &&
      !silent                     &&
      !reading_cqprc) {
    if (QueryBuffer[0] != '\0') {
      if (!(dst = cl_open_stream(cqp_history_file, CL_STREAM_APPEND, CL_STREAM_FILE)))
        cqpmessage(Error, "Can't open history file %s\n", cqp_history_file);
      else {
        fputs(QueryBuffer, dst);
        fputc('\n', dst);
        cl_close_stream(dst);
      }
    }
  }
}

/**
 * Empties the query buffer and sets to 0 the pointer.
 *
 * Supports parser rule: line -> command
 *
 * @see QueryBuffer
 * @see QueryBufferP
 */
void
resetQueryBuffer(void)
{
  QueryBufferP = 0;
  QueryBuffer[0] = '\0';
  QueryBufferOverflow = 0;
}

/**
 * Misc steps before parsing a query: turn on generate code,
 * and restore old_query_corpus to query_corpus if there is one.
 */
void
prepare_parse(void)
{
  if (old_query_corpus) {
    query_corpus = old_query_corpus;
    old_query_corpus = NULL;
    cqpmessage(Warning, "Query corpus reset");
  }
  generate_code = 1;
}

/* ======================================== Syntax rule: command -> CorpusCommand ';' */

/**
 * Function that handles subcorpus / NQR assignments, that is "SomeNqr = SomeOtherCorpusOrNqr"
 *
 * @param id   String of the name of the NQR to be created/overwritten.
 * @param cl   The CorpusList entry for the source subcorpus/NQR.
 * @return     Value of global "current_corpus" if the assignment was succesful;
 *             otherwise NULL.
 * */
CorpusList *
in_CorpusCommand(char *id, CorpusList *cl)
{
  if (!cl)
    return NULL;
  if (subcorpus_name_is_qualified(id)) {
    cqpmessage(Warning, "You can't use a qualified subcorpus name on the\nleft hand side of an assignment (result in \"Last\")");
    return NULL;
  }
  if (cl->type == SYSTEM) {
    cqpmessage(Warning, "System corpora can't be duplicated.");
    return NULL;
  }
  duplicate_corpus(cl, id, True);
  last_cyc = Assignment;
  return current_corpus;
}

/**
 * Set the current corpus and
 * do the output if it was a query
 */
void
after_CorpusCommand(CorpusList *cl)
{
#if defined(DEBUG_QB)
  if (QueryBufferOverflow)
    Rprintf("+ Query Buffer overflow.\n");
  else if (QueryBuffer[0] == '\0')
    Rprintf("+ Query Buffer is empty.\n");
  else
    Rprintf("Query buffer: >>%s<<\n", QueryBuffer);
#endif

  switch (last_cyc) {

  case Query:
    if (cl) {
      if (auto_subquery)
        set_current_corpus(cl, 0);
      if (autoshow && cl->size > 0)
        cat_listed_corpus(cl, NULL, 0, -1, GlobalPrintMode);
      else if (!silent)
        Rprintf("%d matches.%s\n", cl->size, (cl->size > 0 ? " Use 'cat' to show." : ""));
    }
    query_corpus = NULL;
    break;

  case Activation:
    if (cl)
      set_current_corpus(cl, 0);
    break;

  case SetOperation:
    if (cl) {
      if (auto_subquery)
        set_current_corpus(cl, 0);
      if (autoshow && cl->size > 0)
        cat_listed_corpus(cl, NULL, 0, -1, GlobalPrintMode);
      else if (!silent)
        Rprintf("%d matches.%s\n", cl->size, (cl->size > 0 ? " Use 'cat' to show." : "") );
    }
    break;

  default:
    break;
  }

  if (auto_save && cl && cl->type == SUB && !cl->saved)
    save_subcorpus(cl, NULL);

  LastExpression = last_cyc;
  last_cyc = NoExpression;
}

/* ======================================== UnnamedCorpusCommand -> CYCommand ReStructure */

/**
 * This function is called after an UnnamedCorpusCommand rule is parsed.
 *
 * Seems to be a tidying-up function.
 *
 * @param cl  The result of the corpus-yielding command (first component of this syntax rule).
 * @return    Modified valuse of cl. May be NULL.
 */
CorpusList *
in_UnnamedCorpusCommand(CorpusList *cl)
{
  CorpusList *res = NULL;

  cqpmessage(Message, "Command: UnnamedCorpusCommand");

  if (cl) {
    switch (last_cyc) {

    case Query:
      /* the last command was a query */
      assert(cl->type == TEMP); /* should be true since the last command was a query! */

      if (generate_code) {
        expand_dataspace(cl);
        do_timing("Query result computed"); /* timer must be started by each query execution command */

        /* set the "corpus" created by the query to be the default "Last" subcorpus. */
        res = assign_temp_to_sub(cl, "Last");
      }
      else
        res = NULL;

      drop_temp_corpora();
      break;

    case Activation:
      /* Last command was not a query, that is, it was a corpus activation.
       * We only have to copy if we want to expand the beast. */
      if (expansion.size > 0) {
        if (cl->type == SYSTEM) {
          cqpmessage(Warning, "System corpora can't be expanded (only subcorpora)");
          res = cl;
        }
        else {
          res = make_temp_corpus(cl, "RHS");
          expand_dataspace(res);
          res = assign_temp_to_sub(res, "Last");
        }
      }
      else
        /* a simple activation without restructuring */
        res = cl;
      break;

    case SetOperation:
      assert(cl->type == TEMP);
      expand_dataspace(cl);
      res = assign_temp_to_sub(cl, "Last");
      drop_temp_corpora();
      break;

    default:
      cqpmessage(Warning, "Unknown CYC type: %d\n", last_cyc);
      res = NULL;
      break;
    }
  }

  free_all_environments();
  return res;
}

/* ======================================== Corpus Yielding Commands */

CorpusList *
ActivateCorpus(CorpusList *cl)
{
  cqpmessage(Message, "ActivateCorpus: %s", cl);

  if (inhibit_activation) {
    Rprintf("Activation prohibited\n");
    exit(cqp_error_status ? cqp_error_status : 1); /* hard way! */
  }
  else {
    query_corpus = cl;

    if (query_corpus) {
      if (!next_environment()) {
        cqpmessage(Error, "Can't allocate another evaluation environment");
        generate_code = 0;
        query_corpus = NULL;
      }
      else
        CurEnv->query_corpus = query_corpus;
    }
    last_cyc = Activation;
  }
  return cl;
}

CorpusList *
after_CorpusSetExpr(CorpusList *cl)
{
  last_cyc = SetOperation;

  if (!next_environment()) {
    cqpmessage(Error, "Can't allocate another evaluation environment");
    generate_code = 0;
    CurEnv->query_corpus = NULL;
  }
  else
    CurEnv->query_corpus = cl;

  return cl;
}

/**
 * This function sets things up to run a query.
 *
 * It is called as an "action" before any detected Query in the parser.
 *
 * [AH 2010/8/2: I have added the code checking input character encoding.
 * Anything that is not part of a query should be plain ASCII - if not,
 * then the lexer/parser should pick it up as bad. Filenames, etc. are
 * obvious exceptions - but we can't check the encoding of those, because
 * there's no guarantee it will be the same as that of the corpus, which
 * is the only thing whose encoding we know. So it's up to the user to
 * type filenames in an encoding their OS will accept!
 * Canonicalisation is done within the CL_Regex, not here.]
 */
void
prepare_Query(void)
{
  generate_code = 1;

  /* check whether we've got a corpus loaded */
  if (!current_corpus ) {
    cqpmessage(Error, "No corpus activated");
    generate_code = 0;
  }
  else if (!access_corpus(current_corpus)) {
    cqpmessage(Error, "Current corpus can't be accessed");
    generate_code = 0;
  }

  if (generate_code) {
    assert(current_corpus->corpus );
    assert(!searchstr);
    assert(ee_ix == -1);

    /* validate character encoding according to that corpus, now we know it's loaded */
    if (!cl_string_validate_encoding(QueryBuffer, current_corpus->corpus->charset, 0)) {
      cqpmessage(Error, "Query includes a character or character sequence that is invalid\n"
                        "in the encoding specified for this corpus");
      generate_code = 0;
    }

    if (!next_environment()) {
      cqpmessage(Error, "Can't allocate another evaluation environment");
      generate_code = 0;
      query_corpus = NULL;
    }
    else {
      int size_before;
      assert(ee_ix == 0);
      assert(CurEnv == &(Environment[0]));

      CurEnv->query_corpus = query_corpus = make_temp_corpus(current_corpus, "RHS");

      /* subqueries don't work properly if the mother corpus has overlapping regions -> delete and warn */
      size_before = query_corpus->size;
      apply_range_set_operation(query_corpus, RNonOverlapping, NULL, NULL);

      if (query_corpus->size < size_before)
        cqpmessage(Warning,
                   "Overlapping matches in %s:%s deleted for subquery execution.",
                   query_corpus->mother_name, query_corpus->name);
    }
  }
  within_gc = 0;
}

CorpusList *
after_Query(CorpusList *cl)
{
  last_cyc = Query;

  within_gc = 0;

  if (generate_code) {
    if (cl) {
      cl_free(cl->query_text);
      cl_free(cl->query_corpus);

      if (query_corpus)
        cl->query_corpus = cl_strdup(query_corpus->name);

      /* this is probably where we want to auto-execute the reduce to maximal stuff */

      if (QueryBuffer[0] && QueryBufferP > 0 && !QueryBufferOverflow)
        cl->query_text = cl_strdup(QueryBuffer);
    }
    return cl;
  }
  return NULL;
}

/* ======================================== ``interactive'' commands */

/**
 * Function that implements the "cat" command.
 */
void
do_cat(CorpusList *cl, struct Redir *dst, int first, int last)
{
  if (cl) {
    cqpmessage(Message, "cat command: (%s)", cl->name);
    cat_listed_corpus(cl, dst, first, last, GlobalPrintMode);
  }
}

/**
 * Implements cat "..." > "somewhere" : writes arbitrary string to a redirected output,
 * the way do_cat() does for a CorpusList.
 *
 * It also implements the \\t, \\n, \\r escape sequences.
 *
 * @param str  String to print
 * @param dst  Redirection specifier.
 */
void
do_catString(const char *str, struct Redir *dst)
{
  char *s, *r, *w; /* string, readpoint, writepoint */

  if (!open_rd_output_stream(dst, unknown_charset)) {
    cqpmessage(Error, "Can't redirect output to file or pipe\n");
    return;
  }

  /* make copy of s to interpret \t, \r and \n escapes */
  s = cl_strdup(str);
  r = w = s;
  while (*r) {
    if (*r == '\\' && *(r + 1)) {
      if (*(r + 1) == 't') {
        *w++ = '\t';
        r += 2;
      }
      else if (*(r + 1) == 'r') {
        *w++ = '\r';
        r += 2;
      }
      else if (*(r + 1) == 'n') {
        *w++ = '\n';
        r += 2;
      }
      else {
        *w++ = *r++; /* pass through all other escaped symbols */
        *w++ = *r++;
      }
    }
    else
      *w++ = *r++;
  }
  *w = '\0'; /* terminate modified string */

  Rprintf("%s", s);
  cl_free(s);

  close_rd_output_stream(dst);
}

/**
 * Implements the save command, saving the specified query (in the cl)
 * to the file specified within the redirection info (the r).
 */
void
do_save(CorpusList *cl, struct Redir *dst)
{
  if (cl && dst) {
    if (!data_directory)
      cqpmessage(Warning, "Can't save subcorpus ``%s'' (you haven't set the DataDirectory option)", cl->name);
    else {
      cqpmessage(Message, "save command: %s to %s", cl->name, dst->name);
      save_subcorpus(cl, dst->name);
    }
  }
}


/**
 * Display info on an attribute - in respone to the "show...." commmand.
 */
void
do_attribute_show(char *name, int status)
{
  AttributeInfo *ai;

  if (cl_streq_ci(name, "cpos") && current_corpus && current_corpus->corpus && !cl_new_attribute(current_corpus->corpus, name, ATT_STRUC))
    CD.print_cpos = status;

  else if (cl_streq_ci(name, "targets") && current_corpus && current_corpus->corpus && !cl_new_attribute(current_corpus->corpus, name, ATT_STRUC))
    show_targets = status;

  else if (CD.attributes || CD.alignedCorpora) {
    if (name) {
      if ((ai = FindInAL(CD.attributes, name)))
        ai->status = status;
      else if ((ai = FindInAL(CD.alignedCorpora, name)))
        ai->status = status;
      else if ((ai = FindInAL(CD.strucAttributes, name)))
        ai->status = status;
      else {
        cqpmessage(Error, "No such attribute: %s", name);
        generate_code = 0;
      }
    }
    else {
      for (ai = CD.attributes->list; ai; ai = ai->next)
        ai->status = status;

      if (!status)
        if (NULL != (ai = FindInAL(CD.attributes, CWB_DEFAULT_ATT_NAME)))
          ai->status = 1;
    }
  }
}


CorpusList *
do_translate(CorpusList *source, char *target_name)
{
  CorpusList *res, *target;
  Attribute *alignment;
  int i, n, bead;
  int s1, s2, t1, t2;

  if (generate_code) {
    assert(source);

    if (!(target = findcorpus(target_name, SYSTEM, 0))) {
      cqpmessage(Warning, "System corpus ``%s'' doesn't exist", target_name);
      generate_code = 0;
      return NULL;
    }

    if (!(alignment = cl_new_attribute(source->corpus, target->corpus->registry_name, ATT_ALIGN))) {
      cqpmessage(Error, "Corpus ``%s'' is not aligned to corpus ``%s''", source->mother_name, target->mother_name);
      generate_code = 0;
      return NULL;
    }

    /* allocate temporary NQR for the translated ranges */
    res = make_temp_corpus(target, "RHS");
    res->size = n = source->size;
    cl_free(res->range);     /* allocate ranges for mapped regions */
    res->range = (Range *)cl_calloc(n, sizeof(Range));
    cl_free(res->targets);   /* make sure there are no spurious target / keywords vectors */
    cl_free(res->keywords);

    /* translate each matching range into target bead */
    for (i = 0; i < n; i++) {
      bead = cl_cpos2alg(alignment, source->range[i].start);
      if (bead < 0 ||
          !cl_alg2cpos(alignment, bead, &s1, &s2, &t1, &t2) ||
          !cl_all_ok()) {
        res->range[i].start = -1;
      }
      else {
        res->range[i].start = t1;
        res->range[i].end = t2;
      }
    }

    /* remove unaligned items (but not duplicates) */
    apply_range_set_operation(res, RReduce, NULL, NULL);

    /* make sure target ranges are sorted (preserving original order with sortidx) */
    RangeSort(res, 1);

    return res;
  }
  else
    return NULL;
}


/** Implements set operation commands; returns the CorpusList of the output */
CorpusList *
do_setop(RangeSetOp op, CorpusList *c1, CorpusList *c2)
{
  CorpusList *result = NULL;

  cqpmessage(Message, "Set Expr");

  /* set operations can only be applied to groups of iontervals referring to the same corpus */
  if (c1 && c2) {
    if (c1->corpus != c2->corpus)
      cqpmessage(Warning, "Original corpora of %s (%s) and %s (%s) differ.\n", c1->name, c1->mother_name, c2->name, c2->mother_name);
    else {
      /* apply the operation to a temporary copy of 1st operand, which thus becomes the result data */
      result = make_temp_corpus(c1, "RHS");
      apply_range_set_operation(result, op, c2, NULL);
    }
  }

  return result;
}

void
prepare_do_subset(CorpusList *cl, FieldType field)
{
  int field_exists = 0;

  if (!cl || cl->type != SUB) {
    cqpmessage(Error, "The subset operator can only be applied to subcorpora.");
    generate_code = 0;
    return;
  }
  if (cl->size == 0) {
    cqpmessage(Warning, "The subcorpus is empty; the subset operation therefore has no effect.");
    return;
  }

  switch (field) {
  case MatchField:
  case MatchEndField:
    field_exists = cl->size > 0;
    break;

  case KeywordField:
    field_exists = cl->size > 0 && cl->keywords != NULL;
    break;

  case TargetField:
    field_exists = cl->size > 0 && cl->targets != NULL;
    break;

  default:
    field_exists = 0;
    break;
  }

  if (!field_exists) {
    cqpmessage(Error, "The <%s> anchor is not defined for this subcorpus.", field_type_to_name(field));
    generate_code = 0;
    return;
  }

  if (progress_bar) {
    progress_bar_clear_line();
    progress_bar_message(1, 1, "    preparing");
  }

  /* now we can finally get going */
  query_corpus = make_temp_corpus(cl, "RHS");
  generate_code = 1;
}

CorpusList *
do_subset(FieldType field, Constrainttree bool_tree)
{
  if (generate_code)
    evaluate_subset(query_corpus, field, bool_tree);

  free_booltree(bool_tree);

  if (progress_bar)
    progress_bar_clear_line();

  if (generate_code)
    return query_corpus;
  else
    return NULL;
}

/**
 * Action for "set $Nqr (match) (matchend)" etc.
 */
void
do_set_target(CorpusList *cl, FieldType goal, FieldType source, int source_offset, int overwrite)
{
  if (cl && goal != NoField) /* shouldn't really happen, aborting in set_target() would be the right thing to do */
    set_target(cl, goal, source, source_offset, overwrite);
}

void
do_set_complex_target(CorpusList *cl,
                      FieldType field_to_set,
                      SearchStrategy strategy,
                      Constrainttree bool_tree,
                      enum ctxtdir direction,
                      int number,
                      char *id,
                      FieldType field,
                      int inclusive)
{
  if (generate_code && cl) {
    /* query_corpus has been saved in old_query_corpus and set to cl by parser */
    evaluate_target(cl,
                    field_to_set,
                    field,
                    inclusive,
                    strategy,
                    bool_tree, direction, number, id);
    query_corpus = old_query_corpus; /* reset query_corpus to previous value */
    old_query_corpus = NULL;
  }

  /* clean up */
  if (bool_tree)
    free_booltree(bool_tree);
}


/**
 * Puts the program to sleep.
 *
 * A wrapper round the standard sleep() function (or Sleep() in Windows).
 *
 * @param duration  How many seconds to sleep for.
 */
void
do_sleep(int duration)
{
  if (duration > 0) {
#ifndef __MINGW__
    sleep(duration);       /* sleep in number of seconds (normal POSIX function) */
#else
    Sleep(duration*1000);  /* sleep in number of milliseconds (Windows equivalent) */
#endif
  }
}

/**
 * Execute the commands contained in a CQP script file (CQP "source FILENAME;")
 *
 * The tricky part is to suspend execution of the parser and ongoing macro expansions,
 * then return safely to the current state when the file has been read.
 */
void
do_exec(char *fname)
{
  FILE *src;

  cqpmessage(Message, "source cmd: %s\n", fname);

  if (NULL != (src = cl_open_stream(fname, CL_STREAM_READ, CL_STREAM_MAGIC_NOPIPE))) {
    /* cease reading exec'ed file on parse error within file */
    if (!cqp_parse_file(src, 1)) {
      cqpmessage(Error, "Syntax errors while executing script file %s.\n", fname);
      generate_code = 0;
    }
  }
  else {
    cqpmessage(Error, "Can't read and execute script file %s.\n", fname);
    generate_code = 0;
  }
}

/**
 * Deletes lines from a subcorpus/NQR as specified by a range
 * (start index to end index)
 */
void
do_delete_lines_num(CorpusList *cl, int start, int end)
{
  if (!cl || cl->type != SUB) {
    cqpmessage(Error, "The delete operator can only be applied to subcorpora.");
    generate_code = 0;
    return;
  }
  if (start <= end) {
    Bitfield lines = create_bitfield(cl->size);
    assert(lines);

    for ( ; start <= end && start < cl->size; start++)
      set_bit(lines, start);
    if (nr_bits_set(lines) > 0)
      delete_intervals(cl, lines, SELECTED_LINES);

    destroy_bitfield(&lines);
  }
}

void
do_delete_lines(CorpusList *cl, FieldType f, int mode)
{
  int *positions = NULL;

  if (!cl || cl->type != SUB) {
    cqpmessage(Error, "The delete operator can only be applied to subcorpora.");
    generate_code = 0;
    return;
  }

  switch (f) {
  case MatchField:
  case MatchEndField:
    cqpmessage(Warning, "\"delete ... with[out] match/matchend\" does not make sense.");
    break;

  case NoField:
    return;

  case KeywordField:
    if (!(positions = cl->keywords))
      cqpmessage(Warning, "No keywords set for this subcorpus");
    break;

  case TargetField:
    if (!(positions = cl->targets))
      cqpmessage(Warning, "No collocates set for this subcorpus");
    break;

  default:
    assert(0 && "Can't (well, shouldn't) be.");
    break;
  }

  if (positions) {
    int i;
    Bitfield lines = create_bitfield(cl->size);
    assert(lines);

    for (i = 0; i < cl->size; i++)
      if (positions[i] >= 0)
        set_bit(lines, i);

    delete_intervals(cl, lines, mode);
    destroy_bitfield(&lines);
  }
}


/**
 * Function implementing the "reduce" command - making an NQR smaller.
 * The matches removed are selected randomly.
 *
 * @param cl      The NQR's CorpusList entry.
 * @param number  Number of matches to retain in reduced query.
 *                Or, a percentage, if the "percent" parameter is true
 * @param percent Boolean determining whether "number" is to be treated as
 *                a count, or as a percentage of existing matches.
 */
void
do_reduce(CorpusList *cl, int number, int percent)
{
  if (!cl || SUB != cl->type) {
    cqpmessage(Error, "The reduce operator can only be applied to named query results.");
    generate_code = 0;
    return;
  }
  if (0 == cl->size) {
    cqpmessage(Warning, "The reduce operator has no effect on named query results with zero matches.\n");
    return;
  }

  /* we need to sanity-check the target size when it's a count, but not when it's a percent. */
  if (percent) {
    if (number <= 0 || number >= 100) {
      cqpmessage(Error, "The \"reduce to n percent\" operation requires a number in the range 1 to 99 (inclusive)");
      generate_code = 0;
      return;
    }
    number = (cl->size * number) / 100;
  }
  else if (number <= 0 || number >= cl->size)
      /* nothing to be done -- don't squeal (a general "reduce Last to 50" without checking size actually >= 50 is quite useful) */
      return;

  /* this algorithm uses a selection probability recalculated after every selection/non-selection
     in order to select a random sample of size <number> without replacement */
  {
    Bitfield matches = create_bitfield(cl->size);  /* bitfield tracking which matches we want to retain */
    unsigned int to_examine = cl->size;            /* how many matches remain to be processed (counts down)*/
    unsigned int to_select  = number;              /* how many matches need to be selected for retention (counts down) */

    for ( ; 0 < to_examine ; to_examine-- ) {
      /* chance of selecting current match = n still needed for our reduced set / n still to look at */
      double prob = ((double)to_select) / ((double)to_examine);
      if (cl_random_fraction() <= prob) {
        /* current match number is to_examine-1; set it to 1 to mark match for retention */
        set_bit(matches, to_examine-1);
        to_select--;
      }
    }

    delete_intervals(cl, matches, UNSELECTED_LINES);
    destroy_bitfield(&matches);
  }
}

void
do_cut(CorpusList *cl, int first, int last)
{
  int n_matches, i;

  if (!cl || cl->type != SUB) {
    cqpmessage(Error, "The cut operator can only be applied to named query results.");
    generate_code = 0;
    return;
  }
  if (0 == (n_matches = cl->size)) {
    cqpmessage(Warning, "Named query result is empty - can't cut\n");
    return;
  }

  assert(first >= 0); /* first < 0 is now disallowed by the parser */
  if (last >= n_matches)
    last = n_matches - 1; /* must be >= 0 because n_matches > 1 has been checked */
  if (first >= n_matches)
    first = n_matches;    /* so the loop below cannot overflow */

  if (last < first) {
    cqpmessage(Warning, "Cut operator applied with empty range %d .. %d, so result is empty.", first, last);
    first = last = n_matches;        /* delete all matches, ensuring that index does not run out of bounds */
  }

  /* CQP Tutorial documents "cut" to respect sort order of NQR (Sec. 3.6: "Random subsets")
   * Since it is considered authoritative documentation on CQP, the implementation here has been adjusted in CQP v3.4.15.
   */
  if (cl->sortidx) {
    for (i = 0; i < first; i++)  {
      int j = cl->sortidx[i];
      cl->range[j].start = -1;        /* delete all matches before <first> according to current sort order */
      cl->range[j].end = -1;
    }
    for (i = last + 1; i < n_matches; i++)  {
      int j = cl->sortidx[i];
      cl->range[j].start = -1;        /* delete all matches after <last> according to current sort order */
      cl->range[j].end = -1;
    }
  }
  else {
    for (i = 0; i < first; i++)  {
      cl->range[i].start = -1;        /* delete all matches before <first> */
      cl->range[i].end = -1;
    }
    for (i = last + 1; i < n_matches; i++)  {
      cl->range[i].start = -1;        /* delete all matches after <last> */
      cl->range[i].end = -1;
    }
  }

  apply_range_set_operation(cl, RReduce, NULL, NULL); /* remove matches marked for deletion */
  touch_corpus(cl);
}

void
do_info(CorpusList *cl)
{
  corpus_info(cl);
}

void
do_group(CorpusList *cl,
         FieldType target, int target_offset, char *t_att,
         FieldType source, int source_offset, char *s_att,
         int cut, int expand, int is_grouped, struct Redir *dst, char *within)
{
  Group *group;

  if (expand) {
    cqpmessage(Error, "group ... expand; has not been implemented");
    generate_code = 0;
    return;
  }
  do_start_timer();
  group = compute_grouping(cl, source, source_offset, s_att, target, target_offset, t_att, cut, is_grouped, within);
  do_timing("Grouping computed");

  if (group) {
    print_group(group, dst);
    free_group(&group);
  }
}

/** Like do_group, but with no source anchor; compute_grouping() is therefore called with empty arguments. */
void
do_group_nosource(CorpusList *cl,
                  FieldType target, int target_offset, char *t_att,
                  int cut, int expand, struct Redir *dst, char *within)
{
  do_group(cl,
      target, target_offset, t_att,
      NoField, 0, NULL,
      cut, expand, 0, dst, within);
}

/**
 * Implements the running of a standard query, that is, a token-level regex query.
 *
 * @see              cqp_run_query
 * @param cut_value  Passed to cqp_run_query.
 * @param keep_flag  Boolean: contain the "keep ranges" setting.
 *                   Passed to cqp_run_query.
 * @param modifer    String with intial embedded-modifier, if there was one. Or NULL.
 * @return           CorpusList object for the result.
 */
CorpusList *
do_StandardQuery(int cut_value, int keep_flag, char *modifier)
{
  CorpusList *result = NULL;

  cqpmessage(Message, "Query");

  /* embedded modifier (?<modifier>) at start of query */
  if (modifier) {
    /* currently, modifiers can only be used to set the matching strategy */
    int code = find_matching_strategy(modifier);
    if (code < 0) {
      cqpmessage(Error, "embedded modifier (?%s) not recognized;\n"
                 "\tuse (?longest), (?shortest), (?standard) or (?traditional) to set matching strategy temporarily",
                 modifier);
      generate_code = 0;
    }
    else
      Environment[0].matching_strategy = code;
    cl_free(modifier); /* allocated by lexer */
  }

  if (parse_only || !generate_code)
    return NULL;

  if (Environment[0].evaltree) {
    debug_output();
    do_start_timer();

    if (keep_flag && SUB != current_corpus->type) {
      cqpmessage(Warning, "``Keep Ranges'' only allowed when querying subcorpora (ignored)");
      keep_flag = 0;
    }

    cqp_run_query(cut_value, keep_flag);

    result = Environment[0].query_corpus;

    /* the new matching strategies require post-processing of the query result */
    switch (Environment[0].matching_strategy) {

    case shortest_match:
      apply_range_set_operation(result, RMinimalMatches, NULL, NULL);         /* select shortest from several nested matches */
      break;

    case standard_match:
      apply_range_set_operation(result, RLeftMaximalMatches, NULL, NULL);     /* reduce multiple matches created by optional query prefix */
      break;

    case longest_match:
      apply_range_set_operation(result, RMaximalMatches, NULL, NULL);         /* select longest from several nested matches */
      break;

    case traditional:
    default:
      /* nothing to do here */
      break;
    }

    /* if there's a cut_value, we may need to reduce the result to <cut_value> matches */
    if (0 < cut_value) {
      /* if there is more than 1 initial pattern in the query, it may have returned more than <cut_value> matches */
      if (result->size > cut_value) {
        Bitfield lines = create_bitfield(result->size);
        int i;
        for (i = 0; i < cut_value; i++)
          set_bit(lines, i);
        if (!delete_intervals(result, lines, UNSELECTED_LINES))
          cqpmessage(Error, "Couldn't reduce query result to first %d matches.\n", cut_value);
        destroy_bitfield(&lines);
      }
    }

  } /* end of "if Environment[0] has an evaltre"e. */

  cl_free(searchstr);

  return result;
}

/**
 * Implements the running of a Meet-Union query.
 *
 * @see              cqp_run_mu_query
 * @param evalt      evaltree on which to run the query.
 * @param keep_flag  Boolean: contain thew "keep ranges" setting.
 *                   Passed to cqp_run_mu_query.
 * @param cut_value  Passed to cqp_run_mu_query.
 */
CorpusList *
do_MUQuery(Evaltree evalt, int keep_flag, int cut_value)
{
  CorpusList *result = NULL;

  cqpmessage(Message, "Meet/Union Query");

  if (parse_only || !generate_code)
    return NULL;

  if (evalt) {
    assert(CurEnv == &Environment[0]);
    CurEnv->evaltree = evalt;
    assert(evalt->type == meet_union || evalt->type == leaf);

    debug_output();
    do_start_timer();

    if (keep_flag && SUB != current_corpus->type) {
      cqpmessage(Warning, "``Keep Ranges'' only allowed when querying subcorpora");
      keep_flag = 0;
    }

    cqp_run_mu_query(keep_flag, cut_value);
    result = Environment[0].query_corpus;
  }

  return result;
}

/**
 * Populates the global CurEnv with compiled data regarding the query part of a StandardQuery,
 * that is, the "search pattern" part that contains the token-level regular expression,
 * global constraint, and within clause (and not any leading modifiers or trailing alignment
 * contraints, cut commands, etc.)
 *
 * It takes the expr Evaltree and compiles it to a DFA, and stores that, plus the two arguments,
 * in the global Environment pointed to by CurEnv.
 *
 * @param expr        The eval tree representing the token-level regular expression.
 *                    Parameter $2 when the rule is parsed.
 *                    Gets stored as CurEnv->evaltree.
 * @param constraint  The constraint tree representing any global constraint.
 *                    Parameter $4 when the rule is parsed.
 */
void
do_SearchPattern(Evaltree expr, Constrainttree constraint)
{
  cqpmessage(Message, "SearchPattern");

  if (generate_code) {
    CurEnv->evaltree = expr;
    CurEnv->gconstraint = constraint;

    if (!check_labels(CurEnv->labels)) {
      cqpmessage(Error, "Illegal use of labels, query not evaluated.");
      generate_code = 0;
      return;
    }

    /* create a regex string from the current eval tree, and put it in the regexdfa in-tray */
    searchstr = evaltree2searchstr(CurEnv->evaltree, NULL);

    if (search_debug) {
      Rprintf("Evaltree: \n");
      print_evaltree(ee_ix, CurEnv->evaltree, 0);
      Rprintf("Search String: ``%s''\n", searchstr);
    }

    /* if searchstr is NEITHER empty NOR just a string of spaces, compile it to a DFA */
    if (searchstr && strspn(searchstr, " ") < strlen(searchstr))
      regex2dfa(searchstr, &(CurEnv->dfa));
    else {
      cqpmessage(Error, "Query is vacuous, not evaluated.");
      generate_code = 0;
    }
    cl_free(searchstr);
  }
  /* endif generate_code */
}

/* ======================================== Regular Expressions */

/** Links two evaltrees into a new node using disjunction (implements '|') */
Evaltree
reg_disj(Evaltree left, Evaltree right)
{
  Evaltree ev = NULL;
  if (generate_code)
    NEW_EVALNODE(ev, re_disj, left, right, repeat_none, repeat_none);
  return ev;
}

/** Links two evaltrees into a new node using order dependent concatenation (implements sequencing) */
Evaltree
reg_seq(Evaltree left, Evaltree right)
{
  Evaltree ev = NULL;
  if (generate_code)
    NEW_EVALNODE(ev, re_od_concat, left, right, repeat_none, repeat_none);
  return ev;
}

/** Adds an anchor to CurEnv's pattern list. Returns -1 if something wrong; otherwise, the pattern index of the new anchor in CurEnv */
int
do_AnchorPoint(FieldType field, int is_closing)
{
  int res = -1;

  cqpmessage(Message, "Anchor: <%s%s>", (is_closing ? "/" : ""), field_type_to_name(field));

  if (generate_code) {
    if (MAXPATTERNS == CurEnv->MaxPatIndex) {
      cqpmessage(Error, "Too many patterns (max is %d)", MAXPATTERNS);
      generate_code = 0; /* YYABORT; */
    }
  }

  if (generate_code) {
    /* check that <target> or <keyword> anchor is defined in query_corpus */
    switch (field) {

    case MatchField:
    case MatchEndField:
      /* ok (if query_corpus->size == 0, subquery will simply return no matches) */
      break;

    case TargetField:
      if (query_corpus->targets == NULL) {
        cqpmessage(Error, "<target> anchor not defined in %s", query_corpus->name);
        generate_code = 0;
      }
      break;

    case KeywordField:
      if (query_corpus->keywords == NULL) {
        cqpmessage(Error, "<keyword> anchor not defined in %s", query_corpus->name);
        generate_code = 0;
      }
      break;

    default:
      /* should not be reachable */
      assert("Internal error in do_AnchorPoint()" && 0);
    }
  }

  if (generate_code) {
    CurEnv->MaxPatIndex++;
    CurEnv->patternlist[CurEnv->MaxPatIndex].type = Anchor;
    CurEnv->patternlist[CurEnv->MaxPatIndex].anchor.is_closing = is_closing;
    CurEnv->patternlist[CurEnv->MaxPatIndex].anchor.field = field;
    res = CurEnv->MaxPatIndex;
  }

  if (!generate_code)
    res = -1;

  return res;
}


/**
 * Implements mastching an s-attribute boundary (open tag, open tag with constraint on annotation value, 
 * or close tag) within a standard query. 
 *
 * @param  s_name                       The tag for the XML element (i.e. the s-attribute name).
 * @param  is_closing                   Boolean: are we looking for a closing tag? False for opening tag.
 * @param  op                           Constant for the comparison operator: OP_EQUAL, OP_MATCHES, or OP_CONTAINS. OP_NOT may be bit-OR'ed.
 * @param  regex                        Query term for the annotation value to match (if there is one).
 * @param  flags                        Flag constant: IGNORE_REGEX, or one or both of IGNORE_CASE and IGNORE_DIAC, bit-OR'ed together.
 * @return                              -1 for error; otherwise the number of patterns in the eval environment after this part of the query
 *                                      is complete.
 * Function implementing the actions taken when an XML tag is encountered in the string CQP is parsing
 * (e.g. u_who="Cherie"%l).
 *
 * @param s_name     Name of the s-attribute corresponding to the XML tag.
 * @param is_closing Boolean: true if this is a closing XML tag.
 * @param op         One of the OP_ constants indicating the operqator of the value comparison;
 *                   if the operation is negated, then OP_NOT should be bitwise-or'd onto it.
 * @param regex      Regular expression to match against values (will be passed to CL regopt module);
 *                   pass NULL if no value-matching is desired (or if it's a valueless s-att).
 * @param flags      Bitwise-Or'd-together flags to be passed to the CL regopt module
 *                   (IGNORE_DIAC, IGNORE_CASE, IGNORE_REGEX for %d, %c, and %l respectively).
 */
int
do_XMLTag(char *s_name, int is_closing, int op, char *regex, int flags)
{
  Attribute *attr = NULL;
  int op_type = op & OP_NOT_MASK;
  int negated = op & OP_NOT;
  int res = -1;

  cqpmessage(Message, "StructureDescr: <%s%s>", (is_closing ? "/" : ""), s_name);

  if (generate_code) {
    if (MAXPATTERNS == CurEnv->MaxPatIndex) {
      cqpmessage(Error, "Too many patterns (max is %d)", MAXPATTERNS);
      generate_code = 0; /* YYABORT; */
    }
  }

  if (generate_code) {
    if (!(attr = cl_new_attribute(query_corpus->corpus, s_name, ATT_STRUC))) {
      cqpmessage(Error, "Structural attribute %s.%s does not exist.", query_corpus->name, s_name);
      generate_code = 0; /* YYABORT; */
    }
    else if (regex && !cl_struc_values(attr)) {
      cqpmessage(Error, "Structural attribute %s.%s does not have annotated values.", query_corpus->name, s_name);
      generate_code = 0;
    }
  }

  if (generate_code && (op_type == OP_MATCHES || op_type == OP_CONTAINS) && flags == IGNORE_REGEX) {
    cqpmessage(Error, "Can't use literal strings with 'contains' and 'matches' operators.");
    generate_code = 0;
  }

  if (generate_code) {
    CurEnv->MaxPatIndex++;
    CurEnv->patternlist[CurEnv->MaxPatIndex].type = Tag;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.attr = attr;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.is_closing = is_closing;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.constraint = NULL;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.flags = 0;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.rx = NULL;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.negated = 0;
    CurEnv->patternlist[CurEnv->MaxPatIndex].tag.right_boundary = (LabelEntry) NULL;

    /* start tag may have regex constraint on annotated values */
    if (!is_closing && regex) {
      cl_string_latex2iso(regex, regex, strlen(regex));        /* interpret latex escapes */

      if (flags == IGNORE_REGEX || ((strcspn(regex, "[](){}.*+|?\\") == strlen(regex)) && (flags == 0) && (op_type == OP_EQUAL))) {
        /* match as literal string -> don't compile regex */
      }
      else {
        int   safe_regex = !(strchr(regex, '|') || strchr(regex, '\\'));
        char *conv_regex;        /* OP_CONTAINS and OP_MATCHES */
        char *pattern;
        pattern = "";
        CL_Regex rx;

        switch(op_type) {
        case OP_CONTAINS:
        case OP_MATCHES:
          conv_regex = convert_pattern_for_feature_set(regex);
          pattern = cl_malloc(strlen(conv_regex) + 42); /* leave some room for the regexp wrapper */

          if (OP_CONTAINS == op_type)
            sprintf(pattern, ".*\\|(%s)\\|.*", conv_regex);
          else      /* op_type == OP_MATCHES */
            /* if inner regexp is 'safe', we can omit the parentheses and thus enable optimisation */
            sprintf(pattern, safe_regex ? "\\|(%s\\|)+" : "\\|((%s)\\|)+", conv_regex);

          cl_free(conv_regex);
          break;

        case OP_EQUAL:
          pattern = cl_strdup(regex);
          break;

        default:
          assert(0 && "do_XMLTag(): illegal opcode (internal error)");
        }

        if (!(rx = cl_new_regex(pattern, flags, query_corpus->corpus->charset))) {
          cqpmessage(Error, "Illegal regular expression: %s", regex);
          generate_code = 0;
        }
        else
          CurEnv->patternlist[CurEnv->MaxPatIndex].tag.rx = rx;

        cl_free(pattern);
      }
      CurEnv->patternlist[CurEnv->MaxPatIndex].tag.constraint = regex;
      CurEnv->patternlist[CurEnv->MaxPatIndex].tag.flags      = flags;
      CurEnv->patternlist[CurEnv->MaxPatIndex].tag.negated    = negated;
    }
  }

  if (generate_code && strict_regions) {
    /* label is 'defined' by first open tag and 'used' by following close tag -> in this case, it is activated */
    LabelEntry label;
    if (!is_closing) {            /* open tag -> 'define' label */
      label = label_lookup(CurEnv->labels, s_name, LAB_DEFINED|LAB_RDAT, 1);
      CurEnv->patternlist[CurEnv->MaxPatIndex].tag.right_boundary = label;
    }
    else {                        /* close tag -> if label is already defined, it is 'used', i.e. activated */
      label = find_label(CurEnv->labels, s_name, LAB_RDAT);
      if (label && (label->flags & LAB_DEFINED)) {
        label->flags |= LAB_USED; /* activate this label for strict regions */
        CurEnv->patternlist[CurEnv->MaxPatIndex].tag.right_boundary = label;
      }
      else {
        /* end tag doesn't check or reset the label if it isn't preceded by a corresponding open tag */
        /*           label = label_lookup(CurEnv->labels, s_name+offset, LAB_RDAT, 1); */
      }
    }
  }

  if (generate_code)
    res = CurEnv->MaxPatIndex;
  else {
    res = -1;
    cl_free(regex);
  }

  return res;
}

/**
 * Implements the CQP-syntax rule for a region element (i.e. <<att>> or <<NamedQuery>>).
 *
 * @param name          unqualified name of the query result, or name of the s-attribute
 * @param start_target  whether start of range should be marked as target or keyword
 * @param start_label   label to set on start of range
 * @param end_target    whether end of range should be marked as target or keyword
 * @param end_label     label to set on end of range
 * @param zero_width    if True, only test whether current position is the start of a suitable range
 *
 * @return              new Evaltree object representing the query fragment that implements a region element
 */
Evaltree do_RegionElement(char *name,
                          target_nature start_target, char *start_label,
                          target_nature end_target, char *end_label, int zero_width) {
  Evaltree res = NULL, enter = NULL, hold = NULL, emit = NULL;
  Attribute *attr = NULL;
  CorpusList *nqr = NULL;
  StateQueue queue = NULL;
  char *corpus_name = NULL;
  char *nqr_name = NULL;
  LabelEntry start_lab, end_lab;

  if (!generate_code)
    return NULL;

  if ((CurEnv->MaxPatIndex + 3) >= MAXPATTERNS) {
    cqpmessage(Error, "Too many patterns (max is %d)", MAXPATTERNS);
    generate_code = 0;
    return NULL;
  }
  if (zero_width && (end_label != NULL || end_target != IsNotTarget)) {
    cqpmessage(Error, "Cannot set label or target marker on end of zero-width region <<%s/>>", name);
    generate_code = 0;
    return NULL;
  }


  /* try to find a named query result for the query_corpus */
  corpus_name = (query_corpus->type == SUB || query_corpus->type == TEMP) ? query_corpus->mother_name : query_corpus->name; /* type TEMP is a temporary subcorpus, which seems to be used for all subqueries */
  nqr_name = cl_malloc(strlen(corpus_name) + strlen(name) + 2);
  sprintf(nqr_name, "%s:%s", corpus_name, name); /* construct qualified NQR name for lookup with findcorpus() */
  nqr = findcorpus(nqr_name, SUB, 0); /* also ensures that NQR is loaded from disk if necessary */
  cl_free(nqr_name);
  if (!nqr) {
    /* otherwise try to find a suitable s-attribute */
    attr = cl_new_attribute(query_corpus->corpus, name, ATT_STRUC);
    if (!attr) {
      cqpmessage(Error, "<<%s>> is neither a named query result nor an s-attribute of corpus %s", name, corpus_name);
      generate_code = 0;
      return NULL;
    }
  }

  /* look up start and end labels (cf. do_NamedWfPatter()) */
  if (start_label) {
     start_lab = label_lookup(CurEnv->labels, start_label, LAB_DEFINED, 1);
     if (start_lab->flags & LAB_SPECIAL) {
       cqpmessage(Error, "Can't set special label %s", start_label);
       generate_code = 0;
       return NULL;
     }
   }
   else
     start_lab = NULL;

  if (end_label) {
     end_lab = label_lookup(CurEnv->labels, end_label, LAB_DEFINED, 1);
     if (end_lab->flags & LAB_SPECIAL) {
       cqpmessage(Error, "Can't set special label %s", end_label);
       generate_code = 0;
       return NULL;
     }
   }
   else
     end_lab = NULL;

  /* define target / keyword labels if necessary (cf. do_NamedWfPattern()) */
  if (start_target == IsTarget || end_target == IsTarget) {
    CurEnv->has_target_indicator = 1;
    CurEnv->target_label = label_lookup(CurEnv->labels, "target", LAB_DEFINED|LAB_USED, 1);
  }
  if (start_target == IsKeyword || end_target == IsKeyword) {
    CurEnv->has_keyword_indicator = 1;
    CurEnv->keyword_label = label_lookup(CurEnv->labels, "keyword", LAB_DEFINED|LAB_USED, 1);
  }

  /* allocate the wait list for FSA states, based on the active symbol table for labels */
  if (!zero_width)
    queue = StateQueue_new(CurEnv->labels);

  /* For a zero-width region element <<NQR/>>, we only create the ENTER zero-width transition
   * and do not associate a StateQueue object.  The FSA simulation code recognises the zero-width
   * case from queue == NULL and sets anchor / label directly for the destination state.
   */

  /* the ENTER transition */
  CurEnv->MaxPatIndex++;
  CurEnv->patternlist[CurEnv->MaxPatIndex].type = Region;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.opcode = AVSRegionEnter;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.name = cl_strdup(name);
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.queue = queue;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.nqr = nqr;   /* exactly one of nqr and attr is non-NULL */
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.attr = attr;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.start_label = start_lab;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.start_target = start_target;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.end_label = end_lab;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.end_target = end_target;
  NEW_EVALLEAF(enter, CurEnv->MaxPatIndex);

  if (zero_width)
    return enter;

  /* the WAIT transition */
  CurEnv->MaxPatIndex++;
  CurEnv->patternlist[CurEnv->MaxPatIndex].type = Region;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.opcode = AVSRegionWait;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.name = cl_strdup(name);
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.queue = queue; /* weak reference, don't deallocate */
  NEW_EVALLEAF(hold, CurEnv->MaxPatIndex);

  /* the EMIT transition */
  CurEnv->MaxPatIndex++;
  CurEnv->patternlist[CurEnv->MaxPatIndex].type = Region;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.opcode = AVSRegionEmit;
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.name = cl_strdup(name);
  CurEnv->patternlist[CurEnv->MaxPatIndex].region.queue = queue; /* weak reference, don't deallocate */
  NEW_EVALLEAF(emit, CurEnv->MaxPatIndex);

  /* construct query fragment <enter> (<hold>)* <emit> */
  NEW_EVALNODE(res, re_repeat, hold, NULL, 0, repeat_inf);
  res = reg_seq(enter, res);
  res = reg_seq(res, emit);
  return res;
}

/**
 * Implements the CQP-syntax rule for a named wordform pattern.
 *
 * @param  is_target     Enum flagging whether it has no marker, is marked as target, or is marked as keyword.
 * @param  label_str     Label string, will be looked up in the symbol table.
 * @param  pat_idx       Pattern list index (which pattern to work with).
 * @return               An output pattern list index (or 0 or -1)
 * @return               One of: 0 (if not generate_code) or the value passed as pat_idx.
 */
int
do_NamedWfPattern(target_nature is_target, char *label_str, int pat_idx)
{
  int res = -1;
  LabelEntry lab;

  cqpmessage(Message, "NamedWfPattern");

  if (generate_code) {
    if (label_str ) {
      /* lookup or create label_str */
      lab = label_lookup(CurEnv->labels, label_str, LAB_DEFINED, 1);
      /* user isn't allowed to set special label */
      if (lab->flags & LAB_SPECIAL) {
        cqpmessage(Error, "Can't set special label %s", label_str);
        generate_code = 0;
        return 0;
      }
    }
    else
      lab = NULL;

    switch (CurEnv->patternlist[pat_idx].type) {

    case Pattern:
      CurEnv->patternlist[pat_idx].con.label = lab;
      CurEnv->patternlist[pat_idx].con.is_target = is_target;
      break;

    case MatchAll:
      CurEnv->patternlist[pat_idx].matchall.label = lab;
      CurEnv->patternlist[pat_idx].matchall.is_target = is_target;
      break;

    default:
      assert("Can't be" && 0);
      break;
    }

    if (is_target == IsTarget) {
      CurEnv->has_target_indicator = 1;
      CurEnv->target_label = label_lookup(CurEnv->labels, "target", LAB_DEFINED|LAB_USED, 1);
      /* the special "target" label is never formally ``used'' in the construction of the
         NFA, so we declare it as both DEFINED and USED (which it will be in <eval.h>) */
    }
    else if (is_target == IsKeyword) {
      CurEnv->has_keyword_indicator = 1;
      CurEnv->keyword_label = label_lookup(CurEnv->labels, "keyword", LAB_DEFINED|LAB_USED, 1);
    }

    res = pat_idx;
  }
  else
    res = 0;

  return res;
}

/**
 * Implements the grammar rule that matches a token pattern in a CQP query, by
 * adding a tree of compiled Boolean statements as the next entry in the CurEnv's patternlist,
 * which is of type Patternlist, i.e. an array of AVStructure objects.

 * @see    AVStructure
 * @see    Patternlist
 * @param  boolt         The tree of boolean constraints which will be added to the patternlist.
 * @param  lookahead     Value to insert for the lookahead variable in the new entry in
 *                       the patternlist.
 * @return               The index of the patternlist entry that boolt has been added to.
 *                       -1 for error (which arises only if we have run out of patternlist slots,
 *                       or there was an error before this function was called).
 */
int
do_WordformPattern(Constrainttree boolt, int lookahead)
{
  int res;

  if (generate_code) {
    if (MAXPATTERNS == CurEnv->MaxPatIndex) {
      cqpmessage(Error, "Too many patterns (max is %d)", MAXPATTERNS);
      generate_code = 0;
    }
  }

  if (generate_code) {
    CurEnv->MaxPatIndex++;

    if ((boolt->type == cnode) && (boolt->constnode.val == 1)) {
      /* matchall */

      cl_free(boolt);

      CurEnv->patternlist[CurEnv->MaxPatIndex].type = MatchAll;
      CurEnv->patternlist[CurEnv->MaxPatIndex].matchall.label = NULL;
      CurEnv->patternlist[CurEnv->MaxPatIndex].matchall.is_target = IsNotTarget;
      CurEnv->patternlist[CurEnv->MaxPatIndex].matchall.lookahead = lookahead;
    }
    else {
      CurEnv->patternlist[CurEnv->MaxPatIndex].type = Pattern;

/* the assertion below is utter bollocks; that pattern may have had a different type
   in the previous query, so the assertion will only be true if the particular bit of memory
   storing the pointer has been initialised to zeroes */
/*        assert(CurEnv->patternlist[CurEnv->MaxPatIndex].con.constraint == NULL); */
      CurEnv->patternlist[CurEnv->MaxPatIndex].con.constraint = boolt;
      CurEnv->patternlist[CurEnv->MaxPatIndex].con.label      = NULL;
      CurEnv->patternlist[CurEnv->MaxPatIndex].con.is_target = IsNotTarget;
      CurEnv->patternlist[CurEnv->MaxPatIndex].con.lookahead  = lookahead;
    }
    res = CurEnv->MaxPatIndex;
  }
  else
    res = -1;

  return res;
}


Constrainttree
OptimizeStringConstraint(Constrainttree left, enum b_ops op, Constrainttree right)
{
  Constrainttree c = NULL;

  if (right->type == cnode) {
    cl_free(left);
    c = right;
    right = NULL;
    if (op == cmp_neq)
      c->constnode.val = !c->constnode.val;
  }
  else {
    NEW_BNODE(c);

    if (right->leaf.pat_type == REGEXP) {

      int range = cl_max_id(left->pa_ref.attr);

      /* optimise regular expressions to idlists for categorical attributes (at most MAKE_IDLIST_BOUND lexicon entries) */
      if ((range > 0) && (range < MAKE_IDLIST_BOUND)) {
        int *items;
        int nr_items;

        items = cl_regex2id(left->pa_ref.attr, right->leaf.ctype.sconst, right->leaf.canon, &nr_items);

        if (!cl_all_ok()) {
          cqpmessage(Error, "Error while collecting matching IDs of %s\n(%s)\n",
                     right->leaf.ctype.sconst,
                     cl_error_string(cl_errno));
          generate_code = 0;

          c->type = cnode;
          c->constnode.val = 0;
        }
        else if (nr_items == 0) {
          cl_free(items);
          c->type = cnode;
          c->constnode.val = (op == cmp_eq ? 0 : 1);
        }
        else if (nr_items == range) {
          cl_free(items);
          c->type = cnode;
          c->constnode.val = (op == cmp_eq ? 1 : 0);
        }
        else {
          c->type = id_list;
          c->idlist.attr = left->pa_ref.attr;
          c->idlist.label = left->pa_ref.label;
          c->idlist.del = left->pa_ref.del;

          c->idlist.nr_items = nr_items;
          c->idlist.items = items;
          c->idlist.negated = (op == cmp_eq ? 0 : 1);

          /* if more than half of all IDs match, the ID list can be processed more
             efficiently when it is inverted (exchanging == for != in the comparison
             where it is used, and vice versa); however, for sparse attributes (where
             the most frequent type is "no value") the inverted list might match almost
             all tokens in the corpus, which can be catastrophic when the ID list is
             used for index lookup (in query-initial position); therefore, the decision
             for or against negation should be based on whether the ID list matches
             more than half of all tokens; for "normal" attributes, the two criteria
             will lead to very similar decisions anyway */

          /* previous condition removed: */
          /*           if (nr_items > range/2) { */

          if (cl_idlist2freq(left->pa_ref.attr, items, nr_items) > cl_max_cpos(left->pa_ref.attr) / 2) {
            int i, k, pos, last_id;
            int *ids;

            ids = (int *)cl_malloc((range - nr_items) * sizeof(int));
            pos = 0;
            last_id = -1;

            for (i = 0; i < nr_items; i++) {
              if (last_id < 0) {
                for (k = 0; k < items[i]; k++)
                  ids[pos++] = k;
              }
              else {
                for (k = last_id + 1; k < items[i]; k++)
                  ids[pos++] = k;
              }
              last_id = items[i];
            }
            for (k = last_id + 1; k < range; k++)
              ids[pos++] = k;

            assert(pos == range - nr_items);

            c->idlist.nr_items = range - nr_items;
            c->idlist.items = ids;
            c->idlist.negated = !c->idlist.negated;

            cl_free(items);
          }
        }

        cl_free(left);
        cl_free(right);
      }
      else {
        c->type = bnode;
        c->node.op_id = op;
        c->node.left = left;
        c->node.right = right;
      }
    }
    else {
      int id;
      assert(right->leaf.pat_type == NORMAL);
      id = cl_str2id(left->pa_ref.attr, right->leaf.ctype.sconst);

      if (id < 0) {
        if (catch_unknown_ids) {
          /* nb effectively if (0) since catch_unknown_ids is initialised to 0 and no code changes it -- AH*/
          cqpmessage(Error, "The string ``%s'' is not in the value space of ``%s''\n",
                     right->leaf.ctype.sconst, left->pa_ref.attr->any.name);
          generate_code = 0;
        }

        cl_free(right);
        cl_free(left);
        c->type = cnode;
        c->constnode.val = (op == cmp_eq ? 0 : 1);
      }
      else {
        c->type = bnode;
        c->node.op_id = op;
        c->node.left = left;
        c->node.right = right;

        cl_free(right->leaf.ctype.sconst);

        right->leaf.pat_type = CID;
        right->leaf.ctype.cidconst = id;
      }
    }
  }

  return c;
}

/**
 * Implements a token expression which consists only of a string (regex pattern) and its flags,
 * outside the usual token-square-brackets with no attribute specified, so the default p-attribute,
 * generally "word", is matched.
 *
 * @param  s        String to be matched.
 * @param  flags    IGNORE_CASE and/or IGNORE_DIAC (bitwise-OR'd); or, IGNORE_REGEX.
 * @return          Newly-created Constrainttree node for this token expression. NULL in case of error.
*/
Constrainttree
do_StringConstraint(char *s, int flags)
{
  Constrainttree c = NULL, left = NULL, right = NULL;
  Attribute *attr = NULL;

  if (generate_code) {
    if (!(attr = cl_new_attribute(query_corpus->corpus, def_unbr_attr, ATT_POS))) {
      cqpmessage(Error,
                 "``%s'' attribute not defined for corpus ``%s'',\nusing ``%s''",
                 def_unbr_attr, query_corpus->name, CWB_DEFAULT_ATT_NAME);

      set_string_option_value("DefaultNonbrackAttr", CWB_DEFAULT_ATT_NAME);

      if (!(attr = cl_new_attribute(query_corpus->corpus, CWB_DEFAULT_ATT_NAME, ATT_POS))) {
        cqpmessage(Error,
                   "``%s'' attribute not defined for corpus ``%s''",
                   CWB_DEFAULT_ATT_NAME, query_corpus->name);

        generate_code = 0;
      }
    }
  }

  if (generate_code) {
    if (!(right = do_flagged_string(s, flags)))
      generate_code = 0;

    /* if the compiled node turns out to be a constant, it is assigned to c (the return value) */
    else if (right->type == cnode)
      c = right;

    else {
      /* make a new leaf node which holds the attribute */
      NEW_BNODE(left);
      left->type = pa_ref;
      left->pa_ref.attr = attr;
      left->pa_ref.label = NULL;
      left->pa_ref.del = 0;

      /* and what gets returned is then the optimised-constraint created from
         an equals-comparison of the node for the string and the node for the default p-attribute */
      c = OptimizeStringConstraint(left, cmp_eq, right);
    }
  }

  /* return the new constraint tree node if all OK; otherwise, NULL */
  return generate_code ? c : NULL;
}

/**
 * Creates a node of type id_list with items drawn from variable var_name
 */
Constrainttree
Varref2IDList(Attribute *attr, enum b_ops op, char *var_name)
{
  Constrainttree node = NULL;

  if (generate_code) {
    Variable v;
    if (!(v = FindVariable(var_name))) {
      cqpmessage(Error, "%s: no such variable.");
      generate_code = 0;
    }
    else {
      NEW_BNODE(node);
      node->type = id_list;
      node->idlist.attr = attr;
      node->idlist.label = NULL;
      node->idlist.del = 0;
      node->idlist.negated = (op == cmp_eq ? 0 : 1);
      node->idlist.items = GetVariableItems(v, query_corpus->corpus, attr, &(node->idlist.nr_items));

      if (0 == node->idlist.nr_items) {
        /* optimise: empty ID list -> constant */
        node->type = cnode;
        node->constnode.val = (op == cmp_eq ? 0 : 1); /* always FALSE for '=', always TRUE for '!=' */
        /* NB: no need to free idlist.items, because the list is empty (NULL pointer) */
      }
    }
  }

  return node;
}

Constrainttree
do_SimpleVariableReference(char *var_name)
{
  Attribute *attr = NULL;

  if (generate_code) {
    if (!(attr = cl_new_attribute(query_corpus->corpus, def_unbr_attr, ATT_POS))) {
      cqpmessage(Error,
                 "``%s'' attribute not defined for corpus ``%s'',"
                 "\nusing ``%s''",
                 def_unbr_attr, query_corpus->name,
                 CWB_DEFAULT_ATT_NAME);

      set_string_option_value("DefaultNonbrackAttr", CWB_DEFAULT_ATT_NAME);

      if (!(attr = cl_new_attribute(query_corpus->corpus, CWB_DEFAULT_ATT_NAME, ATT_POS))) {
        cqpmessage(Error,
                   "``%s'' attribute not defined for corpus ``%s''",
                   CWB_DEFAULT_ATT_NAME, query_corpus->name);

        generate_code = 0;
      }
    }
  }

  if (generate_code)
    return Varref2IDList(attr, cmp_eq, var_name);
  else
    return NULL;
}

void
do_MatchSelector(char *start, int start_offset, char *end, int end_offset) {
  LabelEntry start_label = NULL, end_label = NULL;

  if (generate_code) {
    if (start) {
      start_label = label_lookup(CurEnv->labels, start, LAB_USED, 0);
      if (!start_label) {
        cqpmessage(Error, "Label ``%s'' hasn't been defined.", start);
        generate_code = 0;
        return;
      }
      if (start_label->flags & LAB_SPECIAL) {
        cqpmessage(Error, "Special label ``%s'' not allowed here.", start);
        generate_code = 0;
        return;
      }
      CurEnv->match_selector.begin = start_label;
    }
    CurEnv->match_selector.begin_offset = start_offset;

    if (end) {
      end_label = label_lookup(CurEnv->labels, end, LAB_USED, 0);
      if (!end_label) {
        cqpmessage(Error, "Label ``%s'' hasn't been defined.", end);
        generate_code = 0;
        return;
      }
      if (end_label->flags & LAB_SPECIAL) {
        cqpmessage(Error, "Special label ``%s'' not allowed here.", end);
        generate_code = 0;
        return;
      }
      CurEnv->match_selector.end = end_label;
    }
    CurEnv->match_selector.end_offset = end_offset;
  }
}

void
prepare_AlignmentConstraints(char *id)
{
  Attribute *algattr;
  CorpusList *cl;

  if (NULL == (cl = findcorpus(id, SYSTEM, 0))) {
    cqpmessage(Warning, "System corpus ``%s'' is undefined", id);
    generate_code = 0;
  }
  else if (!access_corpus(cl)) {
    cqpmessage(Warning, "Corpus ``%s'' can't be accessed", id);
    generate_code = 0;
  }
  else if (!(algattr = cl_new_attribute(Environment[0].query_corpus->corpus, cl->corpus->registry_name, ATT_ALIGN))) {
    cqpmessage(Error, "Corpus ``%s'' is not aligned to corpus ``%s''", Environment[0].query_corpus->mother_name, id);
    generate_code = 0;
  }
  else if (!next_environment()) {
    cqpmessage(Error, "Can't allocate another evaluation environment (too many alignments)");
    generate_code = 0;
    query_corpus = NULL;
  }
  else {
    CurEnv->aligned = algattr;
    CurEnv->query_corpus = cl;
    query_corpus = cl;
  }
}

/* ======================================== BOOLEAN OPS */

/**
 * Creates a constraint tree node by linking two other 
 * nodes into a boolean OR relationship.
 */
Constrainttree
bool_or(Constrainttree left, Constrainttree right)
{
  Constrainttree result = NULL;

  if (generate_code) {
    if (left->node.type == cnode) {
      if (left->constnode.val == 0) {
        result = right;
        free_booltree(left);
      }
      else {
        result = left;
        free_booltree(right);
      }
    }
    else if (right->node.type == cnode) {
      if (right->constnode.val == 0) {
        result = left;
        free_booltree(right);
      }
      else {
        result = right;
        free_booltree(left);
      }
    }
    else {
      NEW_BNODE(result);
      result->node.type = bnode;
      result->node.op_id = b_or;
      result->node.left = left;
      result->node.right = right;

      result = try_optimization(result);
    }
  }

  return result;
}

Constrainttree
bool_implies(Constrainttree left, Constrainttree right)
{
  Constrainttree result = NULL;

  if (generate_code) {
    if (left->node.type == cnode) {
      if (left->constnode.val == 0) { /* LHS is FALSE -> implication always TRUE */
        result = left;
        result->constnode.val = 1;
        free_booltree(right);
      }
      else {                        /* LHS is TRUE -> implication == RHS */
        result = right;
        free_booltree(left);
      }
    }
    else if (right->node.type == cnode) {
      if (right->constnode.val == 0) { /* RHS is FALSE -> implication == !(LHS) */
        result = bool_not(left);
        free_booltree(right);
      }
      else {                        /* RHS is TRUE -> implication always TRUE */
        result = right;
        free_booltree(left);
      }
    }
    else {
      NEW_BNODE(result);
      result->node.type = bnode;
      result->node.op_id = b_implies;
      result->node.left = left;
      result->node.right = right;

      result = try_optimization(result);
    }
  }
  else
    result = NULL;

  return result;
}

/**
 * Creates a constraint tree node by linking two other 
 * nodes into a boolean AND relationship.
 */
Constrainttree
bool_and(Constrainttree left, Constrainttree right)
{
  Constrainttree result = NULL;

  if (generate_code) {
    if (left->node.type == cnode) {
      if (left->constnode.val == 0) {
        result = left;
        free_booltree(right);
      }
      else {
        result = right;
        free_booltree(left);
      }
    }
    else if (right->node.type == cnode) {
      if (right->constnode.val == 0) {
        result = right;
        free_booltree(left);
      }
      else {
        result = left;
        free_booltree(right);
      }
    }
    else {
      NEW_BNODE(result);
      result->node.type = bnode;
      result->node.op_id = b_and;
      result->node.left = left;
      result->node.right = right;
    }
  }

  return result;
}

/**
 * Creates a constraint tree node by applying 
 * the Boolean NOT operator to the outcome of 
 * an existing node (representing whatever is 
 * leftward of the ! ).
 */
Constrainttree
bool_not(Constrainttree left)
{
  Constrainttree result = NULL;

  if (generate_code) {
    if (left->node.type == cnode) {
      left->constnode.val = !(left->constnode.val);
      result = left;
    }
    else if (left->type == id_list) {
      left->idlist.negated = !left->idlist.negated;
      result = left;
    }
    else if (left->type == bnode && left->node.op_id == b_not && NULL == left->node.right) {
      result = left->node.left;
      left->node.left = NULL;
      free_booltree(left);
    }
    else {
      NEW_BNODE(result);
      result->node.type = bnode;
      result->node.op_id = b_not;
      result->node.left = left;
      result->node.right = NULL;
    }
  }

  return result;
}

Constrainttree
do_RelExpr(Constrainttree left, enum b_ops op, Constrainttree right)
{
  Constrainttree result = NULL;

  if (generate_code) {
    if (right->type == var_ref) {
      if (left->type == pa_ref) {
        result = Varref2IDList(left->pa_ref.attr, op, right->varref.varName);

        /* be careful: res might be of type cnode, when an empty id_list has been optimised away */
        if (result && result->type == id_list && generate_code) {
          result->idlist.label  = left->pa_ref.label;
          result->idlist.del = left->pa_ref.del;
        }
      }
      else {
        cqpmessage(Error, "LHS of variable reference must be the name of a positional attribute");
        generate_code = 0;
      }
      free_booltree(left);
      free_booltree(right);
    }

    else if ((left->type == pa_ref) && (right->type == string_leaf)) {
      if (op == cmp_eq || op == cmp_neq)
        result = OptimizeStringConstraint(left, op, right);
      else {
        cqpmessage(Error, "Inequality comparisons (<, <=, >, >=) are not allowed for strings and regular expressions");
        generate_code = 0;
      }
    }

    else {
      NEW_BNODE(result);
      result->type = bnode;
      result->node.op_id = op;
      result->node.left = left;
      result->node.right = right;
      result = try_optimization(result);
    }
  }

  return result;
}

Constrainttree
do_RelExExpr(Constrainttree left)
{
  Constrainttree result = NULL;

  if (generate_code) {
    NEW_BNODE(result);
    result->type = bnode;
    result->node.op_id = cmp_ex;
    result->node.left = left;
    result->node.right = NULL;

    result = try_optimization(result);
  }
  else
    result = NULL;
  return result;
}

Constrainttree
do_LabelReference(char *label_name, int auto_delete)
{
  Constrainttree result = NULL;
  Attribute *attr = NULL;
  LabelEntry label = NULL;
  char *hack = NULL;

  if (CurEnv == NULL) {
    cqpmessage(Error, "No label references allowed");
    generate_code = 0;
  }
  else {
    /* find the dot in the qualified name */
    hack = strchr(label_name, '.');
    if (hack == NULL) {
      cqpmessage(Error, "``%s'' is not a valid label reference.", label_name);
      generate_code = 0;
    }
  }

  if (generate_code) {
    *hack = '\0';
    hack++;
    /* now, label_name keeps the label, hack points to the attribute */

    label = label_lookup(CurEnv->labels, label_name, LAB_USED, 0);
    /*     if (!(lab->flags & LAB_SPECIAL) && !(lab->flags & LAB_DEFINED)) { */
    if (!label) {
      /* this is more like what we want: label hasn't been defined yet ('this' label is implicitly defined) */
      cqpmessage(Error, "Label ``%s'' used before it was defined", label_name);
      generate_code = 0;
    }
    else if (label->flags & LAB_SPECIAL) {
      if (auto_delete) {
        cqpmessage(Warning, "Cannot auto-delete special label '%s' [ignored].", label_name);
        auto_delete = 0;
      }
    }
  }

  if (generate_code) {
    if (NULL != (attr = cl_new_attribute(query_corpus->corpus, hack, ATT_POS))) {
      /* reference to positional attribute at label */
      NEW_BNODE(result);
      result->type = pa_ref;
      result->pa_ref.attr = attr;
      result->pa_ref.label = label;
      result->pa_ref.del = auto_delete;
    }
    else if (!(attr = cl_new_attribute(query_corpus->corpus, hack, ATT_STRUC))) {
      cqpmessage(Error, "Attribute ``%s'' is not defined for corpus", hack);
      generate_code = 0;
    }
    else {
      /* reference to (value of) structural attribute at label */
      if (!cl_struc_values(attr)) {
        cqpmessage(Error, "Need attribute with values (``%s'' has no values)", hack);
        generate_code = 0;
      }
      else {
        NEW_BNODE(result);
        result->type = sa_ref;
        result->sa_ref.attr = attr;
        result->sa_ref.label = label;
        result->sa_ref.del = auto_delete;
      }
    }
  }

  cl_free(label_name);

  if (!generate_code)
    result = NULL;

  return result;
}

Constrainttree
do_IDReference(char *id_name, int auto_delete)  /* auto_delete may only be set if this ID is a bare label */
{
  Constrainttree result = NULL;
  Attribute *attr = NULL;
  LabelEntry lab = NULL;

  if (generate_code) {
    if (!within_gc && (attr = cl_new_attribute(query_corpus->corpus, id_name, ATT_POS))) {
      NEW_BNODE(result);
      result->type = pa_ref;
      result->pa_ref.attr = attr;
      result->pa_ref.label = NULL;
      result->pa_ref.del = 0;
    }
    else if (NULL != (lab = label_lookup(CurEnv->labels, id_name, LAB_USED, 0))) {
      NEW_BNODE(result);
      result->type = pa_ref;
      result->pa_ref.attr = NULL;
      result->pa_ref.label = lab;
      if ((lab->flags & LAB_SPECIAL) && auto_delete) {
        cqpmessage(Warning, "Cannot auto-delete special label '%s' [ignored].", id_name);
        auto_delete = 0;
      }
      result->pa_ref.del = auto_delete;
      auto_delete = 0;                /* we'll check that below */
    }
    else if (NULL != (attr = cl_new_attribute(query_corpus->corpus, id_name, ATT_STRUC))) {
      /* Well I was wondering myself ... this is needed for references
         to structural attributes in function calls. The semantics of say
         's' is to return an INT value of
           1 ... if current position is the start of a region
           2 ... if it's the end of a region
           0 ... otherwise
         If the current position is not within an 's' region, the whole
         boolean expression where the reference occurred evals to False */

      NEW_BNODE(result);
      result->type = sa_ref;
      result->sa_ref.attr = attr;
      /* Need to set label to NULL now that we put sa_ref to better use.
         A label's sa_ref now returns the value of the enclosing region */
      result->sa_ref.label = NULL;
      result->sa_ref.del = 0;
    }
    else {
      if (within_gc)
        cqpmessage(Error, "``%s'' is not a (qualified) label reference", id_name);
      else
        cqpmessage(Error, "``%s'' is neither a positional/structural attribute nor a label reference", id_name);

      generate_code = 0;
      auto_delete = 0;                /* so we won't raise another error */
      result = NULL;
    }
  }

  /* if auto_delete is still set, it was set on an attribute -> error */
  if (auto_delete) {
    cqpmessage(Error, "Auto-delete expression '~%s' not allowed ('%s' is not a label)", id_name, id_name);
    generate_code = 0;
    result = NULL;
  }

  cl_free(id_name);
  return result;
}

/**
 * Implements expansion of a variable within the RE() operator.
 */
Constrainttree
do_flagged_re_variable(char *varname, int flags)
{
  Constrainttree tree= NULL;
  Variable var;
  char *s, *mark, **items;
  int length, i, l, N_strings;

  if (flags == IGNORE_REGEX) {
    cqpmessage(Warning, "%c%c flag doesn't make sense with RE($%s) (ignored)", '%', 'l', varname);
    flags = 0;
  }

  if (NULL != (var = FindVariable(varname))) {
    items = GetVariableStrings(var, &N_strings);
    if (NULL == items || 0 == N_strings) {
      cqpmessage(Error, "Variable $%s is empty.", varname);
      generate_code = 0;
    }
    else {
      /* compute length of interpolated regular expression */
      length = 1;
      for (i = 0; i < N_strings; i++)
        length += strlen(items[i]) + 1;
      s = cl_malloc(length);
      l = sprintf(s, "%s", items[0]);
      mark = s + l;        /* <mark> points to the trailing null byte */
      for (i = 1; i < N_strings; i++) {
        l = sprintf(mark, "|%s", items[i]);
        mark += l;
      }
      cl_free(items);
      /* now <s> contains the disjunction over all REs stored in <var> */

      /* since the var strings were loaded without charset checking,
       * we need to now check the regex for the present corpus's encoding. */
      if (! cl_string_validate_encoding(s, query_corpus->corpus->charset, 0)){
        cqpmessage(Error, "Variable $%s used with RE() includes one or more strings with characters that are invalid\n"
                "in the encoding specified for corpus [%s]", varname, query_corpus->corpus->name);
        generate_code = 0;
        cl_free(s);
      }
      else
        tree = do_flagged_string(s, flags);
        /* note that <s> is inserted into the constraint tree and mustn't be freed here */
    }
  }
  else {
    cqpmessage(Error, "Variable $%s is not defined.", varname);
    generate_code = 0;
  }

  cl_free(varname);
  return tree;
}

/**
 * Creates a constraint tree node for a flagged regular expression.
 *
 * @param  s     The constraint stringh (regex, or literal with IGNORE_REGEX).
 * @param  flags IGNORE_REGEX, or either or both of IGNORE_CASE/IGNORE_DIAC (bitwise-OR'd).
 * @return       The new node, ort NULL in case of error.
 */
/**
 * Creates constraint tree node for a string pattern of form "REGEX"%cdl
 * (where the %cdl give rise to IGNORE_CASE, IGNORE_DIAC, IGNORE_REGEX).
 *
 * @param s      String pattern to match.
 * @param flags  Bitwise-Or'd collection of IGNORE_CASE, IGNORE_DIAC, IGNORE_REGEX (or 0 for none of them).
 * @return       Resulting boolean node.
 */
Constrainttree
do_flagged_string(char *s, int flags)
{
  Constrainttree this_node = NULL;

  if (generate_code) {
    NEW_BNODE(this_node);
    this_node->type = string_leaf;
    this_node->leaf.canon = flags;

    cl_string_latex2iso(s, s, strlen(s));

    if ( IGNORE_REGEX == flags || (strcspn(s, "[](){}.*+|?\\") == strlen(s) && 0 == flags) ) {
      /* literal string matching dueto IGNORE_REGEX or a regex with no special-symbols used */
      this_node->leaf.ctype.sconst = s;
      this_node->leaf.pat_type = NORMAL;
    }
    else {
      /* regex string matching: if some regex special-symbol was found, or flags includes IGNORE_CASE and/or IGNORE_DIAC. */
      this_node->leaf.pat_type = REGEXP;
      this_node->leaf.ctype.sconst = s;
      this_node->leaf.rx = cl_new_regex(s, flags, query_corpus->corpus->charset);
      if (!this_node->leaf.rx) {
        cqpmessage(Error, "Illegal regular expression: %s", s);
        this_node->leaf.pat_type = NORMAL;
        generate_code = 0;
      }

    }
  }
  if (!generate_code) {
    free_booltree(this_node);
    /* this was a memory leak (the BNODE created in this func was never passed to its destructor) [AH 2021-03-09] */
    this_node = NULL; /* signal failure to caller (otherwise do_StringConstraint() would continue to operate on, and, free the now invalid pointer) [SE 2021-12-31] */
  }

  return this_node;
}

/**
 * Converts a string that has been given one of the multi-value-matching
 * operators, rather than just equals, to the form of a regular expression that
 * will interact correctly with the features-enclosed-by-vertical-bars format
 * used by feature-set p-attributes,
 *
 * (Why needed? because, in an mval_string regexp, matchall dots ('.') need to be converted to '[^|]'
 * in order to give intuitive results; without this conversion, '.*' might gobble up all the following
 * separator bars ('|'), which we obviously don't want when using "contains" etc.)
 * @param  s  String to adjust.
 * @return    Newly allocated string; NULL for error.
 *
 * @param s   String containing the pattern to match against individual values (not whole p-att value)
 */
static char *
convert_pattern_for_feature_set(char *s)
{
  char *result, *p;
  int n_dots = 0;

  /* count dots in <s> */
  for (p = s; *p; p++)
    if (*p == '.')
      n_dots++;

  p = result = (char *)cl_malloc(strlen(s) + 3*n_dots + 1);   /* every '.' -> '[^|]' replacement adds three characters */

  while (*s) {
    if (*s == '\\') {
      /* copy escaped character verbatim */
      *p++ = *s++;
      if (!(*s)) {
        cqpmessage(Error, "convert_pattern_for_feature_set(): RegExp '%s' ends with escape", s);
        generate_code = 0;
        cl_free(result);
        return NULL;
      }
      *p++ = *s++;
    }
    else if (*s == '.') {
      s++;
      *p++ = '[';
      *p++ = '^';
      *p++ = '|';
      *p++ = ']';
    }
    else
      *p++ = *s++;
  }

  *p = '\0';
  /* end of string */

  return result;
}


/** do_feature_set_string() replaces do_flagged_string() for 'contains' and 'matches' operators
    that operate on multi-valued attributes (feature-sets) */
Constrainttree
do_feature_set_string(char *s, int op, int flags)
{
  Constrainttree result = NULL;
  char *pattern;     /* regexp that implements the multi-value operator */
  char *converted_s; /* holder for adjusted argument s */
  int safe_regexp;

  if (generate_code) {
    if (IGNORE_REGEX == flags) {
      cqpmessage(Error, "Can't use literal strings with 'contains' and 'matches' operators.");
      generate_code = 0;
      return NULL;
    }
    safe_regexp = !(strchr(s, '|') || strchr(s, '\\')); /* see below */
    converted_s = convert_pattern_for_feature_set(s);
    if (!converted_s)
      return NULL; /* generate_code already set to 0 in subroutine */

    pattern = cl_malloc(strlen(converted_s) + 42); /* leave some room for the regexp wrapper */

    switch (op & OP_NOT_MASK) {
    case OP_CONTAINS:
      sprintf(pattern, ".*\\|(%s)\\|.*", converted_s);
      break;
    case OP_MATCHES:
      if (safe_regexp)                /* inner regexp is 'safe' so we can omit the parentheses and thus enable optimisation */
        sprintf(pattern, "\\|(%s\\|)+", converted_s);
      else
        sprintf(pattern, "\\|((%s)\\|)+", converted_s);
      break;
    default:
      /* undefined operator */
      assert(0 && "do_feature_set_string(): illegal opcode (internal error)");
    }

    result = do_flagged_string(pattern, flags);
    cl_free(converted_s);
    if (!result)
      cl_free(pattern);      /* the pattern is inserted into the RegExp node, so don't free it unless do_flagged_string() failed */
  }

  return result;
}

/**
 * Create a constraint tree node that contains a function call.
 *
 * @param f_name   Name of the function.
 * @param apl      Argument list for the function.
 * @return         New Constrainttree object, of type function,
 *                 with the supplied argument list.
 */
Constrainttree
do_FunctionCall(char *f_name, ActualParamList *apl)
{
  Constrainttree result = NULL;
  int n_args, predef;
  ActualParamList *p;
  Attribute *attr;

  cqpmessage(Message, "FunctionCall: %s(...)", f_name);

  if (generate_code) {
    /* set n_args to the length of the linked list of arguments */
    for (n_args = 0, p = apl; p; p = p->next)
      n_args++;

    /* predef = ID of a predefined function which matches the function name. */
    predef = find_predefined_function(f_name);

    if (predef >= 0) {
      /* a predef function was found */
      if (n_args != builtin_function[predef].nr_args) {
        generate_code = 0;
        cqpmessage(Error, "Illegal number of arguments for %s (need %d, got %d)", f_name, builtin_function[predef].nr_args, n_args);
      }
      else {
        NEW_BNODE(result);
        result->type = func;
        result->func.predef = predef;
        result->func.dynattr = NULL;
        result->func.args = apl;
        result->func.nr_args = n_args;
      }
    }
    else if (NULL != (attr = cl_new_attribute(query_corpus->corpus, f_name, ATT_DYN))) {
      /* no predef function found: are we able to get a reference to the named function as a DynamicAttribute? */
      NEW_BNODE(result);
      result->type = func;
      result->func.predef = -1;
      result->func.dynattr = attr;
      result->func.args = apl;
      result->func.nr_args = n_args;
    }
    else {
      /* no predef function found, and no DynamicAttribute could be found/created */
      cqpmessage(Error, "Function ``%s'' is not defined", f_name);
      generate_code = 0;
    }
  }

  return generate_code ? result : NULL;
}


void
do_Description(Context *context, int nr, char *name)
{
  if (!context)
    return;
  context->space_type = word;
  context->attrib = NULL;
  context->size = 0;

  if (generate_code) {
    if (nr < 0) {
      cqpmessage(Error,  "Can't expand to negative size: %d", nr);
      generate_code = 0;
    }
    else if (Environment[0].query_corpus) {
      context->size = nr;
      if (!name || cl_str_is(name, "word") || cl_str_is(name, "words") ) {
        context->space_type = word;
        context->attrib = NULL;
      }
      else {
        if (!(context->attrib = cl_new_attribute(Environment[0].query_corpus->corpus, name, ATT_STRUC))) {
          cqpmessage(Error,
                     "Structure ``%s'' is not defined for corpus ``%s''",
                     name, Environment[0].query_corpus->name);
          generate_code = 0;
        }
        else
          context->space_type = structure;
      }
    }
    else {
      cqpmessage(Error, "No query corpus yielded and/or accessible");
      generate_code = 0;
    }
  }
}


Evaltree
do_MeetStatement(Evaltree left, Evaltree right, Context *context, int negated)
{
  Evaltree ev = NULL;

  if (generate_code) {
    ev = (Evaltree)cl_malloc(sizeof(union e_tree));
    ev->type = meet_union;
    ev->cooc.op_id = cooc_meet;
    ev->cooc.left = left;
    ev->cooc.right = right;
    ev->cooc.lw = context->size;
    ev->cooc.rw = context->size2;
    ev->cooc.struc = context->attrib;
    ev->cooc.negated = (negated != 0);
  }

  return ev;
}

Evaltree
do_UnionStatement(Evaltree left, Evaltree right)
{
  Evaltree ev = NULL;

  if (generate_code) {
    ev = (Evaltree)cl_malloc(sizeof(union e_tree));
    ev->type = meet_union;
    ev->cooc.op_id = cooc_union;
    ev->cooc.left = left;
    ev->cooc.right = right;
    ev->cooc.lw = 0;
    ev->cooc.rw = 0;
    ev->cooc.struc = NULL;
    ev->cooc.negated = 0;
  }

  return ev;
}

void
do_StructuralContext(Context *context, char *name)
{
  context->space_type = word;
  context->attrib = NULL;
  context->size  = 1;
  context->size2 = 1;

  if (query_corpus) {
    context->size = 1;
    context->size2 = 1;

    if (!(context->attrib = cl_new_attribute(query_corpus->corpus, name, ATT_STRUC))) {
      cqpmessage(Error, "Structure ``%s'' is not defined for corpus ``%s''", name, query_corpus->corpus->id);
      generate_code = 0;
    }
    else
      context->space_type = structure;
  }
  else {
    context->size = 0;
    generate_code = 0;
  }
}

CorpusList *
do_TABQuery(Evaltree patterns)
{
  if (parse_only || !generate_code || !patterns)
    return NULL;

  assert(CurEnv == &Environment[0]);
  CurEnv->evaltree = patterns;
  assert(patterns->type == tabular);
  debug_output();

  do_start_timer();
  cqp_run_tab_query();

  return Environment[0].query_corpus;
}

Evaltree
make_first_tabular_pattern(int pattern_index, Evaltree next)
{
  Evaltree node = NULL;

  if (generate_code) {
    node = (Evaltree)cl_malloc(sizeof(union e_tree));
    node->type = tabular;
    node->tab_el.patindex = pattern_index;
    node->tab_el.min_dist = 0;
    node->tab_el.max_dist = 0;
    node->tab_el.next = next;
  }
  return node;
}

/**
 * Add new tab pattern to end of a linked list of such "patterns";
 * returns new head of list or NULL if not generate_code.
 */
Evaltree
add_tabular_pattern(Evaltree patterns, Context *context, int pattern_index)
{
  Evaltree node, curr;

  if (generate_code) {
    node = (Evaltree)cl_malloc(sizeof(union e_tree));
    node->type = tabular;
    node->tab_el.patindex = pattern_index;
    node->tab_el.min_dist = context->size;
    node->tab_el.max_dist = context->size2;
    node->tab_el.next = NULL;

    if (!patterns)
      return node;

    for (curr = patterns ; curr->tab_el.next ; curr = curr->tab_el.next)
      ;
    curr->tab_el.next = node;
    return patterns;
  }

  return NULL;
}

void
do_OptDistance(Context *context, int l_bound, int u_bound)
{
  if (!context)
    return;

  if (l_bound < 0) {
    cqpmessage(Warning, "Left/Min. distance must be >= 0 (reset to 0)");
    l_bound = 0;
  }

  if (u_bound < 0 && u_bound != repeat_inf) {
    cqpmessage(Warning, "Right/Max. distance must be >= 0 (reset to 0)");
    u_bound = 0;
  }

  if (u_bound < l_bound && u_bound != repeat_inf) {
    cqpmessage(Warning, "Right/Max. distance must be >= Left/Max. distance");
    u_bound = l_bound;
  }

  context->space_type = word;
  context->size = l_bound;
  context->size2 = u_bound;
  context->attrib = NULL;
}

/* ======================================== Variable Settings */

/**
 * Prints the setting of a single Variable as an indented list.
 */
void
printSingleVariableValue(Variable v, int max_items)
{
  int i;

  if (!v)
    return;

  Rprintf("$%s = \n", v->my_name);
  if (max_items <= 0)
    max_items = v->nr_items;

  ilist_start(0, 0, 0);
  for (i = 0; i < v->nr_items; i++) {
    if (i >= max_items) {
      ilist_print_item("...");
      break;
    }
    if (!v->items[i].free)
      ilist_print_item(v->items[i].sval);
  }
  ilist_end();
}

/**
 * Prints settings of all Variables as an indented list. Implements CQP "show variables".
 */
void
do_PrintAllVariables()
{
  Variable v;
  variables_iterator_new();

  /* show at most 44 words from each variable in overview */
  while (NULL != (v = variables_iterator_next()))
    printSingleVariableValue(v, 44);
}

/**
 * Prints values of a Variable. Implements CQP "show (name of var)".
 */
void
do_PrintVariableValue(char *varName)
{
  Variable v;

  if (NULL != (v = FindVariable(varName)))
    printSingleVariableValue(v, 0);
  else
    cqpmessage(Error, "%s: no such variable", varName);
}

void
do_printVariableSize(char *varName)
{
  Variable v = FindVariable(varName);
  int i, size = 0;

  if (v) {
    for (i = 0; i < v->nr_items; i++)
      if (!v->items[i].free)
        size++;
    Rprintf("$%s has %d entries\n", v->my_name, size);
  }
  else
    cqpmessage(Error, "%s: no such variable", varName);
}

/**
 * Implments CQP 'define $var = "aaa bbb ccc"'
 */
void
do_SetVariableValue(char *varName, char operator, char *varValues)
{
  Variable v;

  if (!(v = FindVariable(varName)))
    v = NewVariable(varName);

  if (!v) {
    cqpmessage(Warning, "Can't create variable, probably fatal (bad variable name?)");
    return;
  }

  if (operator != '<')
    cl_string_latex2iso(varValues, varValues, strlen(varValues));

  if (!SetVariableValue(varName, operator, varValues))
    cqpmessage(Error, "Error in variable value definition.");
}

/**
 * Implements addition or subtraction of strings in word-list variables
 * (the CQP command 'define $some_var += $some_other_var', or, likewise '-='. )
 *
 * @param var1Name    Name of variable we're changing.
 * @param add         If true, add strings in 2 to 1. If false, remove strings in 2 from 1.
 * @param var2Name    Name of the variable containing te lsit of things to add/remove.
 */
void
do_AddSubVariables(char *var1Name, int add, char *var2Name)
{
  Variable v1, v2;
  char **items;
  int i, n_strings_in_2;

  if (!(v1 = FindVariable(var1Name))) {
    cqpmessage(Error, "Variable $%s is not defined.", var1Name);
    return;
  }
  if (!(v2 = FindVariable(var2Name))) {
    cqpmessage(Error, "Variable $%s is not defined.", var2Name);
    return;
  }

  if (!(items = GetVariableStrings(v2, &n_strings_in_2)))
    /* v2 is empty, so do nothing */
    return;

  for (i = 0; i < n_strings_in_2; i++)
    if (add)
      VariableAddItem(v1, items[i]);
    else
      VariableSubtractItem(v1, items[i]);

  cl_free(items);
  /* the actual strings are pointers into the variable's internal representation, so don't free them */
}

/* ======================================== PARSER UTILS */

/**
 * Get ready to parse a command.
 *
 * This function is called before the processing of each parsed line that is
 * recognised as a command.
 *
 * Mostly it involves setting the global variables to their starting-state
 * values.
 */
void
prepare_input(void)
{
  regex_string_pos = 0;
  free_all_environments();

  generate_code = 1;
  searchstr = NULL;
  last_cyc = NoExpression;
  LastExpression = NoExpression;
}

/**
 * Expand the dataspace of a subcorpus.
 *
 * This is done, e.g., by the CQP-syntax "expand" command, to include context
 * into the matches found by a query.
 *
 * Each corpus interval stored in the CorpusList is extended by an amount, and in a direction,
 * dependant on the information in the global variable "expansion", a Context object
 * (information which has been put there by the parser).
 *
 * @see       expansion
 * @param cl  The subcorpus to expand.
 */
void
expand_dataspace(CorpusList *cl)
{
  int i, res;

  if (cl == NULL)
    cqpmessage(Warning, "The selected corpus is empty.");
  else if (cl->type == SYSTEM)
    cqpmessage(Warning, "You can only expand subcorpora, not system corpora (nothing has been changed)");
  else if (expansion.size > 0) {

    for (i = 0; i < cl->size; i++) {
      if (expansion.direction == ctxtdir_left || expansion.direction == ctxtdir_leftright) {
        res = calculate_leftboundary(cl,
                                     cl->range[i].start,
                                     expansion);
        if (res >= 0)
          cl->range[i].start = res;
        else
          cqpmessage(Warning, "'expand' statement failed (while expanding corpus interval leftwards).\n");
        /* when the expansion fails, the interval in the subcorpus is left as-is. */
      }
      if (expansion.direction == ctxtdir_right || expansion.direction == ctxtdir_leftright) {
        res = calculate_rightboundary(cl,
                                      cl->range[i].end,
                                      expansion);
        if (res >= 0)
          cl->range[i].end = res;
        else
          cqpmessage(Warning, "'expand' statement failed (while expanding corpus interval rightwards).\n");
        /* as per above: when the expansion fails, the interval in the subcorpus is left as-is. */
      }
    }

    apply_range_set_operation(cl, RUniq, NULL, NULL);

    /* the subcorpus is now unsaved, even if it previously was saved */
    cl->needs_update = True;
    cl->saved = False;
  }
}

/**
 * Prints out all the existing EvalEnvironments in the global array.
 *
 * @see Environment
 */
void
debug_output(void)
{
  int i;
  for (i = 0; i <= ee_ix; i++)
    show_environment(i);
}


/** Global variable for timing functions; not exported. @see do_start_timer @see do_timing */
struct timeval timer_start_time;

/**
 * Starts the timer running.
 */
void
do_start_timer(void)
{
  if (timing)
    gettimeofday(&timer_start_time, NULL);
}

/**
 * Shows the period since the timer started running.
 *
 * @param msg  A message to print along with the reading from the timer.
 */
void
do_timing(char *msg)
{
  struct timeval t;
  long delta_s;
  long delta_ms;

  if (timing) {
    gettimeofday(&t, NULL);
    delta_s = t.tv_sec - timer_start_time.tv_sec;
    delta_ms = (t.tv_usec - timer_start_time.tv_usec) / 1000;
    if (delta_ms < 0) {
      delta_s--;
      delta_ms = delta_ms + 1000;
    }
    cqpmessage(Info, "%s in %ld.%.3ld seconds\n", msg, delta_s, delta_ms);
  }
}



/**
 * Prints to stdout the size of a CorpusList (corpus/subcorpus/NQP).
 *
 * Implements the "size Something" command.
 *
 * @param cl      The (sub)corpus.
 * @param field   If this is TargetField or KeywordField, then the count
 *                of targets/keywords is printed. Otherwise the CL size
 *                is printed.
 */
void
do_size(CorpusList *cl, FieldType field)
{
  if (cl) {
    if (field != NoField) {
      if (field == TargetField) {
        int count = 0, i;
        if (cl->targets)
          for (i = 0; i < cl->size; i++)
            if (cl->targets[i] != -1)
              count++;
        Rprintf("%d\n", count);
      }
      else if (field == KeywordField) {
        int count = 0, i;
        if (cl->keywords)
          for (i = 0; i < cl->size; i++)
            if (cl->keywords[i] != -1)
              count++;
        Rprintf("%d\n", count);
      }
      else
        /* must be Match or MatchEnd then */
        Rprintf("%d\n", cl->size);
    }
    else
      Rprintf("%d\n", cl->size);
  }
  else
    /* undefined corpus */
    Rprintf("0\n");
}


/**
 * Dump subcorpus/NQR (or part of it) as TAB-delimited table of corpus positions.
 *
 * @param cl       The result (as a subcorpus, naturally)
 * @param first    Where in the result to begin dumping (index of cl->range)
 * @param last     Where in the result to end dumping (index of cl->range)
 * @param dst      Pointer to a Redir structure which contains information about
 *                 where to dump to.
 */
void
do_dump(CorpusList *cl, int first, int last, struct Redir *dst)
{
  int i, j, f, l, target, keyword;
  Range *rg;

  if (cl) {
    if (! open_rd_output_stream(dst, cl->corpus->charset)) {
      cqpmessage(Error, "Can't redirect output to file or pipe\n");
      return;
    }

    f = (first >= 0) ? first : 0;
    l = (last < cl->size) ? last : cl->size - 1;

    for (i = f; (i <= l) && !cl_broken_pipe; i++) {
      j = (cl->sortidx) ? cl->sortidx[i] : i;
      target  = (cl->targets)  ? cl->targets[j]  : -1;
      keyword = (cl->keywords) ? cl->keywords[j] : -1;
      rg = cl->range + j;
      Rprintf("%d\t%d\t%d\t%d\n", rg->start, rg->end, target, keyword);
    }

    close_rd_output_stream(dst);
  }
}


/**
 * Read TAB-delimited table of corpus positions and create named query result from it.
 *
 * acceptable values for extension_fields and corresponding row formats:
 * 0 = match [tab] matchend
 * 1 = match [tab] matchend [tab] target
 * 2 = match [tab] matchend [tab] target [tab] keyword
 */
int
do_undump(char *corpname, int extension_fields, int sort_ranges, struct InputRedir *src)
{
  int i, ok, size, match, matchend, target, keyword;
  int max_cpos, mark, abort;                /* for validity checks */
  char line[CL_MAX_LINE_LENGTH], junk[CL_MAX_LINE_LENGTH], mother[CL_MAX_LINE_LENGTH];
  CorpusList *cl = current_corpus, *new = NULL;

  assert(corpname != NULL);
  assert(extension_fields >= 0 && extension_fields <= 2);

  if (! valid_subcorpus_name(corpname)) {
    cqpmessage(Error, "Argument %s is not a valid query name.", corpname);
    return 0;
  }

  if (subcorpus_name_is_qualified(corpname)) {        /* if <corpname> is qualified, use specified mother corpus */
    corpname = split_subcorpus_name(corpname, mother); /* reset <corpname> to point to local name, copy qualifier to <mother> */
    if (!(cl = findcorpus(mother, SYSTEM, 0))) {
      cqpmessage(Error, "Can't find system corpus %s. Undump aborted.\n", mother);
      return 0;
    }
  }
  else {
    /* otherwise, check for activated corpus for which named query result will be created */
    if (!cl) {
      cqpmessage(Error, "No corpus activated. Can't perform undump.");
      return 0;
    }

    if (cl->type != SYSTEM) {        /* if a subcorpus is activated, find the corresponding system corpus */
      CorpusList *tmp;
      assert(cl->mother_name != NULL);
      if (NULL == (tmp = findcorpus(cl->mother_name, SYSTEM, 0))) {
        cqpmessage(Error, "Can't find implicitly activated corpus %s. Undump aborted.\n", cl->mother_name);
        return 0;
      }
      cl = tmp;
    } /* cl now points to the currently active system corpus */
  }

  new = make_temp_corpus(cl, "UNDUMP_TMP"); /* create temporary subcorpus that will hold the undump data */
  assert(new  && "failed to create temporary query result for undump");

  /* open input file, input pipe, or read from stdin */
  if (!open_rd_input_stream(src)) {
    /* error message should printed by open_input_stream() */
    drop_temp_corpora();
    return 0;
  }

  ok = 0;
  /* read undump table header = number of rows */
  if (fgets(line, CL_MAX_LINE_LENGTH, src->stream)) {
    if (1 == sscanf(line, "%d %s", &size, junk))
      ok = 1;
    else if (2 == sscanf(line, "%d %d", &match, &matchend)) {
      /* looks like undump file without line count => determine number of lines */
      if (src->stream == stdin)
        cqpmessage(Warning, "You must always provide an explicit line count if undump data is entered manually (i.e. read from STDIN)");
      else {
        /* undump file without header: count lines, then reopen stream */
        size = 1; /* first line is already in buffer */
        while (fgets(line, CL_MAX_LINE_LENGTH, src->stream))
          size++; /* dump files should not contain any long lines, so this count is correct */
        close_rd_input_stream(src);
        if (! open_rd_input_stream(src))
          cqpmessage(Warning, "Can't rewind undump file after counting lines: line count must be given explicitly");
        else
          ok = 1;
      }
    }
  }

  if (!ok) {
    cqpmessage(Error, "Format error in undump file: expecting number of rows on first line");
    close_rd_input_stream(src);
    drop_temp_corpora();
    return 0;
  }

  /* free previous match data (should only be one range for system corpus) */
  cl_free(new->range);
  cl_free(new->sortidx);
  cl_free(new->targets);
  cl_free(new->keywords);

  /* allocate space for required number of (match, matchend) pairs, targets, keywords */
  new->size = size;
  new->range = (Range *)cl_malloc(sizeof(Range) * size);
  if (extension_fields >= 1)
    new->targets = (int *)cl_malloc(sizeof(int) * size);
  if (extension_fields >= 2)
    new->keywords = (int *)cl_malloc(sizeof(int) * size);

  max_cpos = cl->mother_size - 1; /* check validity, ordering, and non-overlapping of match ranges */
  mark = -1;
  abort = 0;
  for (i = 0; (i < size) && !abort; i++) {        /* now read one data row at a time from the undump table */
    if (feof(src->stream)
        || !fgets(line, CL_MAX_LINE_LENGTH, src->stream) /* parse input line format */
        || sscanf(line, "%d %d %d %d %s", &match, &matchend, &target, &keyword, junk) != (2 + extension_fields)
        ) {
      cqpmessage(Error, "Format error in undump file (row #%d)", i+1);
      abort = 1;
      break;
    }

    /* check validity of match .. matchend range */
    if (matchend < match) {
      cqpmessage(Error, "match (%d) must be <= matchend (%d) on row #%d", match, matchend, i+1);
      abort = 1;
    }
    else if (match < 0 || matchend > max_cpos) {
      cqpmessage(Error, "match (%d .. %d) out of range (0 .. %d) on row #%d", match, matchend, max_cpos, i+1);
      abort = 1;
    }
    else if ((! sort_ranges) && (match <= mark)) { /* current range must start _after_ end of previous range (unless sort_ranges==1) */
      cqpmessage(Error, "matches must be sorted and non-overlapping\n\tmatch (%d) on row #%d overlaps with previous matchend (%d)", match, i+1, mark);
      abort = 1;
    }
    else {
      new->range[i].start = match;
      new->range[i].end = matchend;
      mark = matchend;
    }

    /* check validity of target position if specified */
    if (extension_fields >= 1) {
      if (target < -1 || target > max_cpos) {
        cqpmessage(Error, "target (%d) out of range (0 .. %d) on row #%d", target, max_cpos, i+1);
        abort = 1;
      }
      else
        new->targets[i] = target;
    }

    /* check validity of keyword position if specified */
    if (extension_fields >= 2) {
      if (keyword < -1 || keyword > max_cpos) {
        cqpmessage(Error, "keyword (%d) out of range (0 .. %d) on row #%d", keyword, max_cpos, i+1);
        abort = 1;
      }
      else
        new->keywords[i] = keyword;
    }
  }

  /* if an error was detected, don't create the named query result */
  if (abort) {
    close_rd_input_stream(src);
    drop_temp_corpora();
    return 0;
  }

  if (!close_rd_input_stream(src))  /* ignore trailing junk etc. */
    cqpmessage(Warning, "There may be errors in the undump data. Please check the named query result %s.\n", corpname);

  if (sort_ranges)         /* if ranges aren't sorted in natural order, they must be re-ordered so that CQP can work with them  */
    RangeSort(new, 1);     /* however, a sortidx is automatically constructed to reproduce the ordering of the input file */

  new = assign_temp_to_sub(new, corpname); /* copy the temporary subcorpus to the requested query name */
  drop_temp_corpora();

  if (!new) {
    cqpmessage(Error, "Couldn't assign undumped data to named query %s.\n");
    return 0;
  }

  return 1;
}



