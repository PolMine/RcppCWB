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
#include <glib.h>

#include "../cl/cl.h"

#include "concordance.h"
#include "attlist.h"
#include "options.h"

/** maximum length of a KWIC line (bytes). */
#define MAXKWICLINELEN 65535



/**
 * Reverses the argument string (destructively, that is, in situ).
 *
 * Cf. the non-standard (microsoft) function strrev.
 *
 * This does not respect UTF-8, so anything reversed *must* be re-reversed
 * before output or there will be invalid byte sequences.
 *
 * @param s  The string to modify.
 * @return   A pointer to the modified string (same memory area as
 *           the argument string).
 */
static char *
srev(char *s)
{
  register char buf;
  register int i, l;

  l = strlen(s);

  for (i = 0; i < l/2; i++) {
    buf = s[l-i-1];
    s[l-i-1] = s[i];
    s[i] = buf;
  }

  return s;
}



/**
 * Rendering function for the start of a concordance line (before any left-context):
 * implements leading cpos and print-structures.
 *
 * The s-attribute values are put into a ClAutoString, for use at the start of the printed concordance line.
 *
 * The s-attributes that will be printed are determined by the contents of the ContextDescriptor
 * object; the PrintDescriptionRecord determines what they look like.
 *
 * The leading material generated here refers to a single given cpos ("position").
 *
 * @param cd                   Print settings (context size/type; atts to print)
 * @param position             The CPOS about which this function renders information.
 * @param s                    Results will be concatenated onto this string.
 * @param show_cpos            Boolean: whether or not to print the cpos before the PrintStructures.
 * @param pdr                  Print settings (of main mode) to use
 */
static void
compose_kwic_print_structures(ContextDescriptor *cd,
                              int position,
                              ClAutoString s,
                              int show_cpos,
                              PrintDescriptionRecord *pdr)
{
  if (show_cpos && pdr->CPOSPrintFormat) {
    static char rendered_cpos[CL_MAX_LINE_LENGTH];  /* another 'Oli': this was num[16], definitely not enough for HTML output */
    snprintf(rendered_cpos, CL_MAX_LINE_LENGTH, pdr->CPOSPrintFormat, position);
    cl_autostring_concat(s, rendered_cpos);
  }

  if (cd->printStructureTags) {
    AttributeInfo *ai;
    int pref_printed = 0; /* boolean: has the prefix been printed? */

    for (ai = cd->printStructureTags->list; ai; ai = ai->next) {
      const char *v;
      if (ai->status) {
        assert(ai->attribute);
        if (!pref_printed) {
          cl_autostring_concat(s, pdr->BeforePrintStructures);
          pref_printed = 1;
        }

        cl_autostring_concat(s, pdr->StructureBeginPrefix);
        cl_autostring_concat(s,  pdr->printToken  ?  pdr->printToken(ai->attribute->any.name)  :  ai->attribute->any.name);

        /* print value */
        v = cl_cpos2struc2str(ai->attribute, position);
        if (v && pdr->printToken)
          v = pdr->printToken(v);

        if (v) {
          cl_autostring_concat(s, pdr->PrintStructureSeparator);
          cl_autostring_concat(s, v);
        }
        cl_autostring_concat(s, pdr->StructureBeginSuffix);
      }
    }
    if (pref_printed)
      cl_autostring_concat(s, pdr->AfterPrintStructures);
  }
}



/* ============================== helpers for: get_position_values() */
/* the following code is borrowed from <utils/decode.c> and ensures that XML tags in the kwic output always nest properly */
/* TODO avoid duplication of code!! */

#define MAX_S_ATTRS 1024        /* max. number of s-attribute; same as MAX_ATTRS in <utils/decode.c> and MAXRANGES in <utils/encode.c>  */
typedef struct {
  char *name;                   /* name of the s-attribute */
  int start;
  int end;
  char *annot;                  /* NULL if there is no annotation */
} SAttRegion;
SAttRegion s_att_regions[MAX_S_ATTRS];
int sar_sort_index[MAX_S_ATTRS]; /* index used for bubble-sorting list of regions */
int N_sar = 0;                   /* number of regions currently in list (may change for each token printed) */

