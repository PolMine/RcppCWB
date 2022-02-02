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
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>           /* for select() */

#ifndef __MINGW__
#include <pwd.h>
#endif

#include "../cl/globals.h"
#include "../cl/corpus.h"
#include "../cl/attributes.h"

#include "cqp.h"
#include "options.h"
#include "output.h"
#include "corpmanag.h"
#include "concordance.h"
#include "print-modes.h"
#include "print_align.h"

#include "ascii-print.h"
#include "sgml-print.h"
#include "html-print.h"
#include "latex-print.h"


/** Global list of tabulation items for use with the "tabulate" operator */
TabulationItem TabulationList = NULL;


/* stupid Solaris doesn't have setenv() function, so we need to emulate it with putenv() */
#ifdef EMULATE_SETENV

char emulate_setenv_buffer[CL_MAX_LINE_LENGTH]; /* should be big enough for "var=value" string */

int
setenv(const char *name, const char *value, int overwrite) {
  assert(name != NULL && value != NULL && "Invalid call of setenv() emulation function.");
  sprintf(emulate_setenv_buffer, "%s=%s", name, value);
  return putenv(emulate_setenv_buffer);
}

#endif


/**
 * Prints something like a header for a concordance.
 *
 * @param cl      CorpusList object for the subcorpus representing the query to print out.
 * @param stream  Where to print to.
 * @param mode    The print mode to use (ascii, html etc.)
 * @param force   Boolean: if true, the header will print even if headers are
 *                disabled in the global print options.
 * */
void
print_corpus_info_header(CorpusList *cl,
                         FILE *stream,
                         PrintMode mode,
                         int force)
{
  if (force || GlobalPrintOptions.print_header) {
    switch(mode) {
    case PrintASCII:
      ascii_print_corpus_header(cl, stream);
      break;

    case PrintSGML:
      sgml_print_corpus_header(cl, stream);
      break;

    case PrintHTML:
      html_print_corpus_header(cl, stream);
      break;

    case PrintLATEX:
      latex_print_corpus_header(cl, stream);
      break;

    default:
      break;
    }
  }
}


/**
 * Creates, and opens for text-mode write, a temporary file.
 *
 * Temporary files have the form "cqp-tempfile.XXXXXX" (where XXXXXX = distinguishing letters)
 * and are placed in the directory defined as TEMPDIR_PATH.
 *
 * @see                   TEMPDIR_PATH
 * @see                   TEMP_FILENAME_BUFSIZE
 *
 * @param tmp_name_buffer A pre-allocated buffer which will be overwritten
 *                        with the name of the temporary file.
 *                        This should be at least  ( strlen(TEMPDIR_PATH) + 20 ) bytes in size.
 *                        If opening is unsuccessful, this buffer will be set to "".
 * @return                A stream (FILE *) to the opened temporary file, or NULL if unsuccessful.
 */
FILE *
open_temporary_file(char *tmp_name_buffer)
{
  static char *fixed_template = TEMPDIR_PATH SUBDIR_SEP_STRING "cqp-tempfile.XXXXXX";
  int fd;
  FILE *dst;

  assert(tmp_name_buffer && "Invalid NULL argument in open_temporary_file().");

  /* template is duplicated into the buffer the caller provided, then modified in situ by mkstemp(). */
  cl_strcpy(tmp_name_buffer, fixed_template);

  if (-1 != (fd = mkstemp(tmp_name_buffer)))
    if (NULL != (dst = fdopen(fd, "w")))
      return dst;

  /* otherwise, two possible error conditions... */
  if (-1 == fd)
    perror("open_temporary_file(): can't create temporary file");
  else
    perror("open_temporary_file(): can't get stream to temporary file");

  *tmp_name_buffer = '\0';
  return NULL;
}


