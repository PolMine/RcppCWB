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
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "../cl/cl.h"
#include "../cl/cwb-globals.h"

#include "cqp.h"
#include "options.h"
#include "output.h"
#include "symtab.h"
#include "tree.h"
#include "eval.h"
#include "corpmanag.h"
#include "regex2dfa.h"
#include "ranges.h"
#include "macro.h"
#include "targets.h"

#include "parser.tab.h"


/** File handle used by the CQP-query-language parser. */
extern FILE *yyin;
/** Activates the CQP-query-language parser. Returns 0 for success, 1 for failure. . */
extern int yyparse (void);
/** restarts the CQP-query-language parser. */
extern void yyrestart(FILE *input_file);


/* ======================================== Query Buffer Interface */
/* ========== see parser.l:extendQueryBuffer() for details */
/* ========== initialization done in parse_actions.c:prepare_parse() */

char QueryBuffer[QUERY_BUFFER_SIZE];        /**< buffer for queries */
int QueryBufferP = 0;                       /**< index into this buffer, for appending */
int QueryBufferOverflow = 0;                /**< flag which signals buffer overflows */


/** Global variable indicating type (CYC) of last expression */
CYCtype LastExpression;

/** Pointer that stores the address of strings read from the CLI by CQP. */
char *cqp_input_string;

/** index into the CQP input string; used for character-by-character read. */
int cqp_input_string_ix;

/** Boolean: 1 iff the exit command was issued while parsing. */
int exit_cqp;

/**
 * Boolean: true iff cqp_parse_file() - the main query syntax parsing function -
 * is currently reading from the cqprc file handle OR the cqpmacros file handle.
 *
 * @see cqprc
 * @see cqp_parse_file
 */
int reading_cqprc = 0;

/**
 * Global error status for CQP (will be returned to the caller when CQP exits).
 *
 * For the moment, it's simply boolean (0 = all OK, anything else = error).
 * Later, we might want to define some error macros (maybe have the CDA_
 * macros as a subset for errors bubbling up from CL, plus *also* extra
 * macros for other errors).
 *
 * TODO actually get this variable set in various error conditions.
 */
int cqp_error_status = 0;



/**
 * Boolean indicating that an interruptible process is currently running.
 *
 * The process in question is one that may be expected to be non-instantaneous.
 * This variable is turned off by the Ctrl+C interrupt handler.
 *
 * @see sigINT_signal_handler
 */
int EvaluationIsRunning;

/** Boolean */
int signal_handler_is_installed;


/**
 * This is the signal handler function that is used for the interrupt signal (CTRL+C).
 *
 * @param signum  The signal number (ignored).
 */
static void
sigINT_signal_handler(int signum)
{
  if (!signal_handler_is_installed)
    exit(cqp_error_status ? cqp_error_status : 1); /* make sure we abort if Ctrl-C is pressed a second time (even on platforms where signal handlers don't need to be reinstalled) */

  if (EvaluationIsRunning) {
    Rprintf("** Aborting evaluation ... (press Ctrl-C again to exit CQP)\n");
    EvaluationIsRunning = 0;
  }
  signal_handler_is_installed = 0;
}

/**
 * Installs the interrupt signal handler function with the OS.
 *
 * This function installs a Ctrl-C interrupt handler
 * (which clears the EvaluationIsRunning flag).
 * The function installed is sigINT_signal_handler.
 *
 * @see sigINT_signal_handler
 */
void
install_signal_handler(void)
{
  signal_handler_is_installed = 1; /* pretend it's installed even if it wasn't done properly, so CQP won't keep trying */
  if (SIG_ERR == signal(SIGINT, sigINT_signal_handler)) {
    cqpmessage(Warning, "Can't install interrupt handler.\n");
    signal(SIGINT, SIG_IGN);
  }
}