static void
sort_s_att_regions(void)
{
  int i, temp, modified;

  for (i = 0; i < N_sar; i++)   /* initialise sort index */
    sar_sort_index[i] = i;

  modified = 1;                 /* repeat 'bubble' loop until no more modifications are made */
  while (modified) {
    modified = 0;
    for (i = 0; i < (N_sar-1); i++) {
      SAttRegion *a = &(s_att_regions[sar_sort_index[i]]); /* compare *a and *b */
      SAttRegion *b = &(s_att_regions[sar_sort_index[i+1]]);

      if ( a->end < b->end || (a->end == b->end && a->start > b->start) ) {
        /* swap sar_sort_index[i] and sar_sort_index[i+1] */
        temp = sar_sort_index[i];
        sar_sort_index[i] = sar_sort_index[i+1];
        sar_sort_index[i+1] = temp;
        modified = 1;           /* modified ordering, so we need another loop iteration */
      }
    }
  }

  return;
}


/**
 * Assemble all the bits of a KWIC token (i.e., XML begin/end tags and p-attributes
 * as selected with "show +...") at the given corpus position.
 *
 * @param cd        ContextDescriptor for the concordance.
 * @param position  cpos of token to render
 * @param dest      ClAutoString to place the result in (overwriting any existing content).
 * @param pdr       PrintDescriptionRecord for the concordance.
 */
static void
compose_kwic_token(ContextDescriptor *cd, int position, ClAutoString dest, PrintDescriptionRecord *pdr)
{
  AttributeInfo *ai;
  int i;
  int nr_attrs = 0;
  const char *word;

  char *att_sep = pdr->AttributeSeparator;
  if (*attribute_separator != '\0')
    att_sep = attribute_separator;

  cl_autostring_truncate(dest, 0);

  /* insert all dest-attribute regions which start or end at the current token into s_att_regions[],
     then sort them to ensure proper nesting, and print from the list */
  N_sar = 0;
  if (cd->strucAttributes) {
    for (ai = cd->strucAttributes->list; ai; ai = ai->next)
      if (ai->status) {
        int s_start, s_end, snum;

        if ( ((snum = cl_cpos2struc(ai->attribute, position)) >= 0) &&
             (cl_struc2cpos(ai->attribute, snum, &s_start, &s_end)) &&
             ((position == s_start) || (position == s_end)) ) {

          s_att_regions[N_sar].name = ai->attribute->any.name;
          s_att_regions[N_sar].start = s_start;
          s_att_regions[N_sar].end = s_end;
          if (cl_struc_values(ai->attribute))
            s_att_regions[N_sar].annot = cl_struc2str(ai->attribute, snum);
          else
            s_att_regions[N_sar].annot = NULL;
          N_sar++;
          }

        }
    sort_s_att_regions();
  } /* else N_sar == 0 */


  /* ==================== first, add before-token starting structures */
  if (cd->strucAttributes) {
    /* print open tags from s_att_regions[] (ascending) */
    SAttRegion *region;
    for (i = 0; i < N_sar; i++) {
      region = &(s_att_regions[sar_sort_index[i]]);
      if (region->start == position) {
        /* add start tag to dest */
        static char body[CL_MAX_LINE_LENGTH]; /* 'body' of the start tag, may include annotation  */

        if (show_tag_attributes && region->annot)
          snprintf(body, CL_MAX_LINE_LENGTH, "%s %s", region->name, region->annot);
        else
          cl_strcpy(body, region->name);

        cl_autostring_concat(dest, structure_delimiter);
        cl_autostring_concat(dest, pdr->StructureBeginPrefix);
        cl_autostring_concat(dest, pdr->printToken ? pdr->printToken(body) : body);
        cl_autostring_concat(dest, pdr->StructureBeginSuffix);
        cl_autostring_concat(dest, structure_delimiter);
      }
    }
  }

  /* ==================== then, add p-attribute values */
  for (ai = cd->attributes->list; ai; ai = ai->next) {
    if (ai->attribute && ai->status > 0) {
      if (nr_attrs > 0)
        cl_autostring_concat(dest, att_sep);

      /* pass the p-att value through printToken */
      if (NULL != (word = cl_cpos2str(ai->attribute, position)))
        cl_autostring_concat(dest, pdr->printToken ? pdr->printToken((char *)word) : word);

      nr_attrs++;
    }
  }

  /* ==================== finally, add ending structures */
  /* print close tags from s_att_regions[] (descending) */
  if (cd->strucAttributes) {
    SAttRegion *region;
    for (i = N_sar - 1; i >= 0; i--) {
      region = &(s_att_regions[sar_sort_index[i]]);
      if (region->end == position) {
        /* add end tag to s */
        cl_autostring_concat(dest, structure_delimiter);
        cl_autostring_concat(dest, pdr->StructureEndPrefix);
        cl_autostring_concat(dest, pdr->printToken ? pdr->printToken(region->name) : region->name);
        cl_autostring_concat(dest, pdr->StructureEndSuffix);
        cl_autostring_concat(dest, structure_delimiter);
      }
    }
  }
}


