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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "server.h"
#include "auth.h"
#include "log.h"
#include "cqi.h"

#include "../cl/cl.h"

#include "../cqp/cqp.h"
#include "../cqp/options.h"
#include "../cqp/corpmanag.h"
#include "../cqp/groups.h"
void Rprintf(const char *, ...);


/** String containing the username sent by the currently-connect CQi client */
char *user = "";

/** String containing the password sent by the currently-connect CQi client */
char *passwd = "";



/*
 *
 *  Some common error messages
 *
 */

/**
 * Shuts down the server with an "unknown CQi command" error condition.
 *
 * @param cmd  The integer representing the unknown command received from the client.
 */
void
cqiserver_unknown_command_error(int cmd)
{
  cqiserver_log(Error, "unknown CQi command 0x%04X.", cmd);
  exit(1);
}

/**
 * Shuts down the server with an "CQi command not allowed here" error condition.
 *
 * @param cmd  The integer representing the wrong command received from the client.
 */
void
cqiserver_wrong_command_error(int cmd)
{
  cqiserver_log(Error, "command 0x%04X not allowed in this context.", cmd);
  exit(1);
}

/**
 * Shuts down the server with an "internal error" condition.
 *
 * Both parameters will be printed as part of the shutdown error message.
 *
 * @param function  String: should be name of the calling function, that is,
 *                  the point where the error was raised.
 * @param reason    String containing any other explanatory details about the error.
 */
void
cqiserver_internal_error(char *function, char *reason)
{
  cqiserver_log(Error, "internal error in %s()", function);
  cqiserver_log(Error, "''%s''n", reason);
  exit(1);
}


/*
 *
 *  CL and CQP error messages
 *
 */

/**
 * Sends the current CL error value to the client.
 *
 * This function takes the current contents of of the CL library's global
 * cl_errno error value and sends it to the client.
 *
 * It takes the CL error consant and translates it into the corresponding
 * CQI_CL_ERROR_* constant.
 *
 * NB: This function shuts down the server with an error condition if cl_errno
 * does not actually contain an error condition.
 *
 * @see cl_errno
 */
static void
send_cl_error(void)
{
  int cmd;

  switch (cl_errno) {
  case CDA_EATTTYPE:
    cmd = CQI_CL_ERROR_WRONG_ATTRIBUTE_TYPE;
    break;
  case CDA_EIDORNG:
  case CDA_EIDXORNG:
  case CDA_EPOSORNG:
    cmd = CQI_CL_ERROR_OUT_OF_RANGE;
    break;
  case CDA_EPATTERN:
  case CDA_EBADREGEX:
    cmd = CQI_CL_ERROR_REGEX;
    break;
  case CDA_ENODATA:
    cmd = CQI_CL_ERROR_CORPUS_ACCESS;
    break;
  case CDA_ENOMEM:
    cmd = CQI_CL_ERROR_OUT_OF_MEMORY;
    break;
  case CDA_EOTHER:
  case CDA_ENYI:
    cmd = CQI_CL_ERROR_INTERNAL;
    break;
  case CDA_OK:
    exit(cqiserver_log(Error, "send_cl_error() called with cderrno == CDA_OK") || cqp_error_status);
  default:
    exit(cqiserver_log(Error, "send_cl_error() unknown value in cderrno") || cqp_error_status);
  }
  cqiserver_debug_msg("CL error, returning 0x%04X", cmd);
  cqi_command(cmd);
  return;
}


/*
 *
 *  CQi commands  (called from interpreter loop)
 *
 */

static void
do_cqi_corpus_list_corpora(void)
{
  CorpusList *cl;
  int n = 0;

  cqiserver_debug_msg("CQI_CORPUS_LIST_CORPORA()");

  /* looping through the corpus list twice is ugly, but it's easiest ... first count system corpora, then return their names one by one */
  for (cl = FirstCorpusFromList(); cl ; cl = NextCorpusFromList(cl))
    if (cl->type == SYSTEM)
      n++;

  cqi_send_word(CQI_DATA_STRING_LIST);
  cqi_send_int(n);

  for (cl = FirstCorpusFromList(); cl ; cl = NextCorpusFromList(cl))
    if (cl->type == SYSTEM)
      cqi_send_string(cl->name);

  cqi_flush();
}

static void
do_cqi_corpus_charset(void)
{
  CorpusList *cl;
  char *corpus_name = cqi_read_string();

  cqiserver_debug_msg("CQI_CORPUS_CHARSET('%s')", corpus_name);

  cl = findcorpus(corpus_name, SYSTEM, 0);
  if (!cl || !access_corpus(cl))
    cqi_command(CQI_CQP_ERROR_NO_SUCH_CORPUS);
  else
    cqi_data_string(cl_charset_name(cl->corpus->charset));
  cl_free(corpus_name);
}

static void
do_cqi_corpus_properties(void)
{
  char *corpus_name = cqi_read_string();
  cqiserver_debug_msg("CQI_CORPUS_PROPERTIES('%s')", corpus_name);
  /* this is a dummy until we've implemented the registry extensions */
  cqi_data_string_list(NULL, 0);
  cl_free(corpus_name);
}