/**
 * Initialises the CQP program (or cqpserver or cqpcl).
 *
 * This function:
 * - initialises the global variables;
 * - initialises the built-in random number generator;
 * - initialises the macro database;
 * - parses the program options;
 * - reads the initialisation file;
 * - reads the macro initialisation file;
 * - and loads the default corpus, if any.
 *
 * @param argc  The argc from main()
 * @param argv  The argv from main()
 * @return      Always 1.
 */
int
initialize_cqp(int argc, char **argv)
{
  char *home = NULL;
  char init_file_fullname[CL_MAX_FILENAME_LENGTH];
#ifdef __MINGW__
  char *homedrive = NULL;
  char *homepath = NULL;
#endif

  /* file handle for initialisation files, if any */
  FILE *cqprc;

  extern int yydebug;

  /* initialize global variables */
  exit_cqp = 0;
  ee_ix = -1;

  /* initialise the CL */
  cl_startup();

  /* intialise built-in random number generator */
  cl_randomize();

  /* initialise the global "corpus" (i.e. corpora + subcorpora/NQRs) list */
  init_global_corpuslist();

  /* initialise macro database */
  init_macros();

  /* parse program options */
  parse_options(argc, argv);

  /* let's always run stdout unbuffered */
  /*  if (batchmode || rangeoutput || insecure || !isatty(fileno(stdout))) */
  if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
    perror("unbuffer stdout");

  yydebug = parser_debug;

  /* before we start looking for files, let's get the home directory, if we can,
   * so we don't have to detect it in more than one place. */
#ifndef __MINGW__
  home = (char *)getenv("HOME");
#else
  /* under Windows it is %HOMEDRIVE%%HOMEPATH% */
  if (NULL != (homepath = (char *)getenv("HOMEPATH")) && NULL != (homedrive = (char *)getenv("HOMEDRIVE")))  {
    home = (char *)cl_malloc(256);
    sprintf(home, "%s%s", homedrive, homepath);
  }
#endif
  /* note that either way above, home is NULL if the needed env var(s) were not found. */

  /* read initialization file if specified via -I, or if we are in interactive mode */
  if (
       cqp_init_file
       ||
       (!child_process && (!batchmode || !batchfh) && which_app != cqpserver)
      ) {
    /*
     * Read init file specified with -I <file>
     *   if no init file was specified, and we're not in batchmode, child mode, or cqpserver,
     *   looks for ~/.cqprc
     * Same with macro init file (-M <file> or ~/.cqpmacros), but ONLY if macros are enabled.
     */

    /*
     * allow interactive commands during processing of initialization file ???
     * (I don't think this is the case!!)
     */

    init_file_fullname[0] = '\0';

    /* read init file specified with -I , otherwise look for $HOME/.cqprc */
    if (cqp_init_file)
      sprintf(init_file_fullname, "%s", cqp_init_file);
    else if (home)
      sprintf(init_file_fullname, "%s%c%s", home, SUBDIR_SEPARATOR, CQPRC_NAME);

    if (init_file_fullname[0] != '\0') {
      if (NULL != (cqprc = fopen(init_file_fullname, "r"))) {
        reading_cqprc = 1;        /* not good for very much, really */
        if (!cqp_parse_file(cqprc, 1)) {
          Rprintf("Parse errors while reading %s, exiting.\n",  init_file_fullname);
          exit(cqp_error_status ? cqp_error_status : 1);
        }
        reading_cqprc = 0; /* no need to close the file - cqp_parse_file() does so once it's chewed it up */
      }
      else if (cqp_init_file) {
        Rprintf("Can't read initialization file %s\n", init_file_fullname);
        exit(cqp_error_status ? cqp_error_status : 1);
      }
    }
  }

  if (!enable_macros) {
    if (macro_init_file)
      cqpmessage(Warning, "Macros not enabled. Ignoring macro init file %s.", macro_init_file);
    /* and we do nothing else when !enable_macros. */
  }
  else {
    /* macros ARE enabled: can we use them safely? we need either to have a macro init file,
     * OR to be in interactive CQP (not child, batch, or server mode */
    if (
         macro_init_file
         ||
         (!child_process && (!batchmode || !batchfh) && which_app != cqpserver)
       ) {
      init_file_fullname[0] = '\0';

      /* read macro init file specified with -M ; otherwise look for ~/.cqpmacros */
      if (macro_init_file)
        sprintf(init_file_fullname, "%s", macro_init_file);
      else if (home)
        sprintf(init_file_fullname, "%s%c%s", home, SUBDIR_SEPARATOR, CQPMACROS_NAME);

      if (init_file_fullname[0] != '\0') {
        if (NULL != (cqprc = fopen(init_file_fullname, "r"))) {
          reading_cqprc = 1;
          if (!cqp_parse_file(cqprc, 1)) {
            Rprintf("Parse errors while reading %s, exiting.\n", init_file_fullname);
            exit(cqp_error_status ? cqp_error_status : 1);
          }
          reading_cqprc = 0;
        }
        else if (macro_init_file) {
          Rprintf("Can't read macro initialization file %s\n", init_file_fullname);
          exit(cqp_error_status ? cqp_error_status : 1);
        }
      }
    } /* end of (if we have a macro init file, OR we are in interactive mode & able to seek for ~/.cqpmacros ...) */
  }

  check_available_corpora(UNDEF);

  /* load the default corpus. */
  if ((default_corpus) && !set_current_corpus_name(default_corpus, 0)) {
    Rprintf("Can't set current corpus to default corpus %s, exiting.\n", default_corpus);
    exit(cqp_error_status ? cqp_error_status : 1);
  }

#ifndef __MINGW__
  if (signal(SIGPIPE, SIG_IGN) == SIG_IGN)
    signal(SIGPIPE, SIG_DFL);
    /* Rprintf("Couldn't install SIG_IGN for SIGPIPE signal\n"); */
    /* -- be silent about not being able to ignore the SIGPIPE signal, which often happens in slave mode */
    /* note that SIGPIPE does not seem to exist in signal.h under MinGW */
#endif

#ifdef __MINGW__
  /* due to how the home path was calculated, home contains a malloc'ed string */
  cl_free(home);
#endif

  return 1;
}