/**
 * Scratch string used by compose_anchor_separators only.
 * Global so that Kwic cleanup can catch it!
 */
static ClAutoString scratch = NULL;

/**
 * This oddly-named function prints a series of separators for "fields" to an internal buffer.
 *
 * "Field" in this context means an anchor point (begin, end, target, keyword).
 *
 * @param position   The corpus position (cpos) whose field-separators we want.
 * @param fields     Pointer to array of ConcLineFields object (each of which specifies one of the 4 anchors).
 * @param nr_fields  Number of items in the "fields" array.
 * @param at_end     Boolean: if true, we get the end-separators needed to print at this cpos (after the token printed);
 *                   if false, we get the beginning-separators needed to print at this cpos (before the token printed).
 * @param pdr        The PDR for the current concordance printout.
 * @return           A pointer to a module-internal static string buffer containing
 *                   the requested string (sequence of start or end separators
 *                   for the anchors specified in "fields"). Do not free it or alter it.
 *                   The buffer's content will change when this function is called again.
 *                   The function will return NULL if the requested string would have
 *                   been zero-length.
 */
static const char *
compose_anchor_separators(int position,
                          ConcLineField *fields,
                          int nr_fields,
                          int at_end,
                          PrintDescriptionRecord *pdr)
{
  int i;

  /* start with an empty string ... */
  if (scratch)
    cl_autostring_truncate(scratch, 0);
  else
    scratch = cl_autostring_new(NULL, 0);

  if (!(fields && nr_fields > 0 && position >= 0 && pdr && pdr->printField))
    return NULL;

  if (at_end) {
    for (i = nr_fields; i > 0; i--)
      if (position == fields[i-1].end_position)
        /* note call to the "business end" which is the func pointer in the PDR */
        cl_autostring_concat(scratch, pdr->printField(fields[i-1].type, at_end));
  }
  else
    /* if not at_end then... */
    for (i = 0; i < nr_fields; i++)
      if (position == fields[i].start_position)
        /* note call to the "business end" which is the func pointer in the PDR */
        cl_autostring_concat(scratch, pdr->printField(fields[i].type, at_end));

  return  scratch->len > 0 ? scratch->data : NULL;
}



/*
 * TWO GLOBAL VARIABLES for concordance line construction, used only within compose_kwic_line();
 * they are global so that, once their buffers auto-expand, they will stay expanded for the
 * remainder of the CQP session -- no sense freeing memory when a user has already shown they
 * are inclined to make use of it and will probably request an equally long string soon after !
 */

/** Used to build the concordance line (main line buffer); @see compose_kwic_line */
static ClAutoString line  = NULL;
/** Used to build the concordance line (token buffer);     @see compose_kwic_line */
static ClAutoString token = NULL;

/**
 * Initialises the two empty auto-growing strings used for concordance line concstruction.
 */
static void
setup_kwic_line_memory(void)
{
  if (!line)
    line  = cl_autostring_new(NULL, 0);
  else
    cl_autostring_truncate(line, 0);

  if (!token)
    token = cl_autostring_new(NULL, 0);
  else
    cl_autostring_truncate(line, 0);
}

/**
 * Frees the memory used for building a KWIC line for display.
 *
 * Best used when CQP shuts down.
 */