/** this function sends attributes of a certain type as a STRING[] to the client; helper for do_cqi_corpus_attributes() */
static void
send_cqi_corpus_attributes(Corpus *c, int type)
{
  Attribute *attribute;
  int len = 0;

  /* see note in do_cqi_corpus_list_corpora() on ugnliness... */
  for (attribute = first_corpus_attribute(c) ; attribute ; attribute = next_corpus_attribute())
    if (attribute->type == type)
      len++;

  cqi_send_word(CQI_DATA_STRING_LIST);
  cqi_send_int(len);

  for (attribute = first_corpus_attribute(c) ; attribute ; attribute = next_corpus_attribute())
    if (attribute->type == type)
      cqi_send_string(attribute->any.name);
  cqi_flush();
}

void
do_cqi_corpus_attributes(int type)
{
  char *typename;
  CorpusList *cl;

  char *corpus_name = cqi_read_string();

  if (server_debug) {
    switch (type) {
    case ATT_POS:
      typename = "POSITIONAL";
      break;
    case ATT_STRUC:
      typename = "STRUCTURAL";
      break;
    case ATT_ALIGN:
      typename = "ALIGNMENT";
      break;
    default:
      cqi_general_error("INTERNAL ERROR: do_cqi_corpus_attributes(): unknown attribute type");
      return;
    }
    cqiserver_debug_msg("CQI_CORPUS_%s_ATTRIBUTES('%s')", typename, corpus_name);
  }

  cl = findcorpus(corpus_name, SYSTEM, 0);
  if (!cl || !access_corpus(cl))
    cqi_command(CQI_CQP_ERROR_NO_SUCH_CORPUS);
  else
    send_cqi_corpus_attributes(cl->corpus, type);

  cl_free(corpus_name);
}

void
do_cqi_corpus_full_name(void)
{
  CorpusList *cl;
  char *corpus_name = cqi_read_string();

  cqiserver_debug_msg("CQI_CORPUS_FULL_NAME('%s')", corpus_name);

  cl = findcorpus(corpus_name, SYSTEM, 0);
  if (!cl || !access_corpus(cl))
    cqi_command(CQI_CQP_ERROR_NO_SUCH_CORPUS);
  else
    cqi_data_string(cl->corpus->name);

  cl_free(corpus_name);
}

void
do_cqi_corpus_structural_attribute_has_values(void)
{
  Attribute *attribute;

  char *att_name = cqi_read_string();

  cqiserver_debug_msg("CQI_CORPUS_STRUCTURAL_ATTRIBUTE_HAS_VALUES('%s')", att_name);

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_STRUC)))
    cqi_command(cqi_errno);
  else
    cqi_data_bool(cl_struc_values(attribute));
  cl_free(att_name);
}

void
do_cqi_cl_attribute_size(void)
{
  Attribute *attribute;
  int size;

  char *att_name = cqi_read_string();

  cqiserver_debug_msg("CQI_CL_ATTRIBUTE_SIZE('%s')", att_name);

  /* need to try all possible attribute types */
  if (NULL != (attribute = cqi_lookup_attribute(att_name, ATT_POS))) {
    if (0 > (size = cl_max_cpos(attribute)))
      send_cl_error();
    else
      cqi_data_int(size);
  }
  else if (NULL != (attribute = cqi_lookup_attribute(att_name, ATT_STRUC))) {
    if (0 > (size = cl_max_struc(attribute)))
      /* current version of CL considers 0 regions a data access error condition, but we want to allow that */
      cqi_data_int(0);          /* instead of : send_cl_error(); */
    else
      cqi_data_int(size);
  }
  else if (NULL != (attribute = cqi_lookup_attribute(att_name, ATT_ALIGN))) {
    if (0 > (size = cl_max_alg(attribute)))
      send_cl_error();
    else
      cqi_data_int(size);
  }
  else
    cqi_command(cqi_errno); /* return errno from the last lookup */

  cl_free(att_name);
}

void
do_cqi_cl_lexicon_size(void)
{
  Attribute *attribute;
  int size;

  char *att_name = cqi_read_string();

  cqiserver_debug_msg("CQI_CL_LEXICON_SIZE('%s')", att_name);

  if (NULL != (attribute = cqi_lookup_attribute(att_name, ATT_POS))) {
    if (0 > (size = cl_max_id(attribute)))
      send_cl_error();
    else
      cqi_data_int(size);
  }
  else
    cqi_command(cqi_errno);     /* cqi_errno set by cqi_lookup_attribute() */

  cl_free(att_name);
}



void
do_cqi_cl_drop_attribute(void)
{
  cqiserver_debug_msg("CQI_CL_DROP_ATTRIBUTE()  --  not implemented");
  cqi_general_error("CQI_CL_DROP_ATTRIBUTE not implemented.");
}

/* one might wish to add extensive error checking to all the CL functions,
   but that will need a LOT of code! */