/** Aggregator for different functions that free up global allocations. Call on shutdown. */
void
cqp_cleanup_memory(void)
{
  cleanup_kwic_line_memory();
  drop_all_corpora();
  drop_macros();
}



/**
 * Parses a stream for CQP query syntax.
 *
 * We're assuming that this is a normal pipe, IE, no interactive
 * shenanigans need to be accounted for (either Readline is unavailable,
 * or the cqp program was started without -e.).
 *
 * Note that cqp_parse_file() fclose()s fd unless it is STDOUT.
 *
 * @see cqp_parse_string
 *
 * @param src                   File handle of the file to parse.
 *                              The file may have been opened with cl_open_stream()
 *                              (but also plain FILE * objects are fine too).
 * @param exit_on_parse_errors  Boolean: should CQP stop processing this stream on parse errors?
 * @return                      Boolean: true = all ok, false = a problem.
 */
int
cqp_parse_file(FILE *src, int exit_on_parse_errors)
{
  int ok, quiet;
  int cqp_status;
  static int nesting_depth = 0; /* to catch infinite recursion */

  /* save current parser input state to resume after sourcing file */
  FILE *save_yyin;
  InputBuffer save_input_buffer;
  char *save_input_string;
  int save_input_string_ix;

  ok = 1;
  quiet = silent || (src != stdin);

  if (nesting_depth >= CQP_SOURCE_MAXNEST) {
    Rprintf("CQP: source'd CQP scripts nested too deeply (%d levels)\n", nesting_depth + 1);
    ok = 0;
  }
  else {
    nesting_depth++;
    /* we DO have space in the cqp_files stack; so pile the current yyin file handle into the file stack array. */
    save_yyin = yyin;
    save_input_buffer = InputBufferList;
    save_input_string = cqp_input_string;
    save_input_string_ix = cqp_input_string_ix;
    /* this is because if we've come from do_exec(), we need to process the current stream,
     * then return to the one from which do_exec() was raised! */

    /* pass the stream handle into the parser and re-set the parser state. */
    yyin = src;
    InputBufferList = NULL;
    cqp_input_string = NULL;
    cqp_input_string_ix = 0;
    yyrestart(yyin);

    /* main run-the-parser loop */
    while (ok && !feof(src) && !exit_cqp) {
      if (child_process && ferror(src)) {
        /* in child mode, abort on read errors (to avoid hang-up when parent has died etc.) */
        Rprintf("READ ERROR -- aborting CQP session\n");
        break;
      }

      if (!quiet) {
        /* print the prompt with the active corpus displayed. */
        if (current_corpus) {
          if (cl_streq(current_corpus->name, current_corpus->mother_name))
            Rprintf("%s> ", current_corpus->name);
          else
            Rprintf("%s:%s[%d]> ", current_corpus->mother_name, current_corpus->name, current_corpus->size);
        }
        else
          Rprintf("[no corpus]> ");
      }

      /* call the parser! This will read, interpret, and implement a command. */
      cqp_status = yyparse();

      if (cqp_status) {
        /* something went wrong; if we want to skip the rest of the stream, we therefore break the loop and return false */
        if (exit_on_parse_errors)
          ok = 0;
        /* in child mode, print the error message for parent process to spot the problem */
        if (child_process && !reading_cqprc)
          Rprintf(CQP_PARSE_ERROR_LINE);
      }

      /* in child mode, flush output streams after every parse pass. */
      if (child_process && !reading_cqprc) {
        fflush(stdout);
        fflush(stderr);
      }

    }
    /* end of loop over yyparse() calls. "ok" is now set to what we want to return. */

    /* we can now revert back to whatever the YY stream was when we began this function. */
    nesting_depth--;
    yyin = save_yyin;
    delete_macro_buffers(0); /* free any active macro expansions */
    InputBufferList = save_input_buffer;
    cqp_input_string = save_input_string;
    cqp_input_string_ix = save_input_string_ix;
  }

  /* close file handle, even if we refused to execute the script */
  if (cl_test_stream(src))
    cl_close_stream(src);
  else if (stdin != src)
    fclose(src);

  return ok;
}