/**
 * Create a pipe to a new instance of a specified program to be used as
 * an output pager.
 *
 * If cmd is different from the program specified in the global
 * variable "tested_pager", run a test first.
 *
 * This would normally be something like "more" or "less".
 *
 * @see            tested_pager
 * @see            less_charset_variable
 * @param cmd      Program command to start pager procress.
 * @param charset  Charset to which to set the pager-charset-environment variable
 * @return         Writable stream for the pipe to the pager, or NULL if a
 *                 test of the pager program failed; must be closed with cl_close_stream()
 */
FILE *
open_pager(char *cmd, CorpusCharset charset)
{
  FILE *pipe;

  if (!tested_pager || !cl_streq(tested_pager, cmd)) {
    /* this is a new pager, so test it */
    pipe = popen(cmd, "w");
    if (!pipe || 0 != pclose(pipe))
      return NULL;    /* new pager cmd doesn't work -> return error */

    if (tested_pager)
      cl_free(tested_pager);
    tested_pager = cl_strdup(cmd);
  }

  /* if ( less_charset_variable != "" ) set environment variable accordingly */
  if (*less_charset_variable) {
    char *current_value, *new_value;

    switch (charset){
    case ascii:   /* fallthru is intentional: ASCII is a subset of valid UTF-8 */
    case utf8:
      new_value = "utf-8";    break;

    /* "less" does not distinguish between the different ISO-8859 character sets,
     * so if not using UTF-8, always use ISO-8859 */
    default:
      new_value = "iso8859";  break;
    }

    current_value = getenv(less_charset_variable);

    /* call setenv() if variable is not set or different from desired value */
    if (!current_value || !cl_streq(current_value, new_value))
      setenv(less_charset_variable, new_value, 1);
  }

  return cl_open_stream(cmd, CL_STREAM_WRITE, CL_STREAM_PIPE);
}


/**
 * Open the (output) stream within a Redir(ection) structure.
 *
 * If output is sent to a pipe, a signal handler for SIGPIPE is automatically
 * installed and configured to set the global variable broken_pipe to True.
 * Output functions should check this variable and abort if it is set. The
 * signal handler is uninstalled when close_pipe is called, which may lead to
 * undesired behaviour if multiple streams are open at the same time.
 *
 * @see CLStream
 *
 * @param rd       Redir structure to be opened.
 * @param charset  The charset to be used. Only has an effect if the stream
 *                 to be opened is to an output pager.
 * @return         True for success, false for failure.
 */
int
open_rd_output_stream(struct Redir *rd, CorpusCharset charset)
{
  int mode;

  if (!rd)
    return 0;

  if (rd->stream) {
    /* stream appears to be already open: close, then reopen */
    cl_close_stream(rd->stream);
    rd->stream = NULL;
  }

  if (rd->name) {
    /* open file (with compression and pipe magic) */
    mode = cl_str_is(rd->mode, "a") ? CL_STREAM_APPEND : CL_STREAM_WRITE;
    rd->stream = cl_open_stream(rd->name, mode, (insecure) ? CL_STREAM_MAGIC_NOPIPE : CL_STREAM_MAGIC);
    rd->is_paging = False;
  }
  else {
    if (pager && paging && isatty(fileno(stdout))) {
      if (insecure)
        cqpmessage(Error, "Insecure mode, paging not allowed.\n");
        /* ... and default back to bare stdout below */
      else {
        rd->stream = open_pager(pager, charset);
        if (!rd->stream) {
          cqpmessage(Warning, "Could not start pager '%s', trying fallback '%s'.\n", pager, CQP_FALLBACK_PAGER);
          rd->stream = open_pager(CQP_FALLBACK_PAGER, charset);
          if (!rd->stream) {
            cqpmessage(Warning, "Could not start fallback pager '%s'. Paging disabled.\n", CQP_FALLBACK_PAGER);
            set_integer_option_value("Paging", 0);
            /* ... and default back to bare stdout below */
          }
          else
            set_string_option_value("Pager", cl_strdup(CQP_FALLBACK_PAGER));
        }
      }
    }
    /* if not paging or pager failed to start, open stdout */
    if (rd->stream)
      rd->is_paging = True;
    else {
      rd->stream = cl_open_stream("", CL_STREAM_WRITE, CL_STREAM_STDIO);
      rd->is_paging = False;
    }
  }

  if (!rd->stream) {
    cqpmessage(Error, "Can't write to %s: %s", (rd->name) ? rd->name : "STDOUT", cl_error_string(cl_errno));
    return 0;
  }

  return 1;
}