void
do_cqi_cl_str2id(void)
{
  char **strlist;
  int i, id;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_string_list(&strlist);

  cqiserver_debug_msg("CQI_CL_STR2ID('%s', [%s])", att_name, cqiserver_debug_arglist((char *)strlist, len, 0));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++) {
      id = cl_str2id(attribute, strlist[i]);
      if (id < 0)
        id = -1;                /* -1 => string not found in lexicon */
      cqi_send_int(id);
    }
  }
  cqi_flush();
  cl_free(strlist);              /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_id2str(void)
{
  int *idlist;
  int i;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&idlist);

  cqiserver_debug_msg("CQI_CL_ID2STR('%s', [%s])", att_name, cqiserver_debug_arglist((char *)idlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_STRING_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_STRING_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++)
      cqi_send_string(cl_id2str(attribute, idlist[i])); /* sends "" if return is NULL (ID out of range) */
  }
  cqi_flush();
  cl_free(idlist);              /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_id2freq(void)
{
  int *idlist;
  int i, f;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&idlist);

  cqiserver_debug_msg("CQI_CL_ID2FREQ('%s', [%s])", att_name, cqiserver_debug_arglist((char *)idlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++) {
      f = cl_id2freq(attribute, idlist[i]);
      if (f < 0)
        f = 0;                  /* return 0 if ID is out of range */
      cqi_send_int(f);
    }
  }
  cqi_flush();
  cl_free(idlist);               /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_cpos2str(void)
{
  int *cposlist;
  int i;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&cposlist);

  cqiserver_debug_msg("CQI_CL_CPOS2STR('%s', [%s])", att_name, cqiserver_debug_arglist((char *)cposlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_STRING_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_STRING_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++)
      cqi_send_string(cl_cpos2str(attribute, cposlist[i]));     /* sends "" if return is NULL (cpos out of range) */
  }
  cqi_flush();
  cl_free(cposlist);            /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_cpos2id(void)
{
  int *cposlist;
  int i, id;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&cposlist);

  cqiserver_debug_msg("CQI_CL_CPOS2ID('%s', [%s])", att_name, cqiserver_debug_arglist((char *)cposlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++) {
      id = cl_cpos2id(attribute, cposlist[i]);
      if (id < 0)
        id = -1;                        /* return -1 if cpos is out of range */
      cqi_send_int(id);
    }
  }
  cqi_flush();
  cl_free(cposlist);                     /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_cpos2struc(void)
{
  int *cposlist;
  int i, struc;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&cposlist);

  cqiserver_debug_msg("CQI_CL_CPOS2STRUC('%s', [%s])", att_name, cqiserver_debug_arglist((char *)cposlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_STRUC)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++) {
      if (0 > (struc = cl_cpos2struc(attribute, cposlist[i])))
        struc = -1;                     /* return -1 if cpos is out of range */
      cqi_send_int(struc);
    }
  }
  cqi_flush();
  cl_free(cposlist);                    /* don't forget to free allocated memory */
  cl_free(att_name);
}

/* cqi_cl_cpos2lbound() and cqi_cl_cpos2rbound() are currently temporary functions
   for the Euralex2000 tutorial; they will probably become part of the CQi specification,
   and should be improved with a caching model to avoid the frequent cpos2struc lookup;
   perhaps make them CL functions with an intelligent caching algorithm? */
void
do_cqi_cl_cpos2lbound(void)
{
  int *cposlist;
  int i, struc, lb, rb;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&cposlist);

  cqiserver_debug_msg("CQI_CL_CPOS2LBOUND('%s', [%s])", att_name, cqiserver_debug_arglist((char *)cposlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_STRUC)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++)
      if (0 > (struc = cl_cpos2struc(attribute, cposlist[i])))
        cqi_send_int(-1);     /* return -1 if cpos is not in region */
      else if (cl_struc2cpos(attribute, struc, &lb, &rb))
        cqi_send_int(lb);
      else
        cqi_send_int(-1);     /* cannot return error within list, so send -1 */
  }
  cqi_flush();
  cl_free(cposlist);                    /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_cpos2rbound(void)
{
  int *cposlist;
  int i, struc, lb, rb;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&cposlist);

  cqiserver_debug_msg("CQI_CL_CPOS2RBOUND('%s', [%s])", att_name, cqiserver_debug_arglist((char *)cposlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_STRUC)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate att_name temporary list */
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++) {
      struc = cl_cpos2struc(attribute, cposlist[i]);
      if (struc < 0)
        cqi_send_int(-1);                       /* return -1 if cpos is not in region */
      else {
        if (cl_struc2cpos(attribute, struc, &lb, &rb))
          cqi_send_int(rb);
        else
          cqi_send_int(-1);     /* cannot return error within list, so send -1 */
      }
    }
  }
  cqi_flush();
  cl_free(cposlist);                    /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_cpos2alg(void)
{
  int *cposlist;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&cposlist);

  cqiserver_debug_msg("CQI_CL_CPOS2ALG('%s', [%s])", att_name, cqiserver_debug_arglist((char *)cposlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_ALIGN)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_INT_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    int i, alg;
    cqi_send_word(CQI_DATA_INT_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++) {
      alg = cl_cpos2alg(attribute, cposlist[i]);
      cqi_send_int(alg < 0 ? -1 : alg); /* return -1 if cpos is out of range */
    }
  }
  cqi_flush();
  cl_free(cposlist);                     /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_struc2str(void)
{
  int *struclist;
  int i;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&struclist);

  cqiserver_debug_msg("CQI_CL_STRUC2STR('%s', [%s])", att_name, cqiserver_debug_arglist((char *)struclist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_STRUC)))
    cqi_command(cqi_errno);
  else {
    /* we assemble the CQI_DATA_STRING_LIST() return command by hand,
       so we don't have to allocate a temporary list */
    cqi_send_word(CQI_DATA_STRING_LIST);
    cqi_send_int(len);          /* list size */
    for (i=0; i<len; i++)
      cqi_send_string(cl_struc2str(attribute, struclist[i]));     /* sends "" if return is NULL (wrong alignment number) */
  }
  cqi_flush();
  cl_free(struclist);                    /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_id2cpos(void)
{
  int *cposlist;
  int len;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int id = cqi_read_int();

  cqiserver_debug_msg("CQI_CL_ID2CPOS('%s', %d)\n", att_name, id);

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else if (!(cposlist = cl_id2cpos(attribute, id, &len)))
    send_cl_error();
  else {
    cqi_data_int_list(cposlist, len);
    cl_free(cposlist);
  }

  cl_free(att_name);                      /* don't forget to free allocated space */
}

