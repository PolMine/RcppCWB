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

#include "html-print.h"

#include <stdio.h>
#include <string.h>
#include "../cl/globals.h"
#include "../cl/corpus.h"
#include "../cl/attributes.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "cqp.h"
#include "options.h"
#include "corpmanag.h"
#include "concordance.h"
#include "attlist.h"
#include "print_align.h"

/* ---------------------------------------------------------------------- */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>

#ifndef __MINGW__
#include <pwd.h>
#endif


char *html_print_field(FieldType field, int start);

/* ------------------------------------------------------- */

PrintDescriptionRecord
HTMLPrintDescriptionRecord = {
  "<EM>%d:</EM>",               /* CPOSPrintFormat */

  "<EM>",                       /* BeforePrintStructures */
  " ",                          /* PrintStructureSeparator */
  "</EM>",                      /* AfterPrintStructures */

  "&lt;",                       /* StructureBeginPrefix */
  "&gt;",                       /* StructureBeginSuffix */

  " ",                          /* StructureSeparator */
  "&lt;/",                      /* StructureEndPrefix */
  "&gt;",                       /* StructureEndSuffix */

  NULL,                         /* BeforeToken */
  " ",                          /* TokenSeparator */
  "/",                          /* AttributeSeparator */
  NULL,                         /* AfterToken */

  NULL,                         /* BeforeField */
  NULL,                         /* AfterField */

  "<LI>",                       /* BeforeLine */
  "\n",                         /* AfterLine */

  "<HR>\n<UL>\n",               /* BeforeConcordance */
  "</UL>\n<HR>\n",              /* AfterConcordance */

  html_convert_string,
  html_print_field
};

PrintDescriptionRecord
HTMLTabularPrintDescriptionRecord = {
  "<TD ALIGN=RIGHT>%d:</TD>",   /* CPOSPrintFormat */

  "<TD><EM>",                   /* BeforePrintStructures */
  " ",                          /* PrintStructureSeparator */
  "</EM></TD>",                 /* AfterPrintStructures */

  "&lt;",                       /* StructureBeginPrefix */
  "&gt;",                       /* StructureBeginSuffix */

  " ",                          /* StructureSeparator */
  "&lt;/",                      /* StructureEndPrefix */
  "&gt;",                       /* StructureEndSuffix */

  NULL,                         /* BeforeToken */
  " ",                          /* TokenSeparator */
  "/",                          /* AttributeSeparator */
  NULL,                         /* AfterToken */

  "<TD ALIGN=RIGHT> ",          /* BeforeField */
  "</TD>",                      /* AfterField */

  "<TR>",                       /* BeforeLine */
  "</TR>\n",                    /* AfterLine */

  "<HR>\n<TABLE>\n",            /* BeforeConcordance */
  "</TABLE>\n<HR>\n",           /* AfterConcordance */
  html_convert_string,
  html_print_field
};

PrintDescriptionRecord
HTMLTabularNowrapPrintDescriptionRecord = {
  "<TD ALIGN=RIGHT nowrap>%d:</TD>", /* CPOSPrintFormat */

  "<TD nowrap><EM>",            /* BeforePrintStructures */
  " ",                          /* PrintStructureSeparator */
  "</EM></TD>",                 /* AfterPrintStructures */

  "&lt;",                       /* StructureBeginPrefix */
  "&gt;",                       /* StructureBeginSuffix */

  " ",                          /* StructureSeparator */
  "&lt;/",                      /* StructureEndPrefix */
  "&gt;",                       /* StructureEndSuffix */

  NULL,                         /* BeforeToken */
  " ",                          /* TokenSeparator */
  "/",                          /* AttributeSeparator */
  NULL,                         /* AfterToken */

  "<TD nowrap ALIGN=RIGHT> ",   /* BeforeField */
  "</TD>",                      /* AfterField */

  "<TR>",                       /* BeforeLine */
  "</TR>\n",                    /* AfterLine */

  "<HR>\n<TABLE>\n",            /* BeforeConcordance */
  "</TABLE>\n<HR>\n",           /* AfterConcordance */
  html_convert_string,
  html_print_field
};

/* ---------------------------------------------------------------------- */

static void
html_puts(FILE *dst, const char *s, int flags)
{
  if (!s)
    s = "(null)";

  if (flags) {
    while (*s) {
      if (*s == '<' && (flags & SUBST_LT))
        fputs("&lt;", dst);
      else if (*s == '>' && (flags & SUBST_GT))
        fputs("&gt;", dst);
      else if (*s == '&' && (flags & SUBST_AMP))
        fputs("&amp;", dst);
      else if (*s == '"' && (flags & SUBST_QUOT))
        fputs("&quot;", dst);
      else
        fputc(*s, dst);
      s++;
    }
  }
  else
    fputs(s, dst);
}

