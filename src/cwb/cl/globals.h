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


void Rprintf(const char *, ...);
#ifndef _cl_globals_h_
#define _cl_globals_h_

/* ensure that cl.h is included by all source files */
#include "cl.h"               /* also brings in stdio, stdlib */
#include "cwb-globals.h"


/* standard libraries used by most CL source files */
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <unistd.h>

/* global configuration variables (should not be used outside the CL, instead use accessor functions) */

extern int cl_debug;
extern int cl_optimize;
extern size_t cl_memory_limit;


#endif