void
do_cqi_cl_idlist2cpos(void)
{
  int *idlist, *cposlist;
  int cposlen;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int len = cqi_read_int_list(&idlist);

  cqiserver_debug_msg("CQI_CL_IDLIST2CPOS('%s', [%s])", att_name, cqiserver_debug_arglist((char *)idlist, len, 1));

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else if (!(cposlist = cl_idlist2cpos(attribute, idlist, len, 1, &cposlen)))
    send_cl_error();
  else {
    cqi_data_int_list(cposlist, cposlen);
    cl_free(cposlist);
  }

  cqi_flush();
  cl_free(idlist);               /* don't forget to free allocated memory */
  cl_free(att_name);
}

void
do_cqi_cl_regex2id(void)
{
  int *idlist;
  int len;
  Attribute *attribute;

  char *att_name = cqi_read_string();
  char *regex = cqi_read_string();

  cqiserver_debug_msg("CQi: CQI_CL_REGEX2ID('%s', '%s')\n", att_name, regex);

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_POS)))
    cqi_command(cqi_errno);
  else if (!(idlist = cl_regex2id(attribute, regex, 0, &len))) {
    if (cl_errno != CDA_OK)
      send_cl_error();
    else
      cqi_data_int_list(NULL, 0); /* no matches -> zero size list */
  }
  else {
    cqi_data_int_list(idlist, len);
    cl_free(idlist);
  }
  cl_free(regex);
  cl_free(att_name);              /* don't forget to free allocated space */
}

void
do_cqi_cl_struc2cpos(void)
{
  int struc, start, end;
  char *att_name;
  Attribute *attribute;

  att_name = cqi_read_string();
  struc = cqi_read_int();

  cqiserver_debug_msg("CQI_CL_STRUC2CPOS('%s', %d)", att_name, struc);

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_STRUC)))
    cqi_command(cqi_errno);
  else if (cl_struc2cpos(attribute, struc, &start, &end))
    cqi_data_int_int(start, end);
  else
    send_cl_error();

  cl_free(att_name);                      /* don't forget to free allocated space */
}

void
do_cqi_cl_alg2cpos(void)
{
  Attribute *attribute;

  char *att_name = cqi_read_string();
  int alg = cqi_read_int();

  cqiserver_debug_msg("CQI_CL_ALG2CPOS('%s', %d)\n", att_name, alg);

  if (!(attribute = cqi_lookup_attribute(att_name, ATT_ALIGN)))
    cqi_command(cqi_errno);
  else {
    int s1, s2, t1, t2;
    if (cl_alg2cpos(attribute, alg, &s1, &s2, &t1, &t2))
      cqi_data_int_int_int_int(s1, s2, t1, t2);
    else
      send_cl_error();
  }
  cl_free(att_name);                      /* don't forget to free allocated space */
}

void
do_cqi_cqp_list_subcorpora(void)
{
  CorpusList *cl, *mother;
  int n = 0;

  char *corpus = cqi_read_string();

  cqiserver_debug_msg("CQI_CQP_LIST_SUBCORPORA(%s)\n", corpus);

  if (!check_corpus_name(corpus) || !(mother = cqi_find_corpus(corpus)))
    cqi_command(cqi_errno);
  else {
    /* as noted in do_cqi_cqp_list_corpora: a double loop is ugly, but it's easiest ...
     * so first count subcorpora, then return names one by one */
    for (cl = FirstCorpusFromList(); cl ; cl = NextCorpusFromList(cl))
      if (cl->type == SUB && cl->corpus == mother->corpus)
        n++;

    cqi_send_word(CQI_DATA_STRING_LIST);
    cqi_send_int(n);

    for (cl = FirstCorpusFromList(); cl ; cl = NextCorpusFromList(cl))
      if (cl->type == SUB && cl->corpus == mother->corpus)
        cqi_send_string(cl->name);
    cqi_flush();
  }

  cl_free(corpus);
}

/**
 * Remove any trailing semicolon terminators from a query
 *
 * CQP commands must be terminated with a single semicolon;
 * multiple semicolons will be interpreted as empty commands and
 * produce an error in do_cqi_cqp_query(), which allows only
 * a single command to be executed for safety reasons.
 * So we remove all semicolons from the query here, then add
 * a single semicolon when constructing the full CQP command.
 * Note that a query sent via the CQi API should _not_ have a
 * trailing semicolon in the first place, so this function
 * just provides some leniency for user errors.
 *
 * @param   query  the query string, which may be truncated in-place
 */
static inline void
query_remove_semicolon(char *query)
{
  char *p;
  if (!query || !*query)
    return;
  p = query + strlen(query) - 1;
  while (p >= query && (*p == ' ' || *p == '\t' || *p == ';'))
    p--; /* move p past any trailing whitespace and semicolon characters */
  *(p + 1) = '\0'; /* p is the last character to be preserved */
}