char *
html_print_field(FieldType field, int at_end)
{
  switch (field) {

  case NoField:
    if (GlobalPrintOptions.print_tabular)
      return (at_end ? "</TD>" : "<TD>");
    else
      return NULL;
    break;

  case MatchField:
    if (GlobalPrintOptions.print_tabular) {
      if (at_end) {
        if (GlobalPrintOptions.print_wrap)
          return "</B></TD><TD>";
        else
          return "</B></TD><TD nowrap>";
      }
      return (GlobalPrintOptions.print_wrap ? "</TD><TD><B>" : "</TD><TD nowrap><B>");
    }
    else
      return (at_end ? "</B>" : "<B>");
    break;

  case KeywordField:
    return (at_end ? "</U>" : "<U>");

  case TargetField:
    return (at_end ? "</EM>" : "<EM>");

  default:
    return NULL;
  }
}

/**
 * Dis-entititifies lt, gt and amp entities in a string supplierd.
 * @return   Pointer into an internal static buffer
 */
const char *
html_convert_string(const char *s)
{
  static char html_s[CL_MAX_LINE_LENGTH*2];
  int p; /* "pointer" into array */

  if (!s || strlen(s) > CL_MAX_LINE_LENGTH)
    return NULL;

  for (p = 0; *s; s++) {
    switch (*s) {
    case '<':
      html_s[p++] = '&';
      html_s[p++] = 'l';
      html_s[p++] = 't';
      html_s[p++] = ';';
      break;

    case '>':
      html_s[p++] = '&';
      html_s[p++] = 'g';
      html_s[p++] = 't';
      html_s[p++] = ';';
      break;

    case '&':
      html_s[p++] = '&';
      html_s[p++] = 'a';
      html_s[p++] = 'm';
      html_s[p++] = 'p';
      html_s[p++] = ';';
      break;

    default:
      html_s[p++] = *s;
      break;
    }
  }
  html_s[p] = '\0';

  return html_s;
}

/* ---------------------------------------------------------------------- */

/**
 * Prints a line of text (which will have been previously extracted from a corpus
 * linked to the present corpus by an a-attribute) within HTML markup.
 *
 * @param dest            Destination for the output.
 * @param highlighting    DUMMY: only supported by ascii print.
 * @param attribute_name  The name of the aligned corpus: printed in the leading indicator within the HTML.
 * @param line            Character data of the line of aligned-corpus data to print. This is treated as opaque.
 */
void
html_print_aligned_line(FILE *dest, int highlighting, char *attribute_name, char *line)
{
  fputc('\n', dest);

  if (GlobalPrintOptions.print_tabular)
    Rprintf("<TR><TD colspan=4%s><EM><B><EM>--&gt;", GlobalPrintOptions.print_wrap ? "" : " nowrap");
  else
    html_puts(dest, "<P><B><EM>--&gt;", 0);

  html_puts(dest, attribute_name, SUBST_ALL);
  html_puts(dest, ":</EM></B>&nbsp;&nbsp;", 0);
  html_puts(dest, line, SUBST_NONE); /* entities have already been escaped */

  if (GlobalPrintOptions.print_tabular)
    Rprintf("</TR>\n");
  else
    fputc('\n', dest);
}

void
html_print_context(ContextDescriptor *cd, FILE *dest)
{
  fputs("<tr><td nowrap><em>Left display context:</em></td><td nowrap>", dest);

  switch(cd->left_type) {
  case char_context:
    Rprintf("%d characters", cd->left_width);
    break;
  case word_context:
    Rprintf("%d tokens", cd->left_width);
    break;
  case s_att_context:
    Rprintf("%d %s", cd->left_width,
            cd->left_structure_name ? cd->left_structure_name : "???");
    break;
  default:
    break;
  }

  fputs("</td></tr>\n", dest);
  fputs("<tr><td nowrap><em>Right display context:</em></td><td nowrap>", dest);

  switch(cd->right_type) {
  case char_context:
    Rprintf("%d characters", cd->right_width);
    break;
  case word_context:
    Rprintf("%d tokens", cd->right_width);
    break;
  case s_att_context:
    Rprintf("%d %s", cd->right_width,
            cd->right_structure_name ? cd->right_structure_name : "???");
    break;
  default:
    break;
  }

  fputs("</td></tr>\n", dest);
}

void
html_print_corpus_header(CorpusList *cl, FILE *dest)
{
#ifndef __MINGW__
  struct passwd *pwd = NULL;
#endif

  if (GlobalPrintOptions.print_header) {
    time_t now;
    time(&now);
    /*   pwd = getpwuid(geteuid()); */
    /* disabled because of incompatibilities between different Linux versions */

    Rprintf(
            "<em><b>This concordance was generated by:</b></em><p>\n"
            "<table>\n"
            "<tr><td nowrap><em>User:</em></td><td nowrap>%s (%s)</td></tr>\n"
            "<tr><td nowrap><em>Date:</em></td><td nowrap>%s</td></tr>\n"
            "<tr><td nowrap><em>Corpus:</em></td><td nowrap>%s</td></tr>\n"
            "<tr><td nowrap> </td><td nowrap>%s</td></tr>\n"
            "<tr><td nowrap><em>Subcorpus:</em></td><td nowrap>%s:%s</td></tr>\n"
            "<tr><td nowrap><em>Number of Matches:</em></td><td nowrap>%d</td></tr>\n",
#ifndef __MINGW__
            (pwd ? pwd->pw_name : "unknown"),
            (pwd ? pwd->pw_gecos  : "unknown"),
#else
            "unknown",
            "unknown",
#endif
            ctime(&now),
            (cl->corpus && cl->corpus->registry_name ? cl->corpus->registry_name : "unknown"),
            (cl->corpus && cl->corpus->name ? cl->corpus->name : "unknown"),
            cl->mother_name, cl->name,
            cl->size);

    html_print_context(&CD, dest);

    fputs("</table>\n", dest);

    if (cl->query_corpus && cl->query_text)
      Rprintf("<P><EM>Query text:</EM> <BR>\n<BLOCKQUOTE><CODE>\n%s; %s\n</CODE></BLOCKQUOTE>\n", cl->query_corpus, cl->query_text);

    fputs("<p>\n", dest);
  }

}