/**
 * Closes the (output) stream within a Redir structure.
 *
 * If output was being sent to a pipe, SIGPIPE is set back to the SIG_IGN handler.
 *
 * @param rd  The Redir stream to close.
 * @return    True for all OK, false if closing did not work. If rd does not
 *            actually have an open stream, nothing is done, and that counts
 *            as a success.
 */
int
close_rd_output_stream(struct Redir *rd)
{
  int success = 1;

  if (!rd)
    return 0;

  if (rd->stream) {
    success = !cl_close_stream(rd->stream); /* returns 0 on success */
    rd->stream = NULL;
    rd->is_paging = 0;
  }

  return success;
}

/**
 * Opens the input stream within a InputRedir structure.
 *
 * @param rd   The Redir stream to open for input.
 * @return     True for all OK, false if opening did not work.
 */
int
open_rd_input_stream(struct InputRedir *rd)
{
  int i;
  char *tmp;

  if (!rd)
    return 0;

  if (rd->stream) {
    /* stream appears to be already open: close, then reopen */
    cl_close_stream(rd->stream);
    rd->stream = NULL;
  }

  if (rd->name) {
    /* Check for old-style pipe notation, i.e.
     *   ... < "ls -l |";
     * which is unfortunately documented in the CQP tutorial.  New-style notation
     *   ... < "| ls -l";
     * is automatically supported by cl_open_stream();
     */
    i = strlen(rd->name) - 1;
    while (i > 0 && rd->name[i] == ' ')
      i--;

    if (i >= 1 && (rd->name[i] == '|')) {
      /* read input from a pipe (unless running in "secure" mode) */
      if (insecure) {
        cqpmessage(Error, "Insecure mode, paging not allowed.\n");
        rd->stream = NULL;
        return 0;
      }
      else {
        tmp = (char *) cl_malloc(i + 1); /* pipe command = rd->name[0 .. (i-1)] */
        strncpy(tmp, rd->name, i);
        tmp[i] = '\0';
        rd->stream = cl_open_stream(tmp, CL_STREAM_READ, CL_STREAM_PIPE);
        cl_free(tmp);
      }
    }
    else
      /* open stream with CL automagic */
      rd->stream = cl_open_stream(rd->name, CL_STREAM_READ, insecure ? CL_STREAM_MAGIC_NOPIPE : CL_STREAM_MAGIC);
  }
  else
    rd->stream = cl_open_stream("", CL_STREAM_READ, CL_STREAM_STDIO);

  if (!rd->stream) {
    cqpmessage(Error, "Can't read from %s: %s", (rd->name) ? rd->name : "STDIN", cl_error_string(cl_errno));
    return 0;
  }

  return 1;
}

/**
 * Closes the input stream within a InputRedir structure.
 *
 * @param rd  The InputRedir stream to close.
 * @return    True for all OK, false if closing did not work.
 *            If rd does not actually have an open stream,
 *            nothing is done, and that counts as a success.
 */
int
close_rd_input_stream(struct InputRedir *rd)
{
  int success = 1;

  if (!rd)
    return 0;

  if (rd->stream) {
    success = !cl_close_stream(rd->stream); /* returns 0 on success */
    rd->stream = NULL;
  }

  return success;
}


/**
 * Prints a concordance (with no header) based on the subcorpus supplied
 * (as a CorpusList object called cl) onto the stream supplied.
 *
 * This function does not do much itself - it dispatches to another
 * specialised function based on the mode argument.
 *
 * It's also that delegate function which checks the validity of the range
 * of ranges within the subcorpus specified by7
 *
 * @see PrintMode
 * @param cl
 * @param stream
 * @param interactive
 * @param cd
 * @param first
 * @param last
 * @param mode
 */