void
do_cqi_cqp_query(void)
{
  char *corpus_name, *sc_name;

  char *mother = cqi_read_string();
  char *child = cqi_read_string();
  char *query = cqi_read_string();

  cqiserver_debug_msg("CQI_CQP_QUERY('%s', '%s', '%s')", mother, child, query);
  query_remove_semicolon(query);

  if (!split_subcorpus_spec(mother, &corpus_name, &sc_name))
    cqi_command(cqi_errno);
  else {
    int len = strlen(child) + strlen(query) + 10;
    char *cqp_query = (char *) cl_malloc(len);

    if (!check_subcorpus_name(child) || !cqi_activate_corpus(mother))
      cqi_command(cqi_errno);
    else {
      query_lock = floor(1e9 * cl_random_fraction()) + 1; /* activate query lock mode with random key */
      cqiserver_log(Info, "query_lock = %d\n", query_lock);

      snprintf(cqp_query, len, "%s = %s;", child, query);
      if (!cqp_parse_string(cqp_query))
        cqi_command(CQI_CQP_ERROR_GENERAL); /* should be changed to detailed error messages */
      else {
        char *full_child = combine_subcorpus_spec(corpus_name, child); /* corpus_name is the 'physical' part of the mother corpus */;
        CorpusList *childcl = cqi_find_corpus(full_child);

        if (!childcl)
          cqi_command(CQI_CQP_ERROR_GENERAL);
        else {
          cqiserver_log(Info, "'%s' ran the following query on %s\n\t%s\n\tand got %d matches.", user, mother, cqp_query, childcl->size);
          cqi_command(CQI_STATUS_OK);
        }
        cl_free(full_child);
      }
      query_lock = 0;           /* deactivate query lock mode */
    }
    cl_free(cqp_query);
  }
  cl_free(corpus_name);
  cl_free(sc_name);
}

void
do_cqi_cqp_subcorpus_size(void)
{
  CorpusList *cl;

  char *subcorpus = cqi_read_string();

  cqiserver_debug_msg("CQI_CQP_SUBCORPUS_SIZE('%s')", subcorpus);

  if (!(cl = cqi_find_corpus(subcorpus)))
    cqi_command(cqi_errno);
  else
    cqi_data_int(cl->size);

  cl_free(subcorpus);
}

/**
 * Returns const string representations of CQI_CONST_FIELD_ values. (DO NOT FREE.)
 *
 * Utility function, used for debugging output & to check valid fields in subroutines below.
 */
static inline const char *
cqi_field_name(cqi_byte field)
{
  switch (field) {
  case CQI_CONST_FIELD_MATCH:
    return "MATCH";
  case CQI_CONST_FIELD_MATCHEND:
    return "MATCHEND";
  case CQI_CONST_FIELD_TARGET:
    return "TARGET";
  case CQI_CONST_FIELD_KEYWORD:
    return "KEYWORD";
  default:
    return NULL;    /* invalid field */
  }
}

void
do_cqi_cqp_subcorpus_has_field(void)
{
  int field_ok = 1;             /* field valid? */
  CorpusList *cl;
  const char *field_name;

  char *subcorpus = cqi_read_string();
  cqi_byte field  = cqi_read_byte();

  if (!(field_name = cqi_field_name(field))) {
    field_name = "<invalid field>";
    field_ok = 0;
  }

  cqiserver_debug_msg("CQI_CQP_SUBCORPUS_HAS_FIELD('%s', %s)\n", subcorpus, field_name);

  if (!(cl = cqi_find_corpus(subcorpus)))
    cqi_command(cqi_errno);
  else if (!field_ok)
    cqi_command(CQI_CQP_ERROR_INVALID_FIELD);
  else {
    switch (field) {
    case CQI_CONST_FIELD_MATCH:
      cqi_data_bool(CQI_CONST_YES);
      break;
    case CQI_CONST_FIELD_MATCHEND:
      cqi_data_bool(CQI_CONST_YES);
      break;
    case CQI_CONST_FIELD_TARGET:
      if (cl->targets == NULL)
        cqi_data_bool(CQI_CONST_NO);
      else
        cqi_data_bool(CQI_CONST_YES);
      break;
    case CQI_CONST_FIELD_KEYWORD:
      if (cl->keywords == NULL)
      cqi_data_bool(cl->keywords ? CQI_CONST_YES : CQI_CONST_NO);
      else
        cqi_data_bool(CQI_CONST_YES);
      break;
    default:
      cqiserver_internal_error("do_cqi_cqp_subcorpus_has_field", "Can't identify requested field.");
    }
    cqi_flush();
  }

  cl_free(subcorpus);
}

/**
 * Sends n instances of integer -1 to the client.
 *
 * Utility function for do_cqi_cqp_dump_subcorpus().
 *
 * This is the error condition of the CQI_CQP_DUMP_SUBCORPUS command:
 * it returns a list of (-1) values if requested field is not set.
 *
 * It is assumed that the length of the lsit has already been sent.
 *
 * @param n  Length of list to send.
 */
void
do_cqi_send_minus_one_list(int n)
{
  while (n--)
    cqi_send_int(-1);
}

