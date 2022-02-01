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

#ifndef _cqp_html_print_h_
#define _cqp_html_print_h_

#include <stdio.h>
#include "../cl/bitfields.h"
#include "corpmanag.h"
#include "context_descriptor.h"
#include "print-modes.h"
#include "groups.h"

#define SUBST_NONE 0
#define SUBST_LT   1
#define SUBST_GT   2
#define SUBST_AMP  4
#define SUBST_QUOT 8
#define SUBST_ALL  (SUBST_LT | SUBST_GT | SUBST_AMP | SUBST_QUOT)

extern PrintDescriptionRecord HTMLPrintDescriptionRecord;

const char *html_convert_string(const char *s);

void html_print_aligned_line(FILE *dest, int highlighting, char *attribute_name, char *line);

void html_print_corpus_header(CorpusList *cl,FILE *dest);

void html_print_output(CorpusList *cl, FILE *dest, int interactive,ContextDescriptor *cd, int first, int last);

void html_print_group(Group *group, FILE *dest);

#endif