void
html_print_output(CorpusList *cl, FILE *dest, int interactive, ContextDescriptor *cd, int first, int last)
{
  int line, real_line;
  char *outstr;
  ConcLineField clf[NoField];    /* NoField is largest field code so clf has one entry per anchor-type. */
  PrintDescriptionRecord *pdr;

  ParsePrintOptions();

  if (GlobalPrintOptions.print_tabular) {
    pdr = GlobalPrintOptions.print_wrap ? &HTMLTabularPrintDescriptionRecord : &HTMLTabularNowrapPrintDescriptionRecord;
    Rprintf("<HR><TABLE%s>\n",GlobalPrintOptions.print_border ? " BORDER=1" : "");
  }
  else {
    pdr = &HTMLPrintDescriptionRecord;
    fputs("<HR><UL>\n", dest);
  }

  /* clamp range indexes to size of the "range" array in the CorpusList */
  if (first < 0)
    first = 0;
  if (last >= cl->size || last < 0)
    last = cl->size - 1;

  for (line = first; (line <= last) && !cl_broken_pipe; line++) {
    real_line = (cl->sortidx ? cl->sortidx[line] : line);

    /* ---------------------------------------- concordance fields (IE anchors) */

    clf[MatchField].type = MatchField;
    clf[MatchField].start_position = cl->range[real_line].start;
    clf[MatchField].end_position = cl->range[real_line].end;

    clf[MatchEndField].type = MatchEndField; /* unused, because we use MatchField for the entire match */
    clf[MatchEndField].start_position = -1;
    clf[MatchEndField].end_position = -1;

    clf[KeywordField].type = KeywordField;
    if (cl->keywords) {
      clf[KeywordField].start_position = cl->keywords[real_line];
      clf[KeywordField].end_position = cl->keywords[real_line];
    }
    else {
      clf[KeywordField].start_position = -1;
      clf[KeywordField].end_position = -1;
    }

    clf[TargetField].type = TargetField;
    if (cl->targets) {
      clf[TargetField].start_position = cl->targets[real_line];
      clf[TargetField].end_position = cl->targets[real_line];
    }
    else {
      clf[TargetField].start_position = -1;
      clf[TargetField].end_position = -1;
    }

    fputs(pdr->BeforeLine, dest);
    outstr = compose_kwic_line(cl->corpus, cl->range[real_line].start,
                               cl->range[real_line].end, &CD,
                               NULL, NULL,
                               clf, NoField, /* NoField = # of entries in clf[] */
                               pdr);
    fputs(outstr, dest);
    cl_free(outstr);
    fputs(pdr->AfterLine, dest);

    if (CD.alignedCorpora)
      print_all_aligned_lines(cl->corpus, &CD, cl->range[real_line].start,
          cl->range[real_line].end, 0, /* ASCII print mode only */
          dest);
  }

  fputs(pdr->AfterConcordance, dest);
}


void
html_print_group(Group *group, FILE *dest)
{
  int source_id, target_id, count;
  int cell, last_source_id;
  int nr_targets;

  const char *target_s = "(null)";

  /* na ja... */
  last_source_id = -999;
  nr_targets = 0;

  Rprintf("<BODY>\n<TABLE>\n");

  for (cell = 0; (cell < group->nr_cells) && !cl_broken_pipe; cell++) {
    Rprintf("<TR><TD>");

    source_id = group->count_cells[cell].s;
    if (source_id != last_source_id) {
      last_source_id = source_id;
      html_puts(dest, Group_id2str(group, source_id, 0), SUBST_ALL);
      nr_targets = 0;
    }
    else
      Rprintf("&nbsp;");

    target_id = group->count_cells[cell].t;
    target_s  = Group_id2str(group, target_id, 1);
    count     = group->count_cells[cell].freq;

    Rprintf("<TD>");
    html_puts(dest, target_s, SUBST_ALL);
    Rprintf("<TD>%d</TR>\n", count);

    nr_targets = nr_targets + 1; /* replaces nr_targets++; #83 */ 
  }

  Rprintf("</TABLE>\n</BODY>\n");
}