void
do_cqi_cqp_dump_subcorpus(void)
{
  CorpusList *cl;
  const char *field_name;
  int field_ok = 1;             /* field valid? */

  char *subcorpus = cqi_read_string();
  cqi_byte field = cqi_read_byte();
  int first = cqi_read_int();
  int last = cqi_read_int();

  if (!(field_name = cqi_field_name(field))) {
    field_name = "<invalid field>";
    field_ok = 0;
  }

  cqiserver_debug_msg("CQI_CQP_DUMP_SUBCORPUS('%s', %s, %d, %d)", subcorpus, field_name, first, last);

  if (!(cl = cqi_find_corpus(subcorpus)))
    cqi_command(cqi_errno);
  else if (!field_ok)
    cqi_command(CQI_CQP_ERROR_INVALID_FIELD);
  else if (last < first || first < 0 || last >= cl->size)
    cqi_command(CQI_CQP_ERROR_OUT_OF_RANGE);
  else {
    int i, size;
    cqi_send_word(CQI_DATA_INT_LIST); /* assemble by hand, so we don't have to allocate a temporary list */
    size = last - first + 1;
    cqi_send_int(size);
    switch (field) {
    case CQI_CONST_FIELD_MATCH:
      for (i=first; i<=last; i++)
        cqi_send_int(cl->range[i].start);
      break;
    case CQI_CONST_FIELD_MATCHEND:
      for (i=first; i<=last; i++)
        cqi_send_int(cl->range[i].end);
      break;
    case CQI_CONST_FIELD_TARGET:
      if (cl->targets == NULL)
        do_cqi_send_minus_one_list(size);
      else
        for (i=first; i<=last; i++)
          cqi_send_int(cl->targets[i]);
      break;
    case CQI_CONST_FIELD_KEYWORD:
      if (cl->keywords == NULL)
        do_cqi_send_minus_one_list(size);
      else
        for (i=first; i<=last; i++)
          cqi_send_int(cl->keywords[i]);
      break;
    default:
      cqiserver_internal_error("do_cqi_cqp_dump_subcorpus", "No handler for requested field.");
    }
    cqi_flush();
  }

  cl_free(subcorpus);
}

void
do_cqi_cqp_drop_subcorpus(void)
{
  CorpusList *cl;
  char *corpus_name, *sc_name;

  char *subcorpus = cqi_read_string();

  cqiserver_debug_msg("CQI_CQP_DROP_SUBCORPUS('%s')", subcorpus);

  /* make sure it is a subcorpus, not a root corpus */
  if (!split_subcorpus_spec(subcorpus, &corpus_name, &sc_name))
    cqi_command(cqi_errno);
  else if (!sc_name) {
    cl_free(corpus_name);
    cqi_command(CQI_ERROR_SYNTAX_ERROR);
  }
  else {
    cl_free(corpus_name);
    cl_free(sc_name);
    if (!(cl = cqi_find_corpus(subcorpus)))
      cqi_command(cqi_errno);
    else {
      dropcorpus(cl, NULL);
      cqi_command(CQI_STATUS_OK);
    }
  }

  cl_free(subcorpus);
}

/* temporary functions for CQI_CQP_FDIST_1() and CQI_CQP_FDIST_2() */
void
do_cqi_cqp_fdist_1(void)
{
  CorpusList *cl;
  Group *table;
  const char *field_name;
  FieldType field_type = NoField;
  int field_ok = 1;             /* field valid? */

  char *subcorpus = cqi_read_string();
  int cutoff      = cqi_read_int();
  cqi_byte field  = cqi_read_byte();
  char *att_name  = cqi_read_string();

  /* not exactly the fastest way to do it ... */
  if (!(field_name = cqi_field_name(field))) {
    field_name = "<invalid field>";
    field_ok = 0;
  }
  else
    field_type = field_name_to_type(field_name);

  cqiserver_debug_msg("CQI_CQP_FDIST_1('%s', %d, %s, %s)\n", subcorpus, cutoff, field_name, att_name);

  if (!(cl = cqi_find_corpus(subcorpus)))
    cqi_command(cqi_errno);
  else if (!field_ok)
    cqi_command(CQI_CQP_ERROR_INVALID_FIELD);
  else {
    /* compute_grouping() returns tokens with f > cutoff, but CQi specifies f >= cutoff */
    cutoff = (cutoff > 0) ? cutoff - 1 : 0;
    table = compute_grouping(cl, NoField, 0, NULL, field_type, 0, att_name, cutoff, 0, NULL);
    if (table == NULL)
      cqi_command(CQI_CQP_ERROR_GENERAL);
    else {
      int i, size = table->nr_cells;
      cqi_send_word(CQI_DATA_INT_TABLE);        /* return table with 2 columns & <size> rows */
      cqi_send_int(size);
      cqi_send_int(2);
      for (i=0; i < size; i++) {
        cqi_send_int(table->count_cells[i].t);
        cqi_send_int(table->count_cells[i].freq);
      }
      cqi_flush();
      free_group(&table);
    }
  }

  cl_free(subcorpus);
  cl_free(att_name);
}


void
do_cqi_cqp_fdist_2(void)
{
  CorpusList *cl;
  Group *table;
  int i, size;
  const char *field_name1, *field_name2;
  FieldType field_type1 = NoField, field_type2 = NoField;
  int fields_ok = 1;    /* (both) fields valid? */

  char *subcorpus = cqi_read_string();
  int cutoff      = cqi_read_int();
  cqi_byte field1 = cqi_read_byte();
  char *att_name1 = cqi_read_string();
  cqi_byte field2 = cqi_read_byte();
  char *att_name2 = cqi_read_string();

  /* not exactly the fastest way to do it ... */
  if (!(field_name1 = cqi_field_name(field1))) {
    field_name1 = "<invalid field>";
    fields_ok = 0;
  }
  else
    field_type1 = field_name_to_type(field_name1);
  if (!(field_name2 = cqi_field_name(field2))) {
    field_name2 = "<invalid field>";
    fields_ok = 0;
  }
  else
    field_type2 = field_name_to_type(field_name2);


  cqiserver_debug_msg("CQI_CQP_FDIST_2('%s', %d, %s, %s, %s, %s)", subcorpus, cutoff,
                      field_name1, att_name1,
                      field_name2, att_name2);

  if (!(cl = cqi_find_corpus(subcorpus)))
    cqi_command(cqi_errno);
  else if (!fields_ok)
    cqi_command(CQI_CQP_ERROR_INVALID_FIELD);
  else {
    /* compute_grouping() returns tokens with f > cutoff, but CQi specifies f >= cutoff */
    cutoff = (cutoff > 0) ? cutoff - 1 : 0;
    if (!(table = compute_grouping(cl, field_type1, 0, att_name1, field_type2, 0, att_name2, cutoff, 0, NULL)))
      cqi_command(CQI_CQP_ERROR_GENERAL);
    else {
      size = table->nr_cells;
      cqi_send_word(CQI_DATA_INT_TABLE);        /* return table with 3 columns & <size> rows */
      cqi_send_int(size);
      cqi_send_int(3);
      for (i=0; i < size; i++) {
        cqi_send_int(table->count_cells[i].s);
        cqi_send_int(table->count_cells[i].t);
        cqi_send_int(table->count_cells[i].freq);
      }
      cqi_flush();
      free_group(&table);
    }
  }

  cl_free(subcorpus);
  cl_free(att_name1);
  cl_free(att_name2);
}


