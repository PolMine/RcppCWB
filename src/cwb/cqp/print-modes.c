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
#include <assert.h>

#include "../cl/cl.h"

#include "corpmanag.h"
#include "attlist.h"
#include "output.h"
#include "options.h"

/** String containing print structure separator */
#define PRINT_STRUC_SEP ": ,"


/** Global print-mode setting. */
PrintMode GlobalPrintMode = PrintASCII;

/** Global print-options: all booleans initially set to false */
PrintOptions GlobalPrintOptions = { 0, 0, 0, 0, 0 };


/**
 * Computes a list of s-attributes to print from the PrintStructure global option setting.
 *
 * PrintStructure is itself updated.
 *
 * @param cl  The corpus from which to find the attributes.
 * @return    An attribute list containing the attributes to be printed.
 */
AttributeList *
ComputePrintStructures(CorpusList *cl)
{
  char *token, *p;
  AttributeList *al = NULL;
  AttributeInfo *ai = NULL;
  Attribute *struc = NULL;

  if (!printStructure || printStructure[0] == '\0' || !cl)
    return NULL;

  if (!(token = strtok(printStructure, PRINT_STRUC_SEP)))
    return NULL;

  while (token) {
    if (!(struc = cl_new_attribute(cl->corpus, token, ATT_STRUC)))
      cqpmessage(Warning, "Structure ``%s'' not declared for corpus ``%s''.", token, cl->corpus->registry_name);
    else if (!cl_struc_values(struc)) {
      cqpmessage(Warning, "Structure ``%s'' does not have any values.", token);
      struc = NULL;
    }

    if (struc) {
      if (!al)
        al = NewAttributeList(ATT_STRUC);
      AddNameToAL(al, token, 1, 0);
    }
    token = strtok(NULL, PRINT_STRUC_SEP);
  }

  if (al) {
    if (!VerifyList(al, cl->corpus, 1)) {
      cqpmessage(Error, "Problems while computing print structure list");
      DestroyAttributeList(&al);
      al = NULL;
    }
    else if (!al->list)
      DestroyAttributeList(&al);
  }

  /* rebuild printStructure string to show only valid attributes */
  p = printStructure;
  *p = '\0';
  for ( ai = al ? al->list : NULL ; ai ; ai = ai->next ) {
    if (p != printStructure)
      *p++ = ' ';                /* insert blank between attributes */
    sprintf(p, "%s", ai->attribute->any.name);
    p += strlen(p);
  }

  return al;
}


/**
 * Reads the global string printModeOptions and parses it to update the GlobalPrintOptions.
 *
 * @see printModeOptions
 * @see GlobalPrintOptions
 */
void
ParsePrintOptions(void)
{
  if (printModeOptions) {
    char *token;
    char s[CL_MAX_LINE_LENGTH];
    int value;

    /* we must not destructively modify the global variable, so run strtok on a local copy */
    cl_strcpy(s, printModeOptions);

    token = strtok(s, " \t\n,.");
    while (token) {
      /* handle the "no" prefix that turns a setting off, if present */
      if (0 == strncasecmp(token, "no", 2)) {
        value = 0;
        token += 2;
      }
      else
        value = 1;

      if (cl_streq_ci(token, "wrap"))
        GlobalPrintOptions.print_wrap = value;

      else if (cl_streq_ci(token, "table")  || cl_streq_ci(token, "tbl"))
        GlobalPrintOptions.print_tabular = value;

      else if (cl_streq_ci(token, "header") || cl_streq_ci(token, "hdr"))
        GlobalPrintOptions.print_header = value;

      else if (cl_streq_ci(token, "border") || cl_streq_ci(token, "bdr"))
        GlobalPrintOptions.print_border = value;

      else if (cl_streq_ci(token, "number") || cl_streq_ci(token, "num"))
        GlobalPrintOptions.number_lines = value;

      else if (!silent)
        Rprintf("Warning: %s: unknown print option\n", token);

      token = strtok(NULL, " \t\n,.");
    }
  }
}