void
cleanup_kwic_line_memory(void)
{
  cl_autostring_delete(line);
  cl_autostring_delete(token);
  cl_autostring_delete(scratch);
}

/**
 * Builds a string for a concordance output line.
 *
 * @param corpus             The corpus we're pritning from
 * @param match_start        A corpus position (start of match)
 * @param match_end          A corpus position (end of match)
 * @param cd                 ContrextDescriptor to use for printing.
 * @param left_kwic_delim    String inserted at start of match.
 * @param right_kwic_delim   String inserted at end of match. THIS AND THE PREVIOUS ARE ONLY USED BY ascii-print./... which passes in global the global options set by LeftKWICDelim and RightKWICDelim
 *                           So actually, these could be just 1 param: include_global_hit_match_delimiters
 * @param fields             Array of ConcLineFields object (that is, anchors).
 * @param nr_fields          Number of items in the "fields" array.
 * @param pdr                PDR contains the strings to be printed among bits of data from the ciorpus to make the conconrdance.
 * @return                   String containing the output line.
 */
char *
compose_kwic_line(Corpus *corpus,
                  int match_start,
                  int match_end,
                  ContextDescriptor *cd,
                  char *left_kwic_delim,
                  char *right_kwic_delim,
                  ConcLineField *fields,
                  int nr_fields,
                  PrintDescriptionRecord *pdr)
{
  /* length of the string assembled, in bytes */
  size_t length_bytes;

  /* length of the string assembled, in characters; == length_bytes in all cases other than charset==utf8 */
  size_t length_characters;

  int old_len;  /* Old length - for tracking the n bytes added by a concatenate operation w/o strlen() calls */

  /* total N of tokens on the attribute (IE the first on the list of attributes-to-print);
   * used as a maximum for sanity-check */
  int text_size;
  int start, end, begin_reverse_ix;
  int rng_s, rng_e, rng_n, nr_ranges;
  int token_length_characters;

  int enough_context = 0;       /* Boolean: should we keep adding or not? */

  AttributeInfo *ai;

  size_t tok_sep_len_bytes, tok_sep_len_chars;
  char *rev_tok_sep;

  /* implement CQP option TokenSeparator to override the PDR's tok sep if the global option is not empty. */
  char *tok_sep = token_separator[0] ? token_separator : pdr->TokenSeparator;

  /* speedup vars for character-mode left context */
  tok_sep_len_bytes = strlen(tok_sep);
  tok_sep_len_chars = cl_charset_strlen(corpus->charset, tok_sep);
  rev_tok_sep = cl_strdup(tok_sep);
  srev(rev_tok_sep);

  /** Use with variables "line", "tok_sep" to insert a separator iff there are already 1+ tokens in the line; macro used only in this function  */
# define add_sep_to_line_if_not_empty(l, s) {if (l->len > 0) cl_autostring_concat(l, s);}

  /* set up our two buffers as empty strings ... */
  setup_kwic_line_memory();

  /* make a dummy attribute list (with default p-attribute as its only member) iff we don't yet have one. */
  if (!(cd->attributes && cd->attributes->list)) {
    DestroyAttributeList(&(cd->attributes)); /* avoid mem leak if cd->attributes existed, but had no ->list; call is NULL-safe */
    cd->attributes = NewAttributeList(ATT_POS);
    AddNameToAL(cd->attributes, CWB_DEFAULT_ATT_NAME, 1, 0);
  }
  if (!cd->attributes->list_valid)
    VerifyList(cd->attributes, corpus, 1);

  /* if nothing is selected to "show", select CWB_DEFAULT_ATT_NAME */
  if (0 == NrOfElementsWithStatusAL(cd->attributes, 1)) {
    if ((ai = FindInAL(cd->attributes, CWB_DEFAULT_ATT_NAME)))
      ai->status = 1;
    else {
      Rprintf("ERROR: Failed to activate printing of default attribute ('show +%s')\n", CWB_DEFAULT_ATT_NAME);
      return NULL;
    }
  }

  assert(cd->attributes->list->attribute);
  text_size = cl_max_cpos(cd->attributes->list->attribute);

  /* assert sane values for match_start and match_end */
  assert(match_start >= 0 && match_start < text_size);
  assert(match_end >= 0 && match_end < text_size && match_end >= match_start);

  /* WE ARE NOW READY TO START BUILDING US A KWIC LINE !!! Hurray! */
  compose_kwic_print_structures(cd, match_start, line, cd->print_cpos, pdr);
  cl_autostring_concat(line, pdr->BeforeField);

  /* ==========================================================================
   * first big job: use designated method to put together the left-hand co-text
   * ========================================================================== */

  switch(cd->left_type) {

  case char_context:
    /* we have 0 characters in the left co-text so far */
    length_bytes = length_characters = 0;
    enough_context = 0;

    /* make a note of the old starting point, for purposes of srev() */
    begin_reverse_ix = line->len;

    for (start = match_start - 1; (start >= 0 && !enough_context); start--) {
      /* loop across tokens starting from the tok before the match and then moving backwards */
      if (length_characters >= cd->left_width)
        enough_context = 1;
      else {
        /* we do not yet have enough context, so get position values into (blank) string object token */
        compose_kwic_token(cd, start, token, pdr);
        token_length_characters = cl_charset_strlen(corpus->charset, token->data);
        if (token_length_characters > 0) {
          int this_token_start_offset;

          /* TOKEN SEPARATOR */

          if (line->len > begin_reverse_ix) {
            /* we already have some material in the "reversible" part of the line, so add the token separator */
            old_len = line->len;
            if (tok_sep_len_chars + length_characters < cd->left_width) {
              /* add the WHOLE token separator */
              cl_autostring_concat(line, rev_tok_sep);
              length_bytes += tok_sep_len_bytes;
              length_characters += tok_sep_len_chars;
              /* token separator is added in reverse order so that subsequent reverse of whole string won't break it. */
            }
            else {
              /* we only add the END of the token separator */
              char *mark = tok_sep;
              int chars_to_skip = tok_sep_len_chars - (cd->left_width - length_characters);
              if (corpus->charset != utf8)
                mark += chars_to_skip;
              else
                while (chars_to_skip--)
                  mark = g_utf8_next_char(mark);
              /* so now we can copy and then reverse (since we wrote from tok_sep not rev_tok_sep) */
              cl_autostring_concat(line, mark);
              length_bytes += line->len - old_len;
              length_characters += cl_charset_strlen(corpus->charset, mark);
              if (1 < line->len - old_len)
                srev(line->data + old_len); /* no need to reverse just one byte */
            }
          }
          /* else : no token separator before first token to the left of match */

          /* TOKEN ITSELF */

          this_token_start_offset = line->len; /* first byte of this token EXCLUDING separator */

          /* We build the string as normal to begin with, but then reverse it */
          cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 0, pdr));
          cl_autostring_concat(line, pdr->BeforeToken);

          if (token_length_characters + length_characters < cd->left_width) {
            /* move the whole token to the line. */
            cl_autostring_concat(line, token->data);
            length_bytes += token->len;
            length_characters += token_length_characters;
          }
          else {
            /* not enough space for the whole token */
            char *mark;
            if (corpus->charset != utf8) {
              /* ... so only copy its last several bytes */
              mark = token->data + (token->len - (cd->left_width - length_characters));
            }
            else {
              /* ... so only copy its last several characters */
              mark = token->data;
              while (token_length_characters + length_characters > cd->left_width) {
                mark = g_utf8_next_char(mark);
                --token_length_characters;
              }
            }
            old_len = line->len;
            cl_autostring_concat(line, mark);
            length_bytes += line->len - old_len;
            length_characters += cl_charset_strlen(corpus->charset, mark);
          }

          cl_autostring_concat(line, pdr->AfterToken);
          cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 1, pdr));

          /* switch round the order of everything added AFTER the tok_sep */
          srev(line->data + this_token_start_offset);

          /* TOKEN (WITH PRECEDING SEP) NOW COMPLETE */
        }
        else
          enough_context = 1;
        /* that is, if calling get_position_values() did not generate any more context,
         * then simply declare that we have enough context! */
      }
    } /* endfor */

    /* pad with blanks until we reach the designated width for left co-text */
    while (length_characters < cd->left_width) {
      cl_autostring_concat(line, " ");
      ++length_bytes, ++length_characters;
    }

    /* Now we need to reverse order of characters of the left co-text assembled in the preceding code;
     * but we DON'T flip any printStructures */