/**
 *
 *  The CQP server's command interpreter loop.
 *
 *  The loops starts running when this function is called, and when the
 *  exit command is reveived (CQI_CTRL_BYE)
 *  (returns on exit)
 *
 */
static void
interpreter(void)
{
  int cmd;
  int cmd_group;

  while (42) {
    cmd = cqi_read_command();
    cmd_group = cmd >> 8;

    switch (cmd_group) {

      /* GROUP CQI_CTRL_* */
    case CQI_CTRL:
      switch (cmd) {
      case CQI_CTRL_CONNECT:
        cqiserver_wrong_command_error(cmd);
        break;
      case CQI_CTRL_BYE:
        cqiserver_debug_msg("CQI_CTRL_BYE()");
        cqi_command(CQI_STATUS_BYE_OK);
        return;                 /* exit CQi command interpreter */
      case CQI_CTRL_USER_ABORT:
        cqiserver_debug_msg("CQI_CTRL_ABORT signal ... ignored");
        break;
      case CQI_CTRL_PING:
        cqiserver_debug_msg("CQI_CTRL_PING()");
        cqi_command(CQI_STATUS_PING_OK);
        break;
      case CQI_CTRL_LAST_GENERAL_ERROR:
        cqiserver_debug_msg("CQI_CTRL_LAST_GENERAL_ERROR() => '%s'", cqi_error_string);
        cqi_data_string(cqi_error_string);
        break;
      default:
        cqiserver_unknown_command_error(cmd);
      }
      break;

      /* GROUP CQI_ASK_FEATURE_* */
    case CQI_ASK_FEATURE:
      switch (cmd) {
      case CQI_ASK_FEATURE_CQI_1_0:
        cqiserver_debug_msg("CQI_ASK_FEATURE_CQI_1_0 ... CQi v1.0 ok");
        cqi_data_bool(CQI_CONST_YES);
        break;
      case CQI_ASK_FEATURE_CL_2_3:
        cqiserver_debug_msg("CQI_ASK_FEATURE_CL_2_3 ... CL v2.3 ok");
        cqi_data_bool(CQI_CONST_YES);
        break;
      case CQI_ASK_FEATURE_CQP_2_3:
        cqiserver_debug_msg("CQI_ASK_FEATURE_CQP_2_3 ... CQP v2.3 ok");
        cqi_data_bool(CQI_CONST_YES);
        break;
      default:
        cqiserver_debug_msg("CQI_ASK_FEATURE_* ... <unknown feature> not supported");
        cqi_data_bool(CQI_CONST_NO);
      }
      break;

      /* GROUP CQI_CORPUS_* */
    case CQI_CORPUS:
      switch (cmd) {
      case CQI_CORPUS_LIST_CORPORA:
        do_cqi_corpus_list_corpora();
        break;
      case CQI_CORPUS_CHARSET:
        do_cqi_corpus_charset();
        break;
      case CQI_CORPUS_PROPERTIES:
        do_cqi_corpus_properties();
        break;
      case CQI_CORPUS_POSITIONAL_ATTRIBUTES:
        do_cqi_corpus_attributes(ATT_POS);
        break;
      case CQI_CORPUS_STRUCTURAL_ATTRIBUTES:
        do_cqi_corpus_attributes(ATT_STRUC);
        break;
      case CQI_CORPUS_STRUCTURAL_ATTRIBUTE_HAS_VALUES:
        do_cqi_corpus_structural_attribute_has_values();
        break;
      case CQI_CORPUS_ALIGNMENT_ATTRIBUTES:
        do_cqi_corpus_attributes(ATT_ALIGN);
        break;
      case CQI_CORPUS_FULL_NAME:
        do_cqi_corpus_full_name();
        break;
      default:
        cqiserver_unknown_command_error(cmd);
      }
      break;

      /* GROUP CQI_CL_* */
    case CQI_CL:
      switch (cmd) {
      case CQI_CL_ATTRIBUTE_SIZE:
        do_cqi_cl_attribute_size();
        break;
      case CQI_CL_LEXICON_SIZE:
        do_cqi_cl_lexicon_size();
        break;
      case CQI_CL_DROP_ATTRIBUTE:
        do_cqi_cl_drop_attribute();
        break;
      case CQI_CL_STR2ID:
        do_cqi_cl_str2id();
        break;
      case CQI_CL_ID2STR:
        do_cqi_cl_id2str();
        break;
      case CQI_CL_ID2FREQ:
        do_cqi_cl_id2freq();
        break;
      case CQI_CL_CPOS2ID:
        do_cqi_cl_cpos2id();
        break;
      case CQI_CL_CPOS2STR:
        do_cqi_cl_cpos2str();
        break;
      case CQI_CL_CPOS2STRUC:
        do_cqi_cl_cpos2struc();
        break;
      case CQI_CL_CPOS2LBOUND:
        do_cqi_cl_cpos2lbound();
        break;
      case CQI_CL_CPOS2RBOUND:
        do_cqi_cl_cpos2rbound();
        break;
      case CQI_CL_CPOS2ALG:
        do_cqi_cl_cpos2alg();
        break;
      case CQI_CL_STRUC2STR:
        do_cqi_cl_struc2str();
        break;
      case CQI_CL_ID2CPOS:
        do_cqi_cl_id2cpos();
        break;
      case CQI_CL_IDLIST2CPOS:
        do_cqi_cl_idlist2cpos();
        break;
      case CQI_CL_REGEX2ID:
        do_cqi_cl_regex2id();
        break;
      case CQI_CL_STRUC2CPOS:
        do_cqi_cl_struc2cpos();
        break;
      case CQI_CL_ALG2CPOS:
        do_cqi_cl_alg2cpos();
        break;
      default:
        cqiserver_unknown_command_error(cmd);
      }
      break;

      /* GROUP CQI_CQP_* */
    case CQI_CQP:
      switch (cmd) {
      case CQI_CQP_QUERY:
        do_cqi_cqp_query();
        break;
      case CQI_CQP_LIST_SUBCORPORA:
        do_cqi_cqp_list_subcorpora();
        break;
      case CQI_CQP_SUBCORPUS_SIZE:
        do_cqi_cqp_subcorpus_size();
        break;
      case CQI_CQP_SUBCORPUS_HAS_FIELD:
        do_cqi_cqp_subcorpus_has_field();
        break;
      case CQI_CQP_DUMP_SUBCORPUS:
        do_cqi_cqp_dump_subcorpus();
        break;
      case CQI_CQP_DROP_SUBCORPUS:
        do_cqi_cqp_drop_subcorpus();
        break;
      case CQI_CQP_FDIST_1:
        do_cqi_cqp_fdist_1();
        break;
      case CQI_CQP_FDIST_2:
        do_cqi_cqp_fdist_2();
        break;
      default:
        cqiserver_unknown_command_error(cmd);
      }
      break;

    default:
      cqiserver_unknown_command_error(cmd);

    } /* end outer switch */

  } /* end while 42 */

}