void
print_concordance_body(CorpusList *cl,
                       FILE *stream,
                       int interactive,
                       ContextDescriptor *cd,
                       int first,
                       int last, /* range checking done by the mode-specific print function */
                       PrintMode mode)
{
  switch (mode) {

  case PrintSGML:
    sgml_print_output(cl, stream, interactive, cd, first, last);
    return;

  case PrintHTML:
    html_print_output(cl, stream, interactive, cd, first, last);
    return;

  case PrintLATEX:
    latex_print_output(cl, stream, interactive, cd, first, last);
    return;

  case PrintASCII:
    ascii_print_output(cl, stream, interactive, cd, first, last);
    return;

  default:
    cqpmessage(Error, "Unknown print mode");
    return;
  }
}

/**
 * Prints a "corpus", typically (some of) the matches of a query.
 *
 * This function supports the "cat" command, i.e. it is the main
 * "please print concordance" function.
 *
 * (Not sure why it's called "catalog"; is this a pun on the cat keyword? -- AH 2012-07-17)
 * (I suspect that it's a misinterpretation of what "cat" stands for. -- SE 2016-07-20)
 * (I finally bit the bullet and renamed it from "catalog_corpus" to "cat_listed_corpus". -- AH 2020-05-26)
 *
 * The query is represented by a subcorpus (cl); only results
 * [first]..[last]; will be printed; use (0,-1) for entire corpus.
 *
 * @see do_cat
 *
 * @param cl     The record of the corpus/subcorpus/query to output.
 * @param dst    Block of output redirection info; if NULL, default settings will be used.
 * @param first  Offset of first match to print.
 * @param last   Offset of last match to print.
 * @param mode   Print mode to use.
 */
void
cat_listed_corpus(CorpusList *cl,
                  struct Redir *dst,
                  int first,
                  int last,
                  PrintMode mode)
{
  int i;
  Boolean printHeader = False;
  struct Redir default_dst;

  if (!cl || !access_corpus(cl))
    return;

  if (!dst) {
    default_dst.name = NULL;
    default_dst.mode = "w";
    default_dst.stream = NULL;
    dst = &default_dst;
  }

  if (!open_rd_output_stream(dst, cl->corpus->charset)) {
    cqpmessage(Error, "Can't open output stream.");
    return;
  }

  assert(dst->stream);

  if (output_binary_ranges) {
    /* ====================================== BINARY OUTPUT */
    for (i = 0; (i < cl->size); i++) {
      fwrite(&(cl->range[i].start), sizeof(int), 1, dst->stream);
      fwrite(&(cl->range[i].end)  , sizeof(int), 1, dst->stream);
    }
  }
  else {
    /* ====================================== ASCII, SGML OR HTML OUTPUT */
    printHeader = GlobalPrintOptions.print_header;

    /* questionable... */
    if (GlobalPrintMode == PrintHTML)
      printHeader = True;

    /* do the job. */

    verify_context_descriptor(cl->corpus, &CD, 1);

    /* first version (Oli Christ):
       if ((!silent || printHeader) && !(rd->stream == stdout || rd->is_paging));
       */
    /* second version (Stefan Evert):
       if (printHeader || (mode == PrintASCII && !(rd->stream == stdout || rd->is_paging)));
    */

    /* header is printed _only_ when explicitly requested now (or, when in HTML mode; see above);
     * previous behaviour was to print header automatically when saving results to a file;
     * this makes sense when such files are created to document the results of a corpus search,
     * but nowadays they are mostly used for automatic post-processing (e.g. in a Web interface),
     * where the header is just a nuisance that has to be stripped.
     */
    if (printHeader)
      print_corpus_info_header(cl, dst->stream, mode, 1);
    else if (printNrMatches && mode == PrintASCII)
      fprintf(dst->stream, "%d matches.\n", cl->size);

    print_concordance_body(cl, dst->stream,
                           isatty(fileno(dst->stream)) || dst->is_paging,
                           &CD, first, last, mode);
  }

  close_rd_output_stream(dst);
}

