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
#include <stdlib.h>
#include <string.h>

#include "../cl/cl.h"

#include "attlist.h"

/**
 * Creates a new AttributeList.
 *
 * @param element_type  What type of attribute is this? ATT_POS, ATT_STRUC, etc.
 * @return              Pointer to the new AttributeList object.
 */
AttributeList *
NewAttributeList(int element_type)
{
  AttributeList *list = (AttributeList *)cl_malloc(sizeof(AttributeList));

  list->list = NULL;
  list->element_type = element_type;
  list->list_valid = 0;

  return list;
}

/**
 * Deletes an AttributeList object.
 *
 * @param list  Address of the pointer to the list to delete. NULL-safe.
 * @return      Always 1.
 */
int
DestroyAttributeList(AttributeList **list)
{
  AttributeInfo *curr, *next;

  if (!list)
    return 1;

  if (*list)
    for (curr = (*list)->list ; curr ; curr = next) {
      next = curr->next;
      cl_free(curr->name);
      cl_free(curr);
    }

  cl_free(*list);
  return 1;
}

/**
 * Adds a new AttributeInfo to an AttributeList object.
 *
 * @param list              The list to add to.
 * @param name              The name of the Attribute that this AttributeInfo refers to.
 *                          If the list already contains an att with this name, nothing happens.
 * @param init_show_status  Initial setting for the status member of the new AttributeInfo
 *                          (true implies "show +the_att", false implies "show -the_att")
 * @param position          If this is 1, the new AttributeInfo is added at the beginning
 *                          of the list. If it is 0, it is added at the end of the list.
 *                          Otherwise, this specifies a particular insertion position
 *                          (the given number of steps down the linked list).
 * @return                  A pointer to the new AttributeInfo, or NULL for error.
 */
AttributeInfo
*AddNameToAL(AttributeList *list, char *name, int init_show_status, int position)
{
  AttributeInfo *prev, *ai;

  if (FindInAL(list, name))
    return NULL;

  ai = (AttributeInfo *)cl_malloc(sizeof(AttributeInfo));
  ai->status = init_show_status;
  ai->name = cl_strdup(name);
  ai->attribute = NULL;
  ai->next = NULL;
  ai->prev = NULL;

  if (list->list == NULL)
    list->list = ai;
  else {
    /* list-> not empty */
    if (position == 1) {
      /* insert at beginning */
      ai->next = list->list;
      list->list = ai;
    }
    else if (position == 0) {
      /* insert new element at end of list */
      for (prev = list->list ; prev->next ; prev = prev->next)
        ;
      ai->prev = prev;
      prev->next = ai;
    }
    else {
      /* insert new element at certain position */
      for (prev = list->list ; prev->next && position > 2 ; prev = prev->next )
        position--;

      ai->prev = prev;
      ai->next = prev->next;
      prev->next->prev = ai;
      prev->next = ai;
    }
  }

  /* return the new element after flagging the list as not-verified*/
  list->list_valid = 0;
  return ai;
}

/**
 * Deletes an AttributeInfo from the AttributeList.
 *
 * @param list   The list from which to delete.
 * @param name   The name of the AttributeInfo to delete.
 * @return       True if the attribute info was found and deleted;
 *               otherwise false.
 */