/**
 * Parses a string for CQP query syntax.
 *
 * The string is assumed to have been provided by the Readline
 * library or similar. IE, we are running with "cqp -e".
 *
 * @see       USE_READLINE
 *
 * @param  s  The string to parse.
 * @return    Boolean: true = all ok, false = a problem.
 */
int
cqp_parse_string(char *s)
{
  int cqp_status;

  int ok = 1;
  int abort = 0;
  int len = strlen(s);

  cqp_input_string_ix = 0;
  cqp_input_string = s;

  while (ok && cqp_input_string_ix < len && !exit_cqp) {
    if (abort) {
      /* trying to parse a second command -> abort with error (see below) */
      cqpmessage(Error, "Multiple commands on a single line not allowed in CQPserver mode.");
      ok = 0;
      break;
    }

    /* note that, here unlike cqp_parse_file, we ALWAYS break the loop after a parse error. */
    ok = ( 0 == (cqp_status = yyparse()) );

    if (which_app == cqpserver)
      abort = 1;        /* only one command per line in CQPserver (security reasons), so break on loop */
  }

  cqp_input_string_ix = 0;
  cqp_input_string = NULL;

  return ok;
}





// see notes below.
/** Pointer for the interrupt callback function. */
InterruptCheckProc interruptCallbackHook = NULL;

// seems not to be used????????
///* *
// * Sets the interrupt callback function.
// *
// * @param f  Pointer to the function to set as interrupt callback.
// * @return   Always 1.
// */
//int
//setInterruptCallback(InterruptCheckProc f)
//{
//  interruptCallbackHook = f;
//  return 1;
//}

/**
 * Calls the interrupt callback function, if set.
 *
 * basically a nop since the callback hook is never set.
 * this is called in eval.c and targets.
 */
void
CheckForInterrupts(void)
{
  if (interruptCallbackHook)
    interruptCallbackHook();
}