/**
 * Main function for the cqpserver app.
 */
int
main(int argc, char *argv[])
{
  int cmd;

  which_app = cqpserver;
  silent = 1;
  paging = autoshow = auto_save = 0;

  if (!initialize_cqp(argc, argv))
    exit(cqiserver_log(Error, "ERROR Couldn't initialise CQP engine.") || cqp_error_status);

  for ( ; optind < argc ; optind++) {
    /* remaining command-line arguments are <user>:<password> specifications */
    char *sep = strchr(argv[optind], ':');
    if (sep != NULL) {
      if (sep == argv[optind])
        exit(cqiserver_log(Error, "Invalid account specification '%s' (username must not be empty)", argv[optind]) || cqp_error_status);
      else {
        *sep = '\0';
        add_user_to_list(argv[optind], sep + 1);
      }
    }
    else
      exit(cqiserver_log(Error, "Invalid account specification '%s' (password missing)", argv[optind]) || cqp_error_status);
  }

  cqiserver_welcome();

  if (localhost)
    add_host_to_list("127.0.0.1"); /* in -L mode, connections from localhost are automatically accepted  */

  if (0 < accept_connection(server_port))
    cqiserver_log(Info, "Connected. Waiting for CONNECT request.");
  else
    exit(cqiserver_log(Error, "ERROR Connection failed.") || cqp_error_status);

  /* establish CQi connection: wait for CONNECT request */
  cmd = cqi_read_command();
  if (cmd != CQI_CTRL_CONNECT) {
    cqiserver_log(Info, "Invalid command received. Connection refused.");
    cqiserver_wrong_command_error(cmd);
  }
  user = cqi_read_string();
  passwd = cqi_read_string();
  cqiserver_log(Info, "CONNECT  user = '%s'  passwd = '%s'  pid = %d", user, passwd, (int)getpid());

  /* check password here (always required !!) */
  if (!authenticate_user(user, passwd)) {
    cqiserver_log(Error, "Wrong username or password. Connection refused.\n");
    cqi_command(CQI_ERROR_CONNECT_REFUSED);
  }
  else {
    CorpusList *cl;

    cqi_command(CQI_STATUS_CONNECT_OK);

    /* re-seed the CL RNG for query lock key generation */
    cl_randomize();

    /* check which corpora the user is granted access to */
    for (cl = FirstCorpusFromList() ; cl ; cl = NextCorpusFromList(cl))
      if (!check_grant(user, cl->name))
        dropcorpus(cl, NULL);

    /* start command interpreter loop */
    interpreter();

    /* interpreter call only returns when the user has disconnected */
    cqiserver_log(Info, "User '%s' has logged off.\n", user);
  }

  /* connection terminated; clean up memory and exit */

  cqp_cleanup_memory();

  cqiserver_log(Info, "Exit with status #%d (pid = %d)\n", cqp_error_status, (int)getpid());

  return cqp_error_status;
}

