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

/**
 * @file
 *
 * Functions that encapsulate CQi / CQPserver logging and printing of debug messages.
 */

#include <stdio.h>
#include <stdarg.h>

#include "../cl/cl.h"          /* for CL_MAX_LINE_LENGTH */
#include "../cl/cwb-globals.h" /* to ensure we have CWB_VERSION */

#include "../cqp/output.h" /* for MessageType */
#include "../cqp/options.h"

#include "cqi.h"

/*
 *
 *  General logging
 *
 */

/**
 * Prints the CQi server welcome and copyright message.
 */
void
cqiserver_welcome(void)
{
  Rprintf("** WELCOME TO CQPserver v" CWB_VERSION "\n");
  Rprintf("   implementing version %d.%d of the CQi\n", CQI_MAJOR_VERSION, CQI_MINOR_VERSION);
  Rprintf("   for copyright and other info, see `cqp -v`%s", server_log ? "\n" : "\n\n"); /* if writing to log, skip decorative newline */
}

/**
 * Prints a message to log (if logging enabled); if the message is an error,
 * or debugging is switched on, also prints to stderr.
 *
 * Note that we only distinguish between "Error" and "anything else" in terms of
 * MessageType; it's recommended to pass Info if the message is not an error.
 *
 * @return  Boolean. True if msg_type was Error, otherwise false
 *          (caller can use this as anexit() argument if none other is available)
 */
int
cqiserver_log(MessageType msg_type, const char *msg, ...)
{
  va_list vector;

  /* if logging is on, print to stdout, which is the logging dest */
  if (server_log || Error == msg_type) {
    va_start(vector, msg);
    Rprintf("CQPserver: ");
    vprintf(msg, vector);
    Rprintf("\n");
    va_end(vector);
  }

  /* if it's an error or if debug is on, print to stderr, which is where all debug messages go */
  if (Error == msg_type || server_debug) {
    va_start(vector, msg);
    Rprintf("CQPserver: ");
    Rprintf(msg, vector);
    Rprintf("\n");
    va_end(vector);
    return Error == msg_type;
  }

  return 0;
}




/*
 *
 *  Snooping and debugging
 *
 */

/**
 * Print network communication snooping info on stderr (if global "snoop" option is true).
 *
 * @param format   A printf-style formatting string.
 *                 Must be followed by values to satisfy each specifier in the striong.
 */
void
cqiserver_snoop(const char *format, ...)
{
  if (snoop) {
    va_list vector;
    va_start(vector, format);
    Rprintf("CQi ");
    Rprintf(format, vector);
    Rprintf("\n");
    va_end(vector);
  }
}



/**
 * Support function for calling cqiserver_debug_msg;
 * stringifies a variable-length array of int/string arguments for presentation.
 *
 * @param  arg_list  Array of char pointers or ints. The parameter is a
 *                   pointer-to-byte, so a typecast may be needed in the call.
 * @param  n_args    Number of entries in arg_list.
 * @param  int_args  Boolean: if true, arguments are treted as integers;
 *                   if false, as strings.
 * @return           String with space-delimited argument list
 *                   (internal buffer, do not alter or free).
 *                   If the list is empty, the string returned is just a space.
 */
const char *
cqiserver_debug_arglist(const char *arg_list, int n_args, int int_args)
{
  char *mark;
  int i;

  /* arg_list is an array of bytes. So in the loop, we need to increment by the explicit size of the elements. No pointer arithmentic. */
  int incr = int_args ? sizeof(int) : sizeof(char *);
  /* pain in the posterior given that char * and int are likely the same storage type anyway. Never mind! */

  static char buf[CL_MAX_LINE_LENGTH] = { ' ' , '\0' };
  if (server_debug) {
    for (i = 0, mark = buf ; i < n_args ; i += incr)
      if (int_args)
        sprintf(mark += strlen(mark), "%d ", (int)arg_list[i]);
      else
        /* super creaky typecasting needed to get the compiler to not complain here */
        sprintf(mark += strlen(mark), "'%s' ", arg_list);
  }
  return buf;
}



/**
 * Print server debug message on stderr (if global "server_debug" option is true).
 *
 * @param format   A printf-style formatting string.
 *                 Must be followed by values to satisfy each specifier in the striong.
 */
void
cqiserver_debug_msg(const char *format, ...)
{
  if (server_debug) {
    va_list vector;
    va_start(vector, format);
    Rprintf("CQi: ");
    Rprintf(format, vector);
    Rprintf("\n");
    va_end(vector);
  }
}