#if 0
    Rprintf("line before srev(): >>%s<<\n", line + begin_reverse_ix);
#endif

    srev(line->data + begin_reverse_ix);

#if 0
    Rprintf("line after srev(): >>%s<<\n", line + begin_reverse_ix);
#endif
    break; /* endcase CHAR_CONTEXT */

  case word_context:
    start = match_start - cd->left_width;
    if (start < 0)
      start = 0;
    for ( ; start < match_start; start++) {
      cl_autostring_truncate(token, 0);
      compose_kwic_token(cd, start, token, pdr);
      add_sep_to_line_if_not_empty(line, tok_sep);
      cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 0, pdr));
      cl_autostring_concat(line, pdr->BeforeToken);
      cl_autostring_concat(line, token->data);
      cl_autostring_concat(line, pdr->AfterToken);
      cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 1, pdr));
    }
    break;

  case s_att_context:
  case a_att_context:
    if (!cd->left_structure) {
      Rprintf("concordance.o/compose_kwic_line: left context attribute pointer is NULL\n");
      start = match_start - 20;
      if (start < 0)
        start = 0;
    }
    else {
      if (cd->left_type == a_att_context) {
        /* context == alignment block */
        if (0 > (rng_n = cl_cpos2alg(cd->left_structure, match_start)))
          start = match_start;
        else {
          assert(cd->left_width == 1);
          /* get start of source corpus alignment block */
          if (!cl_alg2cpos(cd->left_structure, rng_n, &rng_s, &rng_e, &rng_e, &rng_e))
            start = match_start;
          else
            start = rng_s;
        }
      }
      else {
        /* context == structural region(s) */
        if (0 > (rng_n = cl_cpos2struc(cd->left_structure, match_start)))
          start = match_start - 20;
        else {
          assert(cd->left_width >= 0);

          /* determine the lower range number */
          rng_n = MAX(0, rng_n - cd->left_width + 1);
          if (!cl_struc2cpos(cd->left_structure, rng_n, &rng_s, &rng_e))
            start = match_start - 20;
          else
            start = rng_s;
        }
      }
    }

    if (start < 0)
      start = 0;

    for ( ; start < match_start; start++) {
      cl_autostring_truncate(token, 0);
      compose_kwic_token(cd, start, token, pdr);
      add_sep_to_line_if_not_empty(line, tok_sep);
      cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 0, pdr));
      cl_autostring_concat(line, pdr->BeforeToken);
      cl_autostring_concat(line, token->data);
      cl_autostring_concat(line, pdr->AfterToken);
      cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 1, pdr));
    }
    break;
  }


  /* =======================================================================
   * Left-hand co-text is now fully built, so now we insert the match tokens
   * ======================================================================= */

  add_sep_to_line_if_not_empty(line, tok_sep);
  cl_autostring_concat(line, left_kwic_delim);
  for (start = match_start; start <= match_end; start++) {
    cl_autostring_truncate(token, 0);
    compose_kwic_token(cd, start, token, pdr);
    cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 0, pdr));
    cl_autostring_concat(line, pdr->BeforeToken);
    cl_autostring_concat(line, token->data);
    cl_autostring_concat(line, pdr->AfterToken);
    cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 1, pdr));
    if (start != match_end)
      add_sep_to_line_if_not_empty(line, tok_sep);
  }
  cl_autostring_concat(line, right_kwic_delim);


  /* ======================================================
   * now all we have to do is add on the right-hand co-text
   * ====================================================== */

  switch(cd->right_type) {

  case char_context:
    /* we have 0 characters so far */
    length_bytes = length_characters = 0;
    enough_context = 0;

    for (start = match_end + 1; (start < text_size && !enough_context); start++) {
      cl_autostring_truncate(token, 0);
      if (length_characters >= cd->right_width)
        enough_context++; /* stop if the requested number of characters have been generated */
      else {
        compose_kwic_token(cd, start, token, pdr);
        if (line->len > 0) {
          old_len = line->len;
          cl_autostring_concat(line, tok_sep);
          length_bytes += line->len - old_len;
          length_characters += cl_charset_strlen(corpus->charset, tok_sep);
        }
        /* else: don't add token separator initially in line */

        cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 0, pdr));
        cl_autostring_concat(line, pdr->BeforeToken);

        old_len = line->len;
        cl_autostring_concat(line, token->data);
        length_bytes += line->len - old_len;
        length_characters += cl_charset_strlen(corpus->charset, token->data);
        if (length_characters > cd->right_width) {
          if (corpus->charset != utf8) {
            /* simple truncation by count of bytes */
            cl_autostring_truncate(line, line->len - (length_characters - cd->right_width));
            length_characters = length_bytes = cd->right_width;
          }
          else {
            /* rewind through the string until we have removed the correct n of characters */
            char *mark = line->data + (line->len - 1);
            while (length_characters > cd->right_width) {
              while (cl_string_utf8_continuation_byte(*mark))
                --mark;
              --length_characters, --mark;
            }
            cl_autostring_truncate(line, (mark - line->data + 1));
          }
        }
        cl_autostring_concat(line, pdr->AfterToken);
        cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 1, pdr));
      }
    }
    break;

  case word_context:
    for (start = 1 ; start <= cd->right_width && match_end + start < text_size ; start++) {
      cl_autostring_truncate(token, 0);
      compose_kwic_token(cd, match_end + start, token, pdr);
      add_sep_to_line_if_not_empty(line, tok_sep);
      cl_autostring_concat(line, compose_anchor_separators(match_end + start, fields, nr_fields, 0, pdr));
      cl_autostring_concat(line, pdr->BeforeToken);
      cl_autostring_concat(line, token->data);
      cl_autostring_concat(line, pdr->AfterToken);
      cl_autostring_concat(line, compose_anchor_separators(match_end + start, fields, nr_fields, 1, pdr));
    }
    break;

  case s_att_context:
  case a_att_context:
    if (!cd->right_structure) {
      Rprintf("concordance.o/compose_kwic_line: right context attribute pointer is NULL\n");
      end = match_end + 20;
    }
    else {
      if (cd->right_type == a_att_context) {
        /* context == alignment block */
        if (0 > (rng_n = cl_cpos2alg(cd->right_structure, match_end)))
          end = match_end;
        else {
          assert(cd->right_width == 1);
          /* get end of source corpus alignment block */
          if (!cl_alg2cpos(cd->right_structure, rng_n, &rng_s, &rng_e, &rng_s, &rng_s))
            end = match_end;
          else
            end = rng_e;
        }
      }
      else {
        /* context == structural region(s) */
        if (0 > (rng_n = cl_cpos2struc(cd->right_structure, match_end)))
          end = match_end + 20;
        else {
          assert(cd->right_width >= 0);
          /* determine the upper range number */
          if (0> (nr_ranges = cl_max_struc(cd->right_structure)))
            end = match_end + 20;
          else {
            rng_n = MIN(nr_ranges-1, rng_n + cd->right_width - 1);
            if (!cl_struc2cpos(cd->right_structure, rng_n, &rng_s, &rng_e))
              end = match_end + 20;
            else
              end = rng_e;
          }
        }
      }
    }
    if (match_end >= text_size)
      match_end = text_size - 1;

    for (start = match_end + 1; start <= end; start++) {
      cl_autostring_truncate(token, 0);
      compose_kwic_token(cd, start, token, pdr);
      add_sep_to_line_if_not_empty(line, tok_sep);
      cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 0, pdr));
      cl_autostring_concat(line, pdr->BeforeToken);
      cl_autostring_concat(line, token->data);
      cl_autostring_concat(line, pdr->AfterToken);
      cl_autostring_concat(line, compose_anchor_separators(start, fields, nr_fields, 1, pdr));
    }
    break;
  }

  cl_autostring_concat(line, pdr->AfterField);

  /*
   * OK, the line is complete!
   */

  cl_free(rev_tok_sep);

  return cl_strdup(cl_autostring_ptr(line));

# undef add_sep_to_line_if_not_empty
}

