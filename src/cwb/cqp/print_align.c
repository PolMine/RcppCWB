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

#include "../cl/cl.h"

#include "concordance.h"

#include "cqp.h"
#include "options.h"
#include "corpmanag.h"
#include "print-modes.h"
#include "context_descriptor.h"

#include "ascii-print.h"
#include "html-print.h"
#include "sgml-print.h"
#include "latex-print.h"



/**
 * Global context descriptor used solely for printing
 * corresponding strings from an aligned corpus.
 *
 * (It exists because the amount fo left/right context to print
 * is not unlikely to be different for the aligned line
 * than for the main KWIC line.)
 */
static ContextDescriptor AlignedCorpusCD;


/**
 * Initialises the alignment-printing module by setting up the
 * static global ContextDescriptor (internal to print_align module).
 *
 * If module initialisation has already run, does nothing.
 *
 * @see AlignedCorpusCD
 */
void
init_align_module()
{
  /* Flag for whether the "print_align" module has been initialised yet or not. */
  static int module_init = 0;

  if (!module_init) {
    initialize_context_descriptor(&AlignedCorpusCD);
    AlignedCorpusCD.left_type  = word_context;
    AlignedCorpusCD.right_type = word_context;
    module_init = 1;
  }
}



/* TODO
 * THIS SHOULD REALLY BE REWRITTTEN SO THAT THE ALIGNED CORPUS, ITS ATTRIBUTES, AND
 * THE (ALIGNED) CD ARE ONLY INITIALISED ONCE (FOR EACH CAT COMMAND)
 */
/**
 * For a given query result, prints the corresponding section of the
 * aligned corpus (if any).
 *
 * This function is the business-end of the "print_align" module.
 *
 * @param sourceCorpus  The corpus the query was run on.
 * @param cd            ContextDescriptor containing data on how the concordance is to be printed.
 * @param begin_target  Starting cpos of the result being printed.
 * @param end_target    Ending cpos of the result being printed.
 * @param highlighting  Boolean: Iff true, highlighting will be used
 *                      (applies only in ASCII print mode; see "ascii_print_aligned_line").
 * @param stream        Output destination stream (will be printed to).
 */
void
print_all_aligned_lines(Corpus *sourceCorpus,
                        ContextDescriptor *cd,
                        int begin_target,
                        int end_target,
                        int highlighting,
                        FILE *stream)
{
  int dummy;
  AttributeInfo *ai, *Sai, *Tai;
  Attribute *align_att;
  Corpus *aligned_corpus;
  PrintDescriptionRecord *which_pdr = NULL;
  print_aligned_line_func print_aligned_line = NULL;

  init_align_module();

  if ( ! (cd && cd->alignedCorpora && cd->alignedCorpora->list) )
    return;

  for (ai = cd->alignedCorpora->list; ai && ai->name; ai = ai->next) {
    /* ai->status = is the attribute activated for printing?
     * if it isn't then skip it. */
    if (!ai->status)
      continue;

    /* get the corpus positions for the aligned corpus  */
    if ((align_att = cl_new_attribute(sourceCorpus, ai->name, ATT_ALIGN))
        &&
        /* if it isn't already there, load it (cl_new_corpus() will just increment the refcount, if the corpus is already loaded) */
        (aligned_corpus = cl_new_corpus(registry, ai->name))
        ) {
      int alg1, alg2, alg_start, alg_end;
      char *s = NULL;
      int sanitise_aligned_data = 0; /* bool: if true, allow only ascii chars in aligned output */

      /* Do we need to recode contents of the aligned corpus? */
      {
        /* the "comparison" charset is never ascii; this is to make sure that utf8/ascii corpora are interchangeable both ways */
        /* -- NB: if output is via a pager, LESSCHARSET will be set to UTF8 if the source corpus is ascii; vide cqp/output.c */
        CorpusCharset compare = (sourceCorpus->charset == ascii ? utf8 : sourceCorpus->charset);
        if (aligned_corpus->charset != ascii && aligned_corpus->charset != compare)
          sanitise_aligned_data = 1;
      }

      alg1 = cl_cpos2alg(align_att, begin_target);
      alg2 = cl_cpos2alg(align_att, end_target);

      if (alg1 < 0 || alg2 < 0 ||
          !cl_alg2cpos(align_att, alg1, &dummy, &dummy, &alg_start, &dummy) ||
          !cl_alg2cpos(align_att, alg2, &dummy, &dummy, &dummy, &alg_end)  ||
          alg_end < alg_start
          ) {
        s = cl_strdup("(no alignment found)");
        /* so after here, s != NULL signifies error or no alignment found;
         * this is checked before attempting to build alignment strings below. */
      }

      /* For some obscure reason, the AlignedCorpusCD sometimes gets corrupted
       * from outside; so we re-initialize for every aligned corpus we print. */
      initialize_context_descriptor(&AlignedCorpusCD);
      update_context_descriptor(aligned_corpus, &AlignedCorpusCD);

      /* Try to show the same attributes in this corpus as in the aligned corpus */
      /* positional attributes */
      if (cd->attributes)
        for (Sai = cd->attributes->list; Sai && Sai->name; Sai = Sai->next)
          if ((Tai = FindInAL(AlignedCorpusCD.attributes, Sai->name)))
            Tai->status = Sai->status;

      /* structural attributes */
      if (cd->strucAttributes)
        for (Sai = cd->strucAttributes->list; Sai && Sai->name; Sai = Sai->next)
          if ((Tai = FindInAL(AlignedCorpusCD.strucAttributes, Sai->name)))
            Tai->status = Sai->status;

      /* printing structural attribute values in the aligned regions doesn't
       * seem to make a lot of sense, so we stick with the first two options */

      switch (GlobalPrintMode) {

      case PrintSGML:
        which_pdr = &SGMLPrintDescriptionRecord;
        print_aligned_line = sgml_print_aligned_line;
        break;

      case PrintHTML:
        which_pdr = &HTMLPrintDescriptionRecord;
        print_aligned_line = html_print_aligned_line;
        break;

      case PrintLATEX:
        which_pdr = &LaTeXPrintDescriptionRecord;
        print_aligned_line = latex_print_aligned_line;
        break;

      case PrintASCII:
      case PrintUNKNOWN:
      default:
        which_pdr = &ASCIIPrintDescriptionRecord;
        print_aligned_line = ascii_print_aligned_line;
        break;
      }

      if (!s) {
        s = compose_kwic_line(aligned_corpus,
                              alg_start, alg_end,
                              &AlignedCorpusCD,
                              NULL, NULL,
                              NULL, 0,
                              which_pdr);
        if (s && sanitise_aligned_data)
          cl_string_validate_encoding(s, ascii, 1);
      }
      print_aligned_line(stream, highlighting, ai->name, s ? s : "(null)");

      cl_free(s);
      /* don't drop the aligned corpus even if we're the only one using it;
       * this may waste some memory, but otherwise we'd keep loading
       * and unloading the thing */
    }
  }
}