/**
 * Print a message to output (for instance a debug message).
 *
 * @see           MessageType
 * @param type    Specifies what type of message (messages of some types are not always printed)
 * @param format  Format string (and ...) are passed as arguments to vfprintf().
 */
void
cqpmessage(MessageType type, const char *format, ...)
{
  char *msg;
  va_list ap;

  va_start(ap, format);

  /* do not print messages of level Message, unless the parser is in verbose mode */
  if (type != Message || verbose_parser) {
    switch (type) {
    case Error:
      msg = "CQP Error";
      break;
    case Warning:
      msg = "Warning";
      break;
    case Message:
      msg = "Message";
      break;
    case Info:
      msg = "Information";
      break;
    default:
      msg = "<UNKNOWN MESSAGE TYPE>";
      break;
    }

    if (!silent || type == Error) {
      Rprintf("%s:\n\t", msg);
      Rprintf(format, ap);
      Rprintf("\n");
    }
  }

  va_end(ap);
}

/**
 * Outputs a blob of information on the specified cl, or its mother-corpus if it's a SUB.
 */
void
corpus_info(CorpusList *cl)
{
  FILE *fh;
  FILE *outfh;

  char buf[CL_MAX_LINE_LENGTH];
  int i, ok, stream_ok;

  struct Redir to_less = { NULL, NULL, NULL, 0 }; /* for paging (with open_stream()) */

  CorpusList *mother = NULL;
  CorpusProperty p;

  if (!cl)
    return;

  /* first, the case where cl is actually a full corpus */
  if (cl->type == SYSTEM) {
    /* use pager, or simply print to stdout if it fails */
    stream_ok = open_rd_output_stream(&to_less, ascii);
    outfh = stream_ok ? to_less.stream : stdout;

    /* print name for child mode (added v3.4.15)  */
    if (child_process)
      Rprintf("Name:    %s\n", cl->name);
    /* print size (should be the mother_size entry) */
    Rprintf("Size:    %d\n", cl->mother_size);
    /* print charset */
    Rprintf("Charset: ");

    if (cl->corpus->charset == unknown_charset)
      Rprintf("<unsupported> (%s)\n", cl_corpus_property(cl->corpus, "charset"));
    else
      Rprintf("%s\n", cl_charset_name(cl->corpus->charset));

    /* print properties */
    Rprintf("Properties:\n");
    if (!(p = cl_first_corpus_property(cl->corpus)))
      Rprintf("\t<none>\n");
    else
      for ( ; p != NULL; p = cl_next_corpus_property(p))
        Rprintf("\t%s = '%s'\n", p->property, p->value);
    Rprintf("\n");

    /* do we have further info in a .INFO file? */
    if (
        !cl->corpus->info_file
        ||
        !(fh = cl_open_stream(cl->corpus->info_file, CL_STREAM_READ_BIN, CL_STREAM_MAGIC))
        /* most of the time this is NOT a problem - it just means the default HOME/.info
         * has not been created. Just another way that there can be no more info. */
        )
      Rprintf("No further information available about %s\n", cl->name);

    else {
      /* we do have some info to print out. */
      do {
        i = fread(&buf[0], sizeof(char), CL_MAX_LINE_LENGTH, fh);
        ok = (i == fwrite(&buf[0], sizeof(char), i, outfh));
      } while (ok && i == CL_MAX_LINE_LENGTH);

      /* if the .info file didn't end in a newline, prit one,
       * to ensure that output from "info;" always does end thus.*/
      if (buf[strlen(buf)-1] != '\n')
        Rprintf("\n");

      cl_close_stream(fh);
    }

    if (stream_ok)
      close_rd_output_stream(&to_less);        /* close pipe to pager if we were using it */

    return;
  }

  /* if cl is not actually a SYSTEM corpus, try to find its mother and call this function on that;
   * if the mother is not loaded, we just have to print an error */
  if (!cl->mother_name)
    cqpmessage(Warning, "Corrupt corpus information for %s", cl->name);

  else if (!(mother = findcorpus(cl->mother_name, SYSTEM, 0)))
      cqpmessage(Info,
                 "%s is a subcorpus of %s, which is not loaded. Try 'info %s' for information about %s.\n",
                 cl->name, cl->mother_name, cl->mother_name, cl->mother_name);
  else
    corpus_info(mother);
}


