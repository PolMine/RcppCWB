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

#ifndef _cqi_log_h_
#define _cqi_log_h_

#include "../cqp/output.h" /* for MessageType */

void cqiserver_welcome(void);

int cqiserver_log(MessageType msg_type, const char *msg, ...);

void cqiserver_snoop(const char *format, ...);

const char *cqiserver_debug_arglist(const char *arg_list, int n_args, int int_args);

void cqiserver_debug_msg(const char *format, ...);

#endif /* _cqi_log_h_ */
