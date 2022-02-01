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

#ifndef _cqp_attlist_h_
#define _cqp_attlist_h_

#include "../cl/attributes.h"
#include "../cl/corpus.h"

/* ======================================== Data types */

/**
 * An AttributeInfo is an entry in an AttributeList.
 *
 * As well as linked-list overhead (for double-linked list!), it contains
 * the Attribute itself, and a bool indicating whether
 * that Att is set to be printed in concordances (with show+ ...)
 */
typedef struct _attrbuf {
  char *name;                           /**< name of the attribute */
  Attribute *attribute;                 /**< the relevant Attribute object, only valid if list_valid==1 */
  int status;                           /**< Boolean. This is user-settable; 0 on initialisation.
                                             It indicates whether an attribute is to be printed (activation with show +...)
                                             Masybe rename show_status? */
  struct _attrbuf *next;                /**< Next in chain. */
  struct _attrbuf *prev;                /**< Previous in chain */
} AttributeInfo;

/** The AttributeList object: holds a list of Attributes of the same type ().
 *  Its methods are all in CamelCase for some reason.  */
typedef struct _attlist {

  int list_valid;                       /**< Are all the Attributes in this list valid? 0: check list */
  int element_type;                     /**< One of the constants defined in attributes.h, format:ATT_x */

  AttributeInfo *list;                  /**< Head of the linked list of attribute-info structures. */

} AttributeList;

/* ======================================== Allocation & Deallocation */

AttributeList *NewAttributeList(int element_type);

int DestroyAttributeList(AttributeList **list);

/* ======================================== Adding and Removing */

AttributeInfo *AddNameToAL(AttributeList *list, char *name, int initial_status, int position);

int RemoveNameFromAL(AttributeList *list, char *name);

//int NrOfElementsAL(AttributeList *list);

int NrOfElementsWithStatusAL(AttributeList *list, int shown);

AttributeInfo *FindInAL(AttributeList *list, char *name);

AttributeList *RecomputeAL(AttributeList *list, Corpus *corpus, int initial_status);

int VerifyList(AttributeList *list, Corpus *corpus, int remove_illegal_entries);



#endif