/** Free the global list of tabulation items (before building a new one). */
void
free_tabulation_list(void) {
  TabulationItem item, next;
  for (item = TabulationList ; item ; item = next) {
    next = item->next;
    cl_free(item->attribute_name);
    cl_free(item);
    /* if we had proper reference counting, we would delete the attribute handle here
       (but calling cl_delete_attribute() would _completely_ remove the attribute
       from the corpus for this session!) */
  }
  TabulationList = NULL;
}

/** allocate and initialize new tabulation item */
TabulationItem
new_tabulation_item(void)
{
  TabulationItem item = (TabulationItem) cl_malloc(sizeof(struct _TabulationItem));
  item->attribute_name = NULL;
  item->attribute = NULL;
  item->attribute_type = ATT_NONE;
  item->flags = 0;
  item->anchor1 = NoField;
  item->offset1 = 0;
  item->anchor2 = NoField;
  item->offset2 = 0;
  item->next = NULL;
  return item;
}

/** append tabulation item to end of current list */
void
append_tabulation_item(TabulationItem item)
{
  TabulationItem end = TabulationList;
  item->next = NULL;            /* make sure that item is marked as end of list */
  if (!end)                     /* empty list: item becomes first entry */
    TabulationList = item;

  else {                        /* otherwise, seek end of list and append item */
    while (end->next)
      end = end->next;
    end->next = item;
  }
}

/**
 * Gets the cpos of one of the "anchors" of a particular query result. Used for tabulation.
 *
 * @param cl      The query being tabulated.
 * @param n       The number of the match we are requesting an anchor for (where first match is 0).
 * @param anchor  Which of the anchors of the query match we are requesting.
 * @param offset  Integer offset from the anchor that we are requesting.
 * @return        The cpos of the requested position, which may fall outside the bounds of the corpus
 *                if an offset has been specified; or CDA_CPOSUNDEF if the anchor has not been set.
 */
static int
pt_get_anchor_cpos(CorpusList *cl, int n, FieldType anchor, int offset)
{
  int real_n, cpos;
  cpos = -1;

  real_n = (cl->sortidx) ? cl->sortidx[n] : n; /* get anchor for n-th match */

  switch (anchor) {
  case KeywordField:
    cpos = cl->keywords[real_n];
    break;
  case TargetField:
    cpos = cl->targets[real_n];
    break;
  case MatchField:
    cpos = cl->range[real_n].start;
    break;
  case MatchEndField:
    cpos = cl->range[real_n].end;
    break;
  case NoField:
  default:
    assert(0 && "Can't be");
    break;
  }

  /* undefined anchor (-1) or invalid anchor position */
  if (cpos < 0 || cpos >= cl->mother_size)
    return CDA_CPOSUNDEF;

  /* return anchor position with offset, may be out of bounds now */
  return cpos + offset;
}

static int
pt_validate_anchor(CorpusList *cl, FieldType anchor)
{
  switch (anchor) {
  case KeywordField:
    if (!cl->keywords) {
      cqpmessage(Error, "No keyword anchors defined for named query %s", cl->name);
      return 0;
    }
    break;
  case TargetField:
    if (!cl->targets) {
      cqpmessage(Error, "No target anchors defined for named query %s", cl->name);
      return 0;
    }
    break;
  case MatchField:
  case MatchEndField:
    /* should always be present */
    assert(cl->range);
    break;
  case NoField:
  default:
    cqpmessage(Error, "Illegal anchor in tabulate command");
    return 0;
    break;
  }
  return 1;
}

