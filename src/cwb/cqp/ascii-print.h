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

#ifndef _cqp_ascii_print_h_
#define _cqp_ascii_print_h_

#include <stdio.h>

#include "corpmanag.h"
#include "context_descriptor.h"
#include "print-modes.h"
#include "groups.h"

extern PrintDescriptionRecord ASCIIPrintDescriptionRecord;

const char *ascii_convert_string(const char *s);

void ascii_print_aligned_line(FILE *dest, int highlighting, char *attribute_name, char *line);

void ascii_print_corpus_header(CorpusList *cl, FILE *dest);

void ascii_print_output(CorpusList *cl,
                        FILE *dest,
                        int interactive,
                        ContextDescriptor *cd,
                        int first, int last);

void ascii_print_group(Group *group, FILE *dest);


char *get_colour_escape(char colour, int foreground);

char *get_typeface_escape(char typeface);

#endif


