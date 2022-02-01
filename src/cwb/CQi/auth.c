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

#ifndef __MINGW__
#include <arpa/inet.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void Rprintf(const char *, ...);

#include "auth.h"
#include "../cl/cl.h"

/* data structures (internal use only) */

/** Internal data structure: name of a corpus to which access is granted */
typedef struct Grant {
  char *corpus;
  struct Grant *next;
} Grant;

/** Internal data structure: a username, the user's password, and the top of a linked list of Grants */
typedef struct UserEntry {
  char *name;
  char *passwd;
  Grant *grants;
  struct UserEntry *next;
} UserEntry;

/** Internal data structure: member of list of IP addresses from which messages are accepted */
typedef struct HostEntry {
  int    accept_any;        /**< this implements the "host *;" command */
  struct in_addr address;
  struct HostEntry *next;
} HostEntry;

/** non-exported global variable for cqpserver's user list */
UserEntry *authorized_users = NULL;

/** non-exported global variable for cqpserver's host list */
HostEntry *authorized_hosts = NULL;


/*
 * internal utilities
 */

/**
 * Finds an entry the user with the specified username
 * on the global user list
 *
 * @see authorized_users
 */
UserEntry *
find_user(char *username)
{
  UserEntry *user;
  for (user = authorized_users ; user ; user = user->next)
    if (cl_streq(user->name, username))
      break;
  return user;
}



/*
 *  implementation of public functions
 */

void
add_user_to_list(char *user, char *passwd)
{
  UserEntry *new_user;

  if (find_user(user))
    Rprintf("WARNING: user '%s' already in list (ignored)\n", user);
  else {
    new_user = (UserEntry *)cl_malloc(sizeof(UserEntry));
    new_user->name   = cl_strdup(user);
    new_user->passwd = cl_strdup(passwd);
    new_user->grants = NULL;
    new_user->next   = authorized_users;
    authorized_users = new_user;
  }
}

void
add_grant_to_last_user(char *corpus)
{
  Grant *grant;

  assert(authorized_users);        /* need a 'last user' in list */
  grant = (Grant *) cl_malloc(sizeof(Grant));
  grant->corpus = cl_strdup(corpus);
  grant->next = authorized_users->grants;
  authorized_users->grants = grant;
}

void
add_host_to_list(char *ipaddr)
{
  HostEntry *host = (HostEntry *) cl_malloc(sizeof(HostEntry));

  if (!ipaddr) {
    host->accept_any = 1;        /* accept connection from any host */
    host->address.s_addr = 0;
  }
  else {
    host->accept_any = 0;
    host->address.s_addr = inet_addr(ipaddr);
  }

  if (host->address.s_addr == -1) {
    Rprintf("WARNING: '%s' isn't a valid IP address (ignored)\n", ipaddr);
    cl_free(host);
  }
  else {
    host->next = authorized_hosts;
    authorized_hosts = host;
  }
}

void
add_hosts_in_subnet_to_list(char *ipsubnet)
{
  char *ipaddr = cl_malloc(strlen(ipsubnet) + 4);    /* 3 digits, NUL */
  int i;

  for (i = 1; i <= 255; i++) {
    sprintf(ipaddr, "%s%d", ipsubnet, i);
    add_host_to_list(ipaddr);
  }
  cl_free(ipaddr);
}

/* returns true if host is in list of allowed hosts */
int
check_host(struct in_addr host_addr)
{
  HostEntry *host;
  for (host = authorized_hosts; host ; host = host->next)
    if (host->accept_any || (host->address.s_addr == host_addr.s_addr))
      break;
  return host ? 1 : 0;
}

/** returns true if (user, passwd) pair is in list */
int
authenticate_user(char *username, char *passwd)
{
  UserEntry *user = find_user(username);
  return user && cl_streq(user->passwd, passwd);
}

/** returns true if user may access the specified corpus */
int
check_grant(char *username, char *corpus)
{
  UserEntry *user;
  Grant *grant;

  if (!(user = find_user(username)))
    return 0;
  if (!(grant = user->grants))
    return 1;             /* user may access all corpora if no specific grants are set */

  for ( ; grant ; grant = grant->next )
    if (cl_streq(grant->corpus, corpus))
      break;
  return grant ? 1 : 0;
}


/** prints all hosts/users/grants to stdout for debugging purposes */
void
show_grants(void)
{
  UserEntry *user;
  HostEntry *host;
  Grant *grant;

  for (host = authorized_hosts; host; host = host->next)
    Rprintf("HOST: %s\n", inet_ntoa(host->address));

  for (user = authorized_users; user; user = user->next) {
    Rprintf("USER: %s, pass='%s'  (", user->name, user->passwd);
    for (grant = user->grants; grant; grant = grant->next)
      Rprintf("%s ", grant->corpus);
    Rprintf(")\n");
  }
}