/**
 * Tabulate specified query result, using settings from global list of tabulation items.
 *
 * @param cl     CorpusList for the query result
 * @param first  first/last = range of hits to tabulate
 * @param last
 * @param dst    Stream to print to.
 * @return       Boolean: true indicates that tabulation was successful (otherwise, generates error message)
 * */
int
print_tabulation(CorpusList *cl, int first, int last, struct Redir *dst)
{
  TabulationItem item;
  int current;

  if (!cl)
    return 0;

  /* make sure that first and last match to tabulate are in range */
  if (first <= 0)
    first = 0;
  if (last >= cl->size)
    last = cl->size - 1;

  /* obtain attribute handles for tabulation items */
  for (item = TabulationList ; item ; item = item->next) {
    if (item->attribute_name) {
      if (NULL != (item->attribute = cl_new_attribute(cl->corpus, item->attribute_name, ATT_POS)))
        item->attribute_type = ATT_POS;
      else if (NULL != (item->attribute = cl_new_attribute(cl->corpus, item->attribute_name, ATT_STRUC))) {
        item->attribute_type = ATT_STRUC;
        if (! cl_struc_values(item->attribute)) {
          cqpmessage(Error, "No annotated values for s-attribute ``%s'' in named query %s", item->attribute_name, cl->name);
          return 0;
        }
      }
      else {
        cqpmessage(Error, "Can't find attribute ``%s'' for named query %s", item->attribute_name, cl->name);
        return 0;
      }
    }
    else
      item->attribute_type = ATT_NONE; /* no attribute -> print corpus position */

    if (cl->size > 0)
      /* work around bug: anchor validation will fail for empty query result (but then loop below is void anyway) */
      if (!( pt_validate_anchor(cl, item->anchor1) && pt_validate_anchor(cl, item->anchor2) ))
        return 0;
  }

  if (!open_rd_output_stream(dst, cl->corpus->charset)) {
    cqpmessage(Error, "Can't redirect output to file or pipe\n");
    return 0;
  }

  /* tabulate selected attribute values for matches <first> .. <last> */
  for (current = first; current <= last && !cl_broken_pipe; current++) {
    for (item = TabulationList ; item ; item = item->next) {
      int start = pt_get_anchor_cpos(cl, current, item->anchor1, item->offset1);
      int end   = pt_get_anchor_cpos(cl, current, item->anchor2, item->offset2);
      int cpos;

      /* if either of the anchors is undefined, print a single undef value for the entire range */
      if (start == CDA_CPOSUNDEF || end == CDA_CPOSUNDEF)
        start = end = -1;

      for (cpos = start; cpos <= end; cpos++) {
        if (cpos >= 0 && cpos <= cl->mother_size) {
          /* valid cpos: print cpos or requested attribute */
          if (item->attribute_type == ATT_NONE)
            fprintf(dst->stream, "%d", cpos);
          else {
            char *string = item->attribute_type == ATT_POS ?  cl_cpos2str(item->attribute, cpos) : cl_cpos2struc2str(item->attribute, cpos);
            if (string) {
              if (item->flags) {
                /* get canonical string as newly alloc'ed duplicate, then print */
                char *copy = cl_string_canonical(string, cl->corpus->charset, item->flags, CL_STRING_CANONICAL_STRDUP);
                fprintf(dst->stream, "%s", copy);
                cl_free(copy);
              }
              else
                fprintf(dst->stream, "%s", string);
            }
          }
        }
        else {
          /* cpos out of bounds: print -1 or empty string */
          if (item->attribute_type == ATT_NONE)
            fprintf(dst->stream, "-1");
        }
        if (cpos < end)         /* tokens in a range item are separated by blanks */
          fprintf(dst->stream, " ");
      }
      if (item->next)           /* multiple tabulation items are separated by TABs */
        fprintf(dst->stream, "\t");
    }
    fprintf(dst->stream, "\n");
  }

  close_rd_output_stream(dst);
  free_tabulation_list();
  return 1;
}

