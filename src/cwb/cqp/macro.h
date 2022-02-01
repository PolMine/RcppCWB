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

#ifndef _cqp_macro_h_
#define _cqp_macro_h_

/**
 * The maximum length of a line in a macro definition file.
 *
 * As of 3.2.x, to simplify things, this is the same as CL_MAX_LINE_LENGTH.
 */
#define MACRO_FILE_MAX_LINE_LENGTH CL_MAX_LINE_LENGTH

int yy_input_char(void);

/**
 * Buffer which stores input strings while parsing a macro.
 */
typedef struct InputBuffer *InputBuffer;

extern InputBuffer InputBufferList;

void init_macros(void);

void drop_macros(void);

int define_macro(char *name, int n_args, char *argstr, char *definition);

void load_macro_file(const char *name);

int expand_macro(const char *name);

int delete_macro_buffers(int trace);


/* macro iterator functions (iterate through all macros in hash) for command-line completion */
void macro_iterator_reset(void);
char *macro_iterator_next_prototype(const char *prefix);

void list_macros(const char *prefix);

void print_macro_definition(const char *name, const int n_args);

void macro_statistics(void);


#endif