int
RemoveNameFromAL(AttributeList *list, char *name)
{
  AttributeInfo *curr, *prev;

  if (!list->list)
    /* list is empty, nothing to do. */
    return 0;

  prev = NULL;
  curr = list->list;

  while (curr && !cl_streq(curr->name, name)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    /* not a member of the list */
    return 0;

  /* curr now points to the member with the given name.  unchain it! */
  if (!prev) {
    /* this is first element of attribute list */
    list->list = curr->next;
    if (curr->next)
      curr->next->prev = list->list;
  }
  else {
    prev->next = curr->next;
    if (curr->next)
      curr->next->prev = prev;
  }

  cl_free(curr->name);
  cl_free(curr);
  return 1;
}

/**
 * Gets the number of entries in an AttributeList which have their "show" status switched on/off.
 *
 * @param list  The list to size up.
 * @param shown If true, counts "show +" elements. If false, counts "show -" elements.
 * @return      The number of elements where status == shown.
 */
int
NrOfElementsWithStatusAL(AttributeList *list, int shown)
{
  AttributeInfo *curr;
  int n = 0;
  for (curr = list->list ; curr ; curr = curr->next)
    if (shown ? curr->status : !curr->status) /* slightly paranoid way of checking status based on possibility of true > 1 */
      n++;
  return n;
}

/**
 * Finds an entry in an AttributeList.
 *
 * @param list  The list to search.
 * @param name  The name of the element to search for.
 * @return      Pointer to the AttributeInfo with the matching name,
 *              or NULL if the name was not found.
 */
AttributeInfo *
FindInAL(AttributeList *list, char *name)
{
  AttributeInfo *curr;

  if (!list || !list->list)
    return NULL;

  for (curr = list->list ; curr && !cl_streq(curr->name, name) ; curr = curr->next)
    ;

  return curr;
}


/**
 * Goes through an AttributeList, deletes entries for attributes
 * that don't exist, and adds entries for those that do.
 *
 * Note that all AttributeInfo entries are linked to the actual Attribute objects by this function.
 *
 * As of CQP v3.4.18, this function re-creates the AttributeList from
 * scratch and copies the current display status of attributes with the
 * same name from the previous list. In this way it ensures that attributes
 * are always displayed in the same order in which they are listed in
 * the registry file.
 *
 * @param list         The AttributeList to recompute.
 * @param corpus       The corpus in which these attributes are found.
 * @param init_status  Not currently used.
 * @return             The updated AttributeList (which will have been re-allocated)
 */
AttributeList *
RecomputeAL(AttributeList *list, Corpus *corpus, int init_status)
{
  /* silly implementation, but usually short lists. so what... */

  Attribute *attr;
  AttributeInfo *ai, *prev;
  AttributeList *addition = NewAttributeList(list->element_type);

  if (corpus) {
    /* fill new AttributeList with suitable attribute type */
    for (attr = corpus->attributes; attr; attr = attr->any.next)
      if (attr->type == list->element_type) {
        ai = AddNameToAL(addition, attr->any.name, 0, 0);
        assert(ai); /* should never fail */
        ai->attribute = attr;
        if (NULL != (prev = FindInAL(list, ai->name)))
          ai->status = prev->status;
      }
  }

  addition->list_valid = 1; /* we have valid Attribute objects for all entries */
  DestroyAttributeList(&list);

  return addition;
}


/**
 * Verifies an AttributeList.
 *
 * Each AttributeInfo entry on the list is linked to the actual Attribute object by this function.
 *
 * If the relevant Attribute cannot be found, the entry is deleted iff remove_illegal_entries.
 *
 * @param list                    The list to verify.
 * @param corpus                  The corpus for which this list should be valid.
 * @param remove_illegal_entries  Boolean: see function description.
 * @return                        Boolean: true if the list entries are all now vlaid, otherwise false.
 *                                This result value is also stored in the AtttributeList's "list_valid" member.
 */
int
VerifyList(AttributeList *list, Corpus *corpus, int remove_illegal_entries)
{
  int all_ok = 1;

  /* linked-list entry pointers */
  AttributeInfo *next = NULL, *prev = NULL, *curr = NULL;

  if (!list)
    return 0;

  next = list->list;

  while (next) {
    curr = next;
    next = next->next;
    curr->attribute = cl_new_attribute(corpus, curr->name, list->element_type);

    if (!curr->attribute) {
      /* we've found a list entry whose Attribute can't be validated */
      if (remove_illegal_entries) {
        if (prev == NULL) {
          /* delete first element */
          list->list = next;
          if (next)
            next->prev = NULL;
        }
        else {
          /* unchain current element */
          prev->next = next;
          if (next)
            next->prev = prev;
        }

        cl_free(curr->name);
        cl_free(curr);
      }
      else
        /* if we can't remove the evil entry, then the list IS NOT verified. */
        all_ok = 0;
    }
    else
      /* the Attribute is just fine, so get ready to scroll. */
      prev = curr;
  }

  return list->list_valid = all_ok;
}

