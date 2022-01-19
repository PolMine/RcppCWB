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

#ifndef _cqp_context_descriptor_h_
#define _cqp_context_descriptor_h_


#include "attlist.h"


/**
 * Enumeration of the four different ways to measure the width of co-text in a concordance line.
 */
typedef enum _context_type {
  /** Context width measured in characters */
  char_context  = -1,
  /** Context width measured in tokens */
  word_context  = -2,
  /** Context width measured in terms of an s-attribute */
  s_att_context = -3,
  /** Context width measured in terms of an a-attribute - that is, alignment blocks as the unit of context */
  a_att_context = -4

} context_type;

/**
 * ContextDescriptor class object: a bundle of CQP options
 * describing how a list of corpus positions is to be
 * displayed in a concordance: with left context,
 * with right context, with what attributes, etc.
 *
 * It is passed around between different print functions
 * so that they know what to do!
 *
 * Note that the options contained here are settable
 * by the user. This is in contrast to the "options"
 * held in the PrintDecriptionRecord, which are built-in
 * for each print style; the user can choose among modes
 * but cannot modify the settings individually.
 *
 * This object is confusingly named, as it DOES NOT
 * merely specify the "Context" size; it ALSO specifies
 * which attributes get printed, and so on.
 * (It would be better called a "concordance line co-text
 * configuration object". Or even just a ConcordanceDescriptor.)
 *
 * @see PrintDescriptionRecord
 * @see PrintOptions
 *
 * A PDR contains strings for "concordance building bits" a the CD
 * contains settings controlling which attributes/how much co-text to show.
 */
typedef struct {

  /* ==================== left context scope description variables */

  int left_width;                    /**< Amount of context to show before the match, in units specified by left_type */
  context_type left_type;            /**< Unit in which context is measured; context_type enum. */
  char *left_structure_name;         /**< If the "unit" is an s-attribute, its name is here. */
  Attribute *left_structure;         /**< If the "unit" is an s-attribute, the Attribute itself is here. */

  /* ==================== right context scope description variables */

  int right_width;                   /**< Amount of context to show after the match, in units specified by right_type */
  context_type right_type;           /**< Unit in which context is measured; context_type enum. */
  char *right_structure_name;        /**< If the "unit" is an s-attribute, its name is here. */
  Attribute *right_structure;        /**< If the "unit" is an s-attribute, the Attribute itself is here. */

  /** Boolean flag: if true, print corpus position number at the start of each concordance line */
  int print_cpos;

  /* ==================== lists of attributes of different types to print */

  AttributeList *attributes;         /**< positional attributes to print */
  AttributeList *strucAttributes;    /**< structural attributes to print */
  AttributeList *printStructureTags; /**< structure tag (s-att values) to print */
  AttributeList *alignedCorpora;     /**< aligned corpora from which to print parallel data */

} ContextDescriptor;


/* ContextDescriptor class methods */

int verify_context_descriptor(Corpus *corpus, ContextDescriptor *cd, int remove_illegal_entries);

int initialize_context_descriptor(ContextDescriptor *cd);

int update_context_descriptor(Corpus *corpus, ContextDescriptor *cd);

void print_context_descriptor(ContextDescriptor *cdp);

#endif
