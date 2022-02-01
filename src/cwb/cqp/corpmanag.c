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

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../cl/cl.h"
#include "../cl/fileutils.h"       /* for file_length */
#include "../cl/ui-helpers.h"

#include "corpmanag.h"
#include "cqp.h"
#include "options.h"
#include "output.h"
#include "ranges.h"

#define COLON ':'
#define SLASH '^'

/** If this is 1, then debug info is printed on subcorpus load. */
#define subcorpload_debug 0

/* module-internal function prototypes */
static Boolean attach_subcorpus(CorpusList *cl, char *advertised_directory, char *advertised_filename);
static CorpusList *GetSystemCorpus(char *name, char *registry);


/**
 * Global pointer to the head of CQP's linked list of the currently-loaded corpora (and NQRs).
 */
static CorpusList *corpuslist = NULL;


/**
 * Global pointer to the "current" corpus.
 */
CorpusList *current_corpus;

/**
 * Resets a CorpusList object to empty.
 *
 * This is done, largely, by freeing all its members
 * (and setting other members to 0 or NULL)...
 *
 * @param cl         The corpus list to initialise.
 * @param free_name  Boolean: the name, mother_name and mother_size members
 *                   will be cleared iff free_name.
 */
static void
initialize_cl(CorpusList *cl, int free_name)
{
  if (!cl)
    return;

  if (free_name) {
    cl_free(cl->name);
    cl_free(cl->mother_name);
    cl->mother_size = 0;
  }
  cl_free(cl->registry);
  cl_free(cl->range);
  cl_free(cl->abs_fn);

  cl->type = UNDEF;
  cl->saved = cl->loaded = cl->needs_update = False;

  cl_free(cl->query_corpus);
  cl_free(cl->query_text);

  cl->corpus = NULL;
  cl->size = 0;

  cl_free(cl->cd);

  cl_free(cl->sortidx);

  cl_free(cl->targets);
  cl_free(cl->keywords);
}

/**
 * Initialises the global corpus list (sets it to NULL, no matter what its value was).
 */
void
init_global_corpuslist(void)
{
  corpuslist = NULL;
  set_current_corpus(NULL, 0);
}

/** Backend destructor for CorpusList object. Unlike dropcorpus(), it doesn't check the global corpuslist. */
static void
destruct_cl(CorpusList *cl)
{
  if (cl) {
    initialize_cl(cl, True); /* to free its internal string memory */
    cl_free(cl);
  }
}


/**
 * Creates a new CorpusList object.
 */
static CorpusList *
NewCL(void)
{
  CorpusList *cl;

  cl = (CorpusList *)cl_malloc(sizeof(CorpusList));

  cl->name = NULL;
  cl->mother_name = NULL;
  cl->mother_size = 0;
  cl->registry = NULL;
  cl->abs_fn = NULL;
  cl->type = UNDEF;

  cl->local_dir = NULL;

  cl->query_corpus = NULL;
  cl->query_text = NULL;

  cl->saved = False;
  cl->loaded = False;
  cl->needs_update = False;
  cl->corpus = NULL;
  cl->range = NULL;
  cl->size = 0;
  cl->sortidx = NULL;
  cl->targets = NULL;
  cl->keywords = NULL;

  cl->cd = NULL;
  cl->next = NULL;

  return cl;
}


/**
 * Returns a FieldType enumeration corresponding to the field name
 * indicated by its string argument.
 */
FieldType
field_name_to_type(const char *name)
{
  if (cl_streq_ci(name, "nofield"))
    return NoField;
  else if (cl_streq_ci(name, "keyword"))
    return KeywordField;
  else if (cl_streq_ci(name, "target") || cl_streq_ci(name, "collocate"))
    return TargetField;
  else if (cl_streq_ci(name, "match"))
    return MatchField;
  else if (cl_streq_ci(name, "matchend"))
    return MatchEndField;
  else
    return NoField;
}


/**
 * Returns a pointer to an internal constant string that labels
 * the FieldType argument.
 */
const char *
field_type_to_name(FieldType ft) {
  switch (ft) {
  case NoField:
    return "nofield";
  case MatchField:
    return "match";
  case MatchEndField:
    return "matchend";
  case TargetField:
    return "target";
  case KeywordField:
    return "keyword";
  default:
    cqpmessage(Error, "Internal Error: Unknown field type #%d", ft);
    return "";
  }
}



/** A utility function required by ensure_corpus_size() */
static int
SystemCorpusSize(Corpus *corpus)
{
  Attribute *attr;

  if (NULL != (attr = cl_new_attribute(corpus, CWB_DEFAULT_ATT_NAME, ATT_POS)))
    return cl_max_cpos(attr);
  else
    return -1;
}


/**
 * This is an internal function used to ensure that a system corpus
 * from the corpus list is accessible and that its size has been
 * computed. In case of subcorpora, this function implements delayed
 * loading. It is necessary because of a hack that prevents CQP from
 * determining the sizes of all know corpora at start-up (which caused
 * annoying delays if one or more corpora are not accessible) and from
 * reading all subcorpora in the local corpus directory (which caused
 * a number of delays and crashes with MP templates).
 * ensure_corpus_size() is needed by findcorpus() and
 * ensure_syscorpus() at the very least. It may be needed in other
 * places to keep CQP from crashing.
 *
 * @param cl The corpus whose accessibility is to be checked.
 * @return   Boolean: true if access is OK.
 */
static Boolean
ensure_corpus_size(CorpusList *cl)
{
  if (cl->type == SYSTEM) {
    /* System corpus: check corpus size (may have to be computed now) */

    /* Check whether or not the size of the corpus has already been determined. */
    if (cl->mother_size <= 0) { /* for a system corpus, this is its size */
      /* Corpus size hasn't been computed yet, so we must do it now */

      int attr_size = SystemCorpusSize(cl->corpus);
      if (attr_size <= 0) {
        cqpmessage(Warning, "Data access error (%s)\n"
                   "Perhaps the corpus %s is not accessible from the machine you are using.",
                   cl_error_string(cl_errno), cl->name);

        /* If we couldn't access the corpus, remove it from the list so
           the user isn't tempted to try again.
           Very Microsoftish, isn't it? ;-) */
        dropcorpus(cl, NULL);

        /* Now tell the calling function that corpus is inaccessible */
        return False;
      }

      /* Set size related fields in corpus list entry */
      cl->mother_size = attr_size;
      cl->range[0].end = attr_size - 1;

      /* OK */
      return True;
    }
  }
  else if (cl->type == SUB) {
    /* Subcorpus: load into memory if necessary */

    if (!cl->loaded) {
      /* load subcorpus (the local_dir entry of the corpus structure contains
         the name of the directory where the disk file can be found */
      char filename[CL_MAX_FILENAME_LENGTH];

      /* re-create subcorpus filename from corpus structure
         (cf. the treatment in load_corpusnames()) */
      if (cl->mother_name == NULL)
        strcpy(filename, cl->name);
      else
        sprintf(filename, "%s:%s", cl->mother_name, cl->name);
      return attach_subcorpus(cl, cl->local_dir, filename);
    }

    /* if it's already loaded, just return OK */
    return True;
  }

  /* If there are any other types, at least we don't need to do anything
     with them, so just return True in this case. */
  return True;
}


/**
 * Finds a loaded corpus.
 *
 * This function tries to find the corpus with name 'name' in the list of currently
 * loaded corpora. In case of subcorpora, qualifier is the mother's
 * name. in case of system corpora, qualifier is the registry. If
 * qualifier is NULL, it is neglected and the first matching corpus is
 * returned. If type is not UNDEF, only a corpus of that type can be
 * returned. No side effects take place.
 *
 * @param name       The corpus we are looking for.
 * @param qualifier  An extra "bit" of the corpus name (see function description).
 * @param type       Which type of corpus is wanted (may be UNDEF, which finds any except TEMP).
 * @return           Pointer to the CorpusList of the corpus that was found, or NULL if not found.
 */
CorpusList *
LoadedCorpus(char *name, char *qualifier, CorpusType type)
{
  CorpusList *corp;

  for (corp = corpuslist; corp; corp = corp->next) {
    if (corp->type == type || (type == UNDEF && corp->type != TEMP)) {
      /* the type is ok. Check the name. */
      if (cl_streq(corp->name, name)) {

        /* ignoring the qualifier is OK for system corpora (although its behaviour is not very well defined if there are
           multiple corpora with the same name in different registry directories), but it can get really messy with subcorpora;
           just imagine you've got two subcorpora with the same name (say, A) for two system corpora (say, BNC and WSJ); you
           have activated the BNC, type "cat A;", and what you get are the results for WSJ because LoadedCorpus() happens to
           find WSJ:A first; thus, if we are comparing an unqualified name to a subcorpus, we use the currently activated corpus
           as a qualifier; if no corpus is activated, we revert to guessing, which is useful when a subcorpus is loaded from
           disk immediately after startup */

        /* the name is also ok. Check the qualifier */

        if (qualifier == NULL) {
          /* no qualifier was specified.... so TRY TO GUESS. */
          if (corp->type == SUB) {
            if (current_corpus) {
              char *qualifier; /* don't overwrite the function argument which will be checked again in the next loop iteration! */

              /* use a qualifier guessed from current_corpus to compare */
              qualifier = (current_corpus->type == SUB ? current_corpus->mother_name : current_corpus->name);
              if (cl_streq(corp->mother_name, qualifier))
                return corp;
            }
            else
              /* no current corpus - no guesswork possible. */
              return corp;
          }
          else
            /* the candidate is not a subcorpus - guesswork possible. */
            return corp;
        }
        else
          /* we DO have a qualifier */
          if ( (corp->type == SYSTEM && cl_streq(corp->registry, qualifier))
               ||
               (corp->type == SUB    && cl_streq(corp->mother_name, qualifier))
             )
            return corp;
          /* otherwise continue search */
      }
    }
  }

  return NULL;
}


/**
 * Finds the pointer to the corpus (or subcorpus, or query result) with the given name.
 *
 * When searching for s (name of corpus) strcmp() is used; no
 * case conversion is done.
 *
 * If "type" is UNDEF, it returns the first
 * corpus with matching name. Otherwise the returned corpus
 * has the type "type".
 *
 * @param s                     Name of the corpus to find (as string)
 * @param type                  If this is UNDEF, all corpora are checked;
 *                              if it is any other type, only corpora of that
 *                              type are checked.
 * @param try_recursive_search  Boolean: whether or not to try to find corpus
 *                              through implicit expansion.
 * @return                      Pointer to the CorpusList object for the
 *                              specified corpus. NULL is returned when the
 *                              corpus is not yet present.
 */
CorpusList *
findcorpus(char *s, CorpusType type, int try_recursive_search)
{
  int i;
  CorpusList *sp, *tmp;
  char *colon;

  char mother_corpus[CL_MAX_LINE_LENGTH];
  char *expansion;
  char *real_name;

  if (type != SYSTEM) {
    if (NULL != (colon = strchr(s, COLON))) {
      cl_strcpy(mother_corpus, s);
      mother_corpus[colon - s] = '\0';
      real_name = colon + 1;
    }
    else {
      mother_corpus[0] = '\0';
      real_name = s;
    }
  }
  else {
    real_name = s;
    mother_corpus[0] = '\0';
  }

  expansion = strchr(real_name, SLASH);

  if (expansion)
    /* don't re-use existing subcorpus generated by implicit expansion */
    sp = NULL;
  else
    sp = LoadedCorpus(real_name, (mother_corpus[0] == '\0' ? NULL : mother_corpus), type);

  /* We need to call ensure_corpus_size() to implement delayed loading. */
  if (sp)
    if (!ensure_corpus_size(sp))
      return NULL;

  /* (sp==0): try to find corpus through implicit expansion ("A^s" == "A expand to s") */
  if (type != SYSTEM && expansion && try_recursive_search) {
    char new_name[CL_MAX_LINE_LENGTH];
    Context ctx;

    cl_strcpy(new_name, real_name);
    new_name[expansion - real_name] = '\0';

    sp = LoadedCorpus(new_name, (mother_corpus[0] == '\0' ? NULL : mother_corpus), type);

    if (sp) {
      if (!ensure_corpus_size(sp)) /* delayed loading */
        return NULL;
      if (!access_corpus(sp))
        return NULL;
      if (sp->type == SYSTEM) {
        cqpmessage(Warning, "Implicit expansion %s only allowed for named query result.", s);
        return NULL;
      }

      assert(sp->corpus);

      if (!(ctx.attrib = cl_new_attribute(sp->corpus, expansion+1, ATT_STRUC))) {
        cqpmessage(Warning, "Can't expand to <%s> regions -- no such structural attribute in %s", expansion+1, sp->mother_name);
        return NULL;
      }
      ctx.direction = ctxtdir_leftright;
      ctx.space_type = structure;
      ctx.size = 1;

      tmp = duplicate_corpus(sp, real_name, True);

      if (!tmp) {
        Rprintf("Internal error in findcorpus() -- this should not happen!\n");
        return NULL;
      }

      for (i = 0; i < tmp->size; i++) {
        int left, right;
        left  = calculate_leftboundary(tmp, tmp->range[i].start, ctx);
        right = calculate_rightboundary(tmp, tmp->range[i].end, ctx);

        if (left >= 0 && right >= 0) {
          tmp->range[i].start = left;
          tmp->range[i].end   = right;
        }
      }

      apply_range_set_operation(tmp, RUniq, NULL, NULL);
      touch_corpus(tmp);
      return tmp;
    }
  }

  return sp;
}


/**
 * Remove a corpus from the global list of corpora.
 *
 * @see                  corpuslist
 * @param cl             The corpus to drop. If NULL, nothing happens.
 * @param search_from    If NULL, dropcorpus searches from the start of the global list.
 *                       Otherwise, it starts at the point indicated -- which must be an item
 *                       known to be BEFORE cl in the global list, or else the global list itself.
 *                       (This makes looping through the whole corpuslist, and deleting some, more efficient.)
 * @return               Pointer to previous item on the linked list
 *                       (which now directly precedes the next item) or, if the first item was
 *                       deleted, to the new head of the list. Only of use if it is fed back
 *                       to "search_from" in a subsequent iteration of a loop through the corpuslist
 *                       (but with caution: if the first item was deleted, it now points to the
 *                       following item in the list, which must not be skipped in the next iteration).
 *                       Returns NULL if the dropped corpus was not found on the part of the list searched
 *                       or if it was the only corpus in corpuslist (which is now empty). See drop_temp_corpora().
 */
CorpusList *
dropcorpus(CorpusList *cl, CorpusList *search_from)
{
  CorpusList *prev;

  if (!cl)
    return NULL;

  if (!corpuslist) {
    Rprintf("%s:dropcorpus(): cl is not in list of loaded corpora (list empty)\n", __FILE__);
    return NULL;
  }

  /* we can't search from search_from if it is equal to cl */
  if (cl == search_from)
    search_from = NULL; /* search from start of corpuslist */

  /* look through the  chain of corpora */
  if (cl == corpuslist)
    /* delete first element of the list */
    prev = corpuslist = cl->next;

  else {
    /* scroll prev till it points at the entry before cl (or reaches terminating NULL) */
    for (prev = (search_from ? search_from : corpuslist) ; prev ; prev = prev->next) {
      if (prev->next == cl) {
        prev->next = cl->next; /* make chain hop over cl */
        break;
      }
    }
    if (!prev) {
      /* loop went across whole of the list! */
      Rprintf("%s:dropcorpus(): cl is not in %slist of loaded corpora\n", __FILE__, search_from ? "the searched-through part of " : "");
      return NULL;
    }
  }

  /* Current corpus must be changed, if it is the one about to be erased from memory!
   * Corpus switches to the first in the linked list of loaded corpora. */
  if (current_corpus == cl)
    set_current_corpus(corpuslist, 0);

  destruct_cl(cl);

  return prev;
}


/**
 * Duplicate a CorpusList object.
 *
 * duplicate_corpus creates a copy of an existing corpus
 * and casts its type to SUB. The new corpus is given the
 * name "new_name". If a subcorpus of that name is already
 * present, NULL is returned if force_overwrite is False. If
 * force_overwrite is True, the old corpus is discarded.
 *
 * This is used for commands likel, for instance,
 *
 * @param cl               The corpus to duplicate
 * @param new_name         Name for the duplicated corpus.
 * @param force_overwrite  Boolean: whether or not to force an
 *                         overwrite if the subcorpus you are
 *                         attempting to create already exists.
 * @return                 NULL if you attempted to overwrite
 *                         with force_overwrite == false.
 *                         Otherwise, a CorpusList pointer to
 *                         the new corpus.
 */
CorpusList *
duplicate_corpus(CorpusList *cl, char *new_name, Boolean force_overwrite)
{
  CorpusList *newc; /* the (existent or non existent) corplist to be overwritten) */

  if (cl == NULL) {
    Rprintf("%s:duplicate_corpus(): WARNING: Called with NULL corpus\n", __FILE__);
    return NULL;
  }

  /* newc = findcorpus(new_name, SUB, 0); */
  newc = LoadedCorpus(new_name, cl->type == SYSTEM ? cl->registry : cl->mother_name, SUB);

  /* try to make a copy of myself? */
  if (newc == cl) {
    if (force_overwrite) {
      /* I do not need to copy anything, just fake that I destroyed the old one.
       * Leave all flags as they were.
       */
      cqpmessage(Warning, "LHS and RHS are identical in assignment (ignored)\n");
      return cl;
    }
    else
      /* I am not allowed to overwrite myself, so say that I did not succeed. */
      return NULL;
  }

  /* we only get here when newc != cl */
  assert(newc != cl);

  if (!newc) {
    /* a CL entry with this name did not exist; so insert it at the start of the chain of corpora */
    newc = NewCL();
    newc->next = corpuslist;
    corpuslist = newc;
  }
  else if (force_overwrite)
    /* a list entry with this name did exist; prepare it for overwrite by clearing all fields */
    initialize_cl(newc, True);
  else
    /* a list entry with this name did exist; but overwrite is not allowed */
    newc = NULL;

  if (newc) {
    /* newc is not NULL iff we are about to make that copy (to a new object, or one forcefully scrubbed).
     * newc is "fresh", i.e., all fields either have just been allocated or are re-initialized.
     */
    newc->name = cl_strdup(new_name);
    newc->mother_name = cl_strdup(cl->mother_name);
    newc->mother_size = cl->mother_size;
    newc->registry = cl_strdup(cl->registry);
    newc->abs_fn = NULL;
    newc->type = SUB;

    /* TODO: does copying the query_* fields really make sense? */

    newc->query_corpus = cl->query_corpus ? cl_strdup(cl->query_corpus) : NULL;
    newc->query_text = cl->query_text ? cl_strdup(cl->query_text) : NULL;

    newc->saved = False;
    newc->loaded = cl->loaded;
    newc->needs_update = True;

    newc->corpus = cl->corpus;
    newc->size = cl->size;

    if (newc->size > 0) {
      /* duplicate the memory containing the cpos interval data */
      newc->range = (Range *)cl_malloc(sizeof(Range) * newc->size);
      memcpy((void *)newc->range, (void *)cl->range, sizeof(Range) * newc->size);
    }
    else
      newc->range = NULL;

    /* and now check for other blocks of data to duplicate */
    if (cl->sortidx) {
      newc->sortidx = (int *)cl_malloc(cl->size * sizeof(int));
      /* bcopy(cl->sortidx, newc->sortidx, cl->size * sizeof(int)); */
      memcpy(newc->sortidx, cl->sortidx, cl->size * sizeof(int));
    }
    else
      newc->sortidx = NULL;

    if (cl->targets) {
      newc->targets = (int *)cl_malloc(cl->size * sizeof(int));
      /* bcopy(cl->targets, newc->targets, cl->size * sizeof(int)); */
      memcpy(newc->targets, cl->targets, cl->size * sizeof(int));
    }
    else
      newc->targets = NULL;

    if (cl->keywords) {
      newc->keywords = (int *)cl_malloc(cl->size * sizeof(int));
      memcpy(newc->keywords, cl->keywords, cl->size * sizeof(int));
    }
    else
      newc->keywords = NULL;

    /* TODO: decide whether we should copy the position_lists */
  }

  if (auto_save)
    save_subcorpus(newc, NULL);

  return newc;
}


/**
 * Copy a corpus as type TEMP.
 *
 * make_temp_corpus makes a copy of the corpus in *cl
 * into a corpus of type "TEMP" with name "new_name".
 * If a temporary corpus with that name already exists,
 * it is overwritten.
 *
// TODO this is a replica of LOTS AND LOTS of the code from duplicate_corpus.
 *
 * @param cl         The corpus to copy.
 * @param new_name   Name for the temporary copy.
 * @return           NULL for error. Otherwise, a
 *                   CorpusList pointer to the new corpus.
 */
CorpusList *
make_temp_corpus(CorpusList *cl, char *new_name)
{
  CorpusList *newc;

  if (!cl) {
    Rprintf("%s:duplicate_corpus(): WARNING: Called with NULL corpus\n", __FILE__);
    return NULL;
  }

  newc = findcorpus(new_name, TEMP, 0);

  /* try to make a copy of myself? */
  if (newc == cl)
    /* we do not need to copy anything, just fake that
     * we destroyed the old one. Leave all flags as they were.
     */
    return cl;

  if (!newc) {
    newc = NewCL();
    /* insert it into the chain of corpora */
    newc->next = corpuslist;
    corpuslist = newc;
  }
  else
    initialize_cl(newc, True);  /* clear all fields of newc */

  if (newc) {
    /* newc is not NULL iff we are about to make that copy.
     * newc is "fresh", i.e., all fields either have just been
     * allocated or are re-initialized.
     */
    newc->name = cl_strdup(cl->name);
    newc->mother_name = cl_strdup(cl->mother_name);
    newc->mother_size = cl->mother_size;
    newc->registry = cl_strdup(cl->registry);
    newc->abs_fn = NULL;
    newc->type = TEMP;

    newc->query_corpus = cl->query_corpus ? cl_strdup(cl->query_corpus) : NULL;
    newc->query_text = cl->query_text ? cl_strdup(cl->query_text) : NULL;

    newc->saved = False;
    newc->loaded = cl->loaded;
    newc->needs_update = False; /* we never want to save a TEMP corpus */

    newc->corpus = cl->corpus;
    newc->size = cl->size;
    newc->sortidx = NULL;
    newc->keywords = NULL;

    /* copy the targets. Thu May 11 12:33:23 1995 (oli) */

    if (cl->targets) {
      assert(newc->size > 0);
      newc->targets = (int *)cl_malloc(sizeof(int) * newc->size);
      memcpy((char *)newc->targets, (char *)cl->targets, sizeof(int) * newc->size);

    }
    else
      newc->targets = NULL;

    /* and the keywords ... (evert) */

    if (cl->keywords) {
      assert(newc->size > 0);
      newc->keywords = (int *)cl_malloc(sizeof(int) * newc->size);
      memcpy((char *)newc->keywords, (char *)cl->keywords, sizeof(int) * newc->size);

    }
    else
      newc->keywords = NULL;

    if (newc->size > 0) {
      newc->range = (Range *)cl_malloc(sizeof(Range) * newc->size);
      memcpy((char *)newc->range, (char *)cl->range, sizeof(Range) * newc->size);
    }
    else
      newc->range = NULL;
  }

  return newc;
}


/**
 * Convert a temporary corpus to a real subcorpus.
 *
 * assign_temp_to_sub assigns the temporary corpus in *tmp
 * to a "real" subcorpus with name "subname". If such a
 * subcorpus already exists, it is overwritten. The temporary
 * corpus is deleted afterwards. The return value is the new
 * subcorpus (which may be equal to tmp, but not necessarily).
 *
 * @param tmp      Temporary corpus to convert.
 * @param subname  Name to use for new subcorpus.
 * @return         Pointer to new subcorpus.
 */
CorpusList *
assign_temp_to_sub(CorpusList *tmp, char *subname)
{
  CorpusList *cl;

  if (!tmp) {
    Rprintf("%s:assign_temp_to_sub(): WARNING: Called with NULL corpus\n", __FILE__);
    return NULL;
  }
  assert(tmp->type == TEMP);

  if (NULL != (cl = findcorpus(subname, SUB, 0))) {
    /* we found the target subc on our list! so we need to scrub it clean then overwrite. */
    initialize_cl(cl, True);

    /* as things go into the subc, they are removed from the tmp */
    cl->name = cl_strdup(subname);
    cl_free(tmp->name);

    cl->mother_name = tmp->mother_name;
    tmp->mother_name = NULL;
    cl->mother_size = tmp->mother_size;
    cl->registry = tmp->registry;
    tmp->registry = NULL;
    cl->range = tmp->range;
    tmp->range = NULL;
    cl->abs_fn = tmp->abs_fn;
    tmp->abs_fn = NULL;

    cl->keywords = tmp->keywords;
    tmp->keywords = NULL;
    cl->targets = tmp->targets;
    tmp->targets = NULL;
    cl->sortidx = tmp->sortidx;
    tmp->sortidx = NULL;

    cl->query_corpus = tmp->query_corpus;
    cl->query_text = tmp->query_text;
    tmp->query_corpus = NULL;
    tmp->query_text = NULL;

    cl->type = SUB;
    tmp->type = UNDEF;
    cl->saved = False;
    cl->loaded = True;
    cl->needs_update = True;

    cl->corpus = tmp->corpus;
    tmp->corpus = NULL;
    cl->size = tmp->size;
    tmp->size = 0;

    if (auto_save)
      save_subcorpus(cl, NULL);
    dropcorpus(tmp, NULL);

    return cl;
  }

  else {
    /* we only have to change some fields of tmp to TRANSFORM it to the subc we want */
    cl_free(tmp->name);
    tmp->name = cl_strdup(subname);
    tmp->type = SUB;
    tmp->needs_update = True;
    cl_free(tmp->abs_fn);

    if (auto_save)
      save_subcorpus(tmp, NULL);
    return tmp;
  }
}

/**
 * Delete temporary corpora, clearing the global list of corpora of all temporary stuff.
 */
void
drop_temp_corpora(void)
{
  CorpusList *curr = corpuslist, *prev = NULL;
  while (curr) {
    if (TEMP == curr->type)
      curr = prev = dropcorpus(curr, prev);
      /* prev now either points to the cl entry before the deleted one, or to the one after it (if curr
       * was the first entry in corpuslist), so continue the iteration from prev rather than prev->next.
       */
    else {
      prev = curr;
      curr = curr->next;
    }
  }
}


/**
 * Release all memory from the global corpus list.
 */
void
drop_all_corpora(void)
{
  CorpusList *curr, *next;

  current_corpus = NULL;

  for (curr = corpuslist ; curr ; curr = next) {
    next = curr->next;
    destruct_cl(curr);
  }
}



/**
 * Returns a newly allocated string representing a disk location for a saved subcorpus.
 * (within the current datadir).
 *
 * In this format:
 *
 *          /path/to/datadir/MOTHERCORPUS:Subcorpus
 *
 * @param cl                 Record of the subcorpus/NQR whose path we're building.
 * @param qualify   Boolean. If true, assume that the name DOES need to be prefixed with MOTHERCORPUS: .
 *                           If false, assume that the name can just be combined with the datadir path.
 * @return                   Newly allocated string, in the format described above
 */
static char *
get_fulllocalpath(CorpusList *cl, int qualify)
{
  char fullname[CL_MAX_FILENAME_LENGTH];
  char *upname;

  if (qualify) {
    upname = cl->mother_name ? cl_strdup(cl->mother_name) : cl_strdup("NONE");
    cl_id_toupper(upname);

    sprintf(fullname, "%s%s%s:%s", data_directory,
            data_directory[strlen(data_directory)-1] == SUBDIR_SEPARATOR ? "" : SUBDIR_SEP_STRING,
            cl->mother_name ? cl->mother_name : "NONE",
            cl->name);
    cl_free(upname);
  }
  else
    sprintf(fullname, "%s%s%s", data_directory,
            data_directory[strlen(data_directory)-1] == SUBDIR_SEPARATOR ? "" : SUBDIR_SEP_STRING,
            cl->name);

  return cl_strdup(fullname);
}



/**
 * Tests whether a file is accessible.
 *
 * A file is considered accessible
 * iff user can read it and it is not a (sub)directory.
 *
 * This test is used for registry entries.
 *
 * @param dir   Directory in which the file is to be found.
 * @param file  The filename to check.
 * @return      Boolean: true iff file is accessible.
 */
static Boolean
is_readable_file(char *dir, char *file)
{
  /* fullname is allocated: lenth of string dir and length of string file */
  /*                        plus 1 for the '\0' character                 */
  /*                        plus 1 for an additional '/'                  */
  Boolean success;
  struct stat filestat;
  char *fullname = (char *)cl_malloc(strlen(dir) + strlen(file) + 2);

  fullname[0] = '\0';
  strcat(fullname, dir);
  if (fullname[strlen(fullname)-1] != SUBDIR_SEPARATOR)
    strcat(fullname, SUBDIR_SEP_STRING);
  strcat(fullname, file);

  success = (0 == stat(fullname, &filestat) && !S_ISDIR(filestat.st_mode) && 0 == access(fullname, R_OK));

  cl_free(fullname);

  return(success);
}

/* NO LONGER USED: check_stamp() WAS PREVIOUSLY CALLED WHEN DataDirectory WAS SET
   Its previous purpose was to check the magic number of potential saved query files in the data directory,
   but this caused enormous delays when there were lots of files in this directory (e.g. in BNCweb).  So now
   every file whose name looks right will be inserted into the internal list, but accessing it will fail
   if it turns out to be bogus (which shouldn't happen anyway if the directory is handled by a sane person).
int
check_stamp(char *directory, char *fname)
{
  FILE *fd;
  char full_name[CL_MAX_FILENAME_LENGTH];
  int magic, ok;

  sprintf(full_name, "%s" SUBDIR_SEP_STRING "%s", directory, fname);

  if (((fd = cl_open_stream(full_name, "rb")) == NULL) ||
      (fread(&magic, sizeof(int), 1, fd) == 0) ||
      ((magic != CWB_SUBCORPMAGIC_ORIG) && (magic != CWB_SUBCORPMAGIC_ORIG+1)))
    ok = 0;
  else
    ok = 1;

  if (fd)
    cl_close_stream(fd);

  return ok;
}
*/


/**
 * Looks for loadable corpora on disk in the usual places
 * (so: registry - global or default - for system corpora;
 * data directory for subcorpora; etc.)
 */
static void
load_corpusnames(enum corpus_type ct)
{
  DIR           *dp;
  struct dirent *ep;
  char          *entry;

  char          dirlist[CL_MAX_LINE_LENGTH];
  char          *dirname;
  CorpusList    *corpus;

  if (SYSTEM == ct)
    strcpy(dirlist, registry ? registry : cl_standard_registry());
  else if (SUB == ct)
    strcpy(dirlist, data_directory);
  else {
    Rprintf("Can't load corpus names for type %d\n", ct);
    return;
  }

  /* clear out the corpus list by marking delenda as TEMP, then calling drop_temp_corpora() */
  for (corpus = corpuslist; corpus != NULL; corpus = corpus->next)
    if (corpus->type == ct && corpus->saved)
      corpus->type = TEMP;
  drop_temp_corpora();

  for (dirname = cl_path_get_component(dirlist) ; dirname ; dirname = cl_path_get_component(NULL)) {
    int optional_dir = 0;
    if (*dirname == '?') {
      /* optional registry directory -> don't issue warning if not mounted */
      dirname++;
      optional_dir = 1;
    }

    if (NULL != (dp = opendir(dirname))) {
      /* discard all (loaded) corpora of this type from list of available corpora. */
      while ((ep = readdir(dp))) {
        if (!strchr(ep->d_name, '.') &&   /* ignore files with '.' char in registry (such as hidden files) */
            !strchr(ep->d_name, '~') &&   /* ignore files with '~' char in registry (such as emacs backup files) */
            is_readable_file(dirname, ep->d_name)) {   /* ignore directories & files user can't access (hidden from user) */
          if (ct == SUB) {
            char *colon;

            /* The fucntion check_stamp used to be used here to check the format of saved-query files.
             * It can take quite long to check all files if data directory contains thousands of saved queries
               (as it often does in BNCweb, for instance), and it counteracts the purpose of delayed loading: */
            /*      if (check_stamp(dirname, ep->d_name)) { */
            /* (note that the magic number will be checked ANYWAY when the saved query is loaded with attach_subcorpus()) */

            /* saved query results should always be named <CORPUS>:<query name> */
            if (NULL != (colon = strchr(ep->d_name, COLON))) {

              /* Since there can be only a single data directory for saved query results, there should be no duplicates
                 and it is unnecessary to check with findcorpus().  It is this check that caused the greatest delays when
                 reading a data directory with thousands of files (because the internal list of corpus names has to be
                 searched linearly by findcorpus() for every file in the directory (-> quadratic complexity). */
              /*     corpus = findcorpus(ep->d_name, SUB, 0); */
              /*     if (corpus == NULL) { */
              /* (NB: one data directory constraint is implicit; loading might work, but save_subcorpus() will crash miserably) */

              char mother[CL_MAX_FILENAME_LENGTH]; /* Judith vs. Oli, round 231 */

              /* allocate memory for the new id */
              corpus = NewCL();

              /* fill the values of the new corpuslist element */
              cl_strcpy(mother, ep->d_name);
              mother[colon - ep->d_name] = '\0';
              corpus->mother_name = cl_strdup(mother);
              corpus->name = cl_strdup(colon+1);

              corpus->next = corpuslist;
              corpuslist = corpus;

              corpus->type = SUB;
              corpus->loaded = corpus->needs_update = False;
              corpus->saved = True;
              /* Delayed loading: We don't want to load ALL subcorpora in
                 the local corpus directory because that gets us into trouble
                 with some MP template suites. So we don't call attach_subcorpus()
                 now (should be done by ensure_corpus_size() later which is
                 called from findcorpus() etc.). In order to call it later,
                 we have to remember in which directory we found it. */
              corpus->local_dir = cl_strdup(dirname);
              /* attach_subcorpus(corpus, dirname, ep->d_name); */
            }
          }
          else {
            /* ct is not SUB */
            entry = cl_strdup(ep->d_name);
            cl_id_toupper(entry);

            if (NULL == (corpus = LoadedCorpus(entry, NULL, SYSTEM))) {
              corpus = GetSystemCorpus(entry, dirname);
              if (corpus) {
                corpus->next = corpuslist;
                corpuslist = corpus;
              }
            }
            cl_free(entry);
          }
        }
      }
      closedir(dp);
    }
    else if (!silent && !optional_dir)
      cqpmessage(Warning, "Couldn't open directory %s (continuing)", dirname);
  }
  /* end for (each directory) */
}


/** Check what corpora of a given type are available on disk. */
void
check_available_corpora(CorpusType ct)
{
  if (ct == UNDEF) {
    load_corpusnames(SYSTEM);
    if (data_directory)
      load_corpusnames(SUB);
  }
  else if (ct != TEMP)
    load_corpusnames(ct);

  /* due to list being fiddled with, current corpus is no longer valid -> reset it */
  set_current_corpus(NULL, 0);
}


/**
 * Creates a CorpusList object for a named system corpus in a specific registry directory.
 *
 * @return  Either a pointer to the CorpusList OR NULL if the corpus could not be loaded.
 */
static CorpusList *
GetSystemCorpus(char *name, char *registry)
{
  Corpus *this_corpus;
  CorpusList *cl;
  char *cname;

  cname = cl_strdup(name);
  cl_id_tolower(cname);
  this_corpus = cl_new_corpus(registry, cname);
  cl_free(cname);

  if (this_corpus) {
    /* to speed up CQP startup, don't try to read the size of each corpus
       that is inserted into the corpus list */
    /* done by:  evert (Thu Apr 30 13:51:45 MET DST 1998) */

    cl = NewCL();
    cl->name = cl_strdup(name);
    cl->mother_name = cl_strdup(name);
    cl->mother_size = 0; /* this will tell us later that we haven't checked corpus size / corpus access yet */

    if (this_corpus->registry_dir)
      cl->registry = cl_strdup(this_corpus->registry_dir);
    else if (registry)
      cl->registry = cl_strdup(registry);
    else {
      Rprintf("Warning: no registry directory for %s\n", name);
      cl->registry = NULL;
    }

    cl->type = SYSTEM;
    cl->abs_fn = NULL;
    cl->saved = True;
    cl->loaded = True;
    cl->needs_update = False;
    cl->corpus = this_corpus;
    cl->size = 1;

    cl->range = (Range *)cl_malloc(sizeof(Range));
    /* the range of a system corpus is <0, attr_size-1> ;
       note that this is [0, -1] on init and must be changed when we
       determine the actual corpus size later on */
    cl->range[0].start = 0;
    cl->range[0].end   = -1;

    cl->sortidx = NULL;
    cl->targets = NULL;
    cl->keywords = NULL;
    cl->next = NULL;
    return cl;
  }

  return NULL;
}



static CorpusList *
ensure_syscorpus(char *registry, char *name)
{
  CorpusList *cl;

  if (!(cl = LoadedCorpus(name, registry, SYSTEM))) {
    /* the system corpus is not yet loaded. Try to get it. */
    /* (of course this shouldn't happen anyway since CQP reads in all
       available system corpora at startup) */
    if (!(cl = GetSystemCorpus(name, registry)))
      return NULL;
    else {
      cl->next = corpuslist;
      corpuslist = cl;
    }
  }

  /* now <cl> contains a valid corpus handle */
  /* If the size of the system corpus hasn't been determined yet,
     do it now. If we can't return failure. */
  if (!ensure_corpus_size(cl))
    return NULL;

  /* At this point, the corpus *cl has been fully set up. */
  return cl;
}


/**
 * Reads in a sbcorpus/NQR that was previously saved to disk.
 *
 * @param cl                    Corpus we're currently working in.
 * @param advertised_directory
 * @param advertised_filename
 * @return                      Boolean: whether the file was loaded correctly.
 */
static Boolean
attach_subcorpus(CorpusList *cl, char *advertised_directory, char *advertised_filename)
{
  int         len, magic;
  char       *fullname;
  FILE       *src;

  char       *field;
  char       *p;

  Boolean     load_ok = False;
  CorpusList *mother;

  /* if it's a subcorpus or temp corpus, work out the fullname (either local or using the specified paths) */
  if ( cl && (cl->type == SUB || cl->type == TEMP) ) {
    initialize_cl(cl, False);

    if (advertised_directory && advertised_filename) {
      char buffer[CL_MAX_FILENAME_LENGTH];
      cl_strcpy(buffer, advertised_directory);
      if (buffer[strlen(buffer)-1] != SUBDIR_SEPARATOR)
        strcat(buffer, SUBDIR_SEP_STRING);
      strcat(buffer, advertised_filename);

      fullname = cl_strdup(buffer);
    }
    else
      fullname = get_fulllocalpath(cl, 0);

    if (!(src = cl_open_stream(fullname, CL_STREAM_READ_BIN, CL_STREAM_MAGIC)) && !advertised_filename) {
      cl_free(fullname);
      fullname = get_fulllocalpath(cl, 1);
      src = cl_open_stream(fullname, CL_STREAM_READ_BIN, CL_STREAM_MAGIC);
    }

    if (!src)
      Rprintf("Subcorpus %s not accessible (can't open %s for reading)\n", cl->name, fullname);
    else {
      if (0 >= (len = file_length(fullname)))
        Rprintf("ERROR: File length of subcorpus is <= 0\n");
      else {
        /* the subcorpus is treated as a byte array */
        field = (char *)cl_malloc(len);

        /* read the subcorpus */
        if (len != fread(field, 1, len, src))
          Rprintf("Read error while reading subcorpus %s\n", cl->name);
        else if ( CWB_SUBCORPMAGIC_ORIG > (magic = *((int *)field)) || CWB_SUBCORPMAGIC_ORIG+1 < magic )
          Rprintf("Magic number incorrect in %s\n", fullname);
        else {
          p = field + sizeof(int);
          cl->registry = cl_strdup(p);

          cl->abs_fn = fullname;
          cl->abs_fn = fullname;
          fullname = NULL;

          /* advance p to the end of the 1st string (path of mother's registry); then skip the '\0' character */
          while (*p)
            p++;
          p++;

          cl->mother_name = cl_strdup(p);
          mother = ensure_syscorpus(cl->registry, cl->mother_name);

          if ( ! (mother && mother->corpus) )
            cqpmessage(Warning, "When trying to load subcorpus %s:\n\tCan't access mother corpus %s", cl->name, cl->mother_name);
          else {
            cl->corpus = mother->corpus;
            cl->mother_size = mother->mother_size;
            assert(cl->mother_size > 0);

            /* advance p to the end of the 2nd string (name of mother); then skip the '\0' character */
            while (*p)
              p++;
            p++;

            /* this bit of the file is padded to a multiple of 4 bytes -- advance p over the additional '\0' characters */
            while ((p - field) % 4)
              p++;

            if (magic == CWB_SUBCORPMAGIC_ORIG) {
              /* original file format */
              cl->size = (len - (p - field)) / (2 * sizeof(int));

              /* the first integer of the first integer tuple starts at the current offset */
              cl->range = (Range *)cl_malloc(sizeof(Range) * cl->size);
              memcpy(cl->range, p, sizeof(Range) * cl->size);
              cl->sortidx  = NULL;
              cl->keywords = NULL;
              cl->targets  = NULL;
            }
            else if (magic == CWB_SUBCORPMAGIC_NEW) {
              /* newer file format */
              int compsize; /* place to store the size of each component as loaded from the file */
              memcpy(&(cl->size), p, sizeof(int));
              p += sizeof(int);

              if (cl->size > 0) {
                /* main range-interval data is the same as in the old format */
                cl->range = (Range *)cl_malloc(sizeof(Range) * cl->size);
                memcpy(cl->range, p, sizeof(Range) * cl->size);
                p += sizeof(Range) * cl->size;

                /* each of the blocks of ints that follow is preceded by an int telling us how big that block is */

                memcpy(&compsize, p, sizeof(int));
                p += sizeof(int);
                if (compsize > 0) {
                  cl->sortidx = (int *)cl_malloc(sizeof(int) * cl->size);
                  memcpy(cl->sortidx, p, sizeof(int) * cl->size);
                  p += sizeof(int) * cl->size;
                }

                memcpy(&compsize, p, sizeof(int));
                p += sizeof(int);

                if (compsize > 0) {
                  cl->targets = (int *)cl_malloc(sizeof(int) * cl->size);
                  memcpy(cl->targets, p, sizeof(int) * cl->size);
                  p += sizeof(int) * cl->size;
                }

                memcpy(&compsize, p, sizeof(int));
                p += sizeof(int);
                if (compsize > 0) {
                  cl->keywords = (int *)cl_malloc(sizeof(int) * cl->size);
                  memcpy(cl->keywords, p, sizeof(int) * cl->size);
                  p += sizeof(int) * cl->size;
                }
              }
            }

            /* the rest is all the same for both formats */
#if subcorpload_debug
            {
              int j;
              Rprintf(
                      "Header size: %ld\n"
                      "Nr Matches: %d\n"
                      "regdir: %s\n"
                      "regname: %s\n",
                      (long int)(p - field),
                      cl->size,
                      cl->registry,
                      cl->mother_name);
              for (j = 0; j < cl->size; j++)
                Rprintf(
                        "range[%d].start = %d\n"
                        "range[%d].end   = %d\n",
                        j, cl->range[j].start, j, cl->range[j].end);
            }
#endif
            cl_free(field);
            cl->type = SUB;
            cl->saved = True;
            cl->loaded = True;
            cl->needs_update = False;

            load_ok = True;
          }
        }
        cl_close_stream(src);
      }
    }
    cl_free(fullname);
  }

  if (!load_ok)
    dropcorpus(cl, NULL);

  return load_ok;
}


/**
 * Save a subcorpus/NQR to disk.
 *
 * @param cl
 * @param fname     String, filename.
 * @return
 */
Boolean
save_subcorpus(CorpusList *cl, char *fname)
{
  int i, l1, l2, magic;

  FILE *dst;
  char fn_buf[CL_MAX_FILENAME_LENGTH];

  if (!cl)
    return False;
  else if (!cl->loaded)
    return False;
  else if (cl->type != SUB)
    return False;
  else if ( !cl->needs_update  || cl->saved )
    return True;
    /* save is ok, although we did not do anything */

  else {
    /* if no filename supplied, get it from the CorpusList object */
    if (fname == NULL) {
      if (cl->abs_fn)
        fname = cl->abs_fn;
      else {
        fname = fn_buf;
        if (!data_directory) {
          cqpmessage(Warning, "Directory for private subcorpora isn't set, can't save %s", cl->name);
          return False;
        }
        sprintf(fname, "%s%c%s:%s",
                data_directory,
                SUBDIR_SEPARATOR,
                cl->mother_name ? cl->mother_name : "NONE",
                cl->name);
      }
    }

    if (NULL != (dst = cl_open_stream(fname, CL_STREAM_WRITE_BIN, CL_STREAM_MAGIC))) {
      int zero = 0;
      magic = CWB_SUBCORPMAGIC_NEW;

      fwrite(&magic, sizeof(int), 1, dst);

      l1 = fwrite(cl->registry, 1, strlen(cl->registry) + 1, dst);
      l2 = fwrite(cl->mother_name, 1, strlen(cl->mother_name) + 1, dst);

      /* fill up */
      for (i = 0; (i+l1+l2) % 4 != 0; i++)
        fwrite(&zero, sizeof(char), 1, dst);

      /* write the size (the number of ranges) */
      fwrite(&cl->size, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */

      if (cl->size > 0) {
        fwrite((char *)cl->range, sizeof(Range), cl->size, dst);

        /* write the sort index  new Mon Jul 31 17:24:52 1995 (oli) */
        if (cl->sortidx) {
          fwrite(&cl->size, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */
          fwrite((char *)cl->sortidx, sizeof(int), cl->size, dst);
        }
        else
          fwrite(&zero, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */

        /* write the targets new Mon Jul 31 17:24:59 1995 (oli) */
        if (cl->targets) {
          fwrite(&cl->size, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */
          fwrite((char *)cl->targets, sizeof(int), cl->size, dst);
        }
        else
          fwrite(&zero, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */

        /* write the keywords new Mon Jul 31 17:25:02 1995 (oli) */
        if (cl->keywords) {
          fwrite(&cl->size, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */
          fwrite((char *)cl->keywords, sizeof(int), cl->size, dst);
        }
        else
          fwrite(&zero, sizeof(int), 1, dst); /* new Mon Jul 31 17:24:47 1995 (oli) */
      }

      cl_close_stream(dst);
      cl->saved = True;
      cl->needs_update = False;

      return True;
    }
    else {
      Rprintf("cannot open output file %s\n", fname);
      return False;
    }
  }
}

void
save_unsaved_subcorpora(void)
{
  CorpusList *cl;
  for (cl = corpuslist; cl; cl = cl->next)
    if (cl->type == SUB && !cl->saved) {
      if (!data_directory) {
        cqpmessage(Warning, "Can't save unsaved subcorpora, directory is not set");
        return;
      }
      save_subcorpus(cl, NULL);
    }
}



/**
 * Gets the CorpusList pointer for the first corpus on the currently-loaded list.
 * Function for iterating through the list of currently-loaded corpora.
 *
 * @return  The requested CorpusList pointer.
 */
CorpusList *
FirstCorpusFromList()
{
  return corpuslist;
}

/**
 * Gets the CorpusList pointer for the next corpus on the currently-loaded list.
 * Function for iterating through the list of currently-loaded corpora.
 *
 * @param cl  The current corpus on the list.
 * @return    The requested CorpusList pointer.
 */
CorpusList *
NextCorpusFromList(CorpusList *cl)
{
  return (cl ? cl->next : NULL);
}


/**
 * Assesses whether a specified corpus can be accessed.
 *
 * That is, it makes sure that the data for corpus in
 * "cl" is loaded and accessible.
 *
 * @param cl  A CorpusList specifying the corpus/NQR to check.
 * @return    A boolean - true if cl can be accessed.
 */
Boolean
access_corpus(CorpusList *cl)
{
  if (cl) {
    if (cl->loaded) {
      /* do we have range data? */
      assert(cl->size == 0 || cl->range);
      /* we do! therefore, it is already loaded, do nothing */
      return True;
    }
    else if (cl->saved) {
      /* it's on disk, and needs to be loaded */
      switch(cl->type) {
      case SUB:
      case TEMP:
        return attach_subcorpus(cl, NULL, NULL);

      case SYSTEM:
        assert(0);  // TODO eh?
        return True;

      default:
        break;
     }
    }
  }
  return False;
}


/**
 * Checks whether corpusname is syntactically valid as a query result name, i.e., either
 *
 *     identifier
 * or
 *     qualifier.identifier
  */
Boolean
valid_subcorpus_name(char *corpusname)
{
  return NULL != split_subcorpus_name(corpusname, NULL);
}


/** Checks whether corpusname is qualified (with name of mother corpus); does not imply syntatic validity */
Boolean
subcorpus_name_is_qualified(const char *corpusname)
{
  return NULL == strchr(corpusname, COLON) ? 0 : 1;
}


/**
 * Splits a query result "corpus"-name into qualifier and local name.
 *
 * This function splits query result name {corpusname} into qualifier (name of mother corpus) and local name;
 * returns pointer to local name part, or NULL if {corpusname} is not syntactically valid.
 *
 * if mother_name is not NULL, it must point to a buffer of suitable length (CL_MAX_LINE_LENGTH is sufficient)
 * where the qualifier will be stored (empty string for unqualified corpus, and return value == {corpusname} in this case)
 *
 * @param corpusname    "Corpus" name to process. This is not really a corpus but a query result or
 *                      subcorpus, in the form CORPUS:TheQuery. It won't be modified.
 * @param mother_name   Buffer for a copy of the qualifer (first part of name, aka the mother, aka the corpus).
 *                      When using split_subcorpus_name as a syntax validator for possibly qualified names,
 *                      this would normally be NULL.
 * @return              Pointer into the corpusname buffer, at the point where the second part
 *                      (the subcorpus/query name) begins.
 *                      NULL is returned if, at any point in corpusname, there is a character that
 *                      should not be there.
 */
char *
split_subcorpus_name(const char *corpusname, char *mother_name)
{
  const char *mark;
  int i, after_colon;

  /* valid characters in names are _ . - and alphanumeric. After the start, also ^ */
  if ( ! (isalnum(corpusname[0])  || corpusname[0] == '_' || corpusname[0] == '-') )
    return NULL;

  mark = corpusname;
  if (mother_name)
    mother_name[0] = '\0';
  after_colon = 0;

  /* character 0 already checked, so start at 1 */
  for (i = 1; corpusname[i]; i++) {
    if (COLON == corpusname[i]) {
      if (after_colon)          /* only one ':' separator is allowed */
        return NULL;
      if (mother_name) {        /* characters 0 .. i-1 give name of mother corpus */
        strncpy(mother_name, corpusname, i);
        mother_name[i] = '\0';
      }
      mark = corpusname + (i+1); /* local name starts from character i+1 */
      after_colon = 1;
    }
    else if (  !(  isalnum(corpusname[i])
                || corpusname[i] == '_'
                || corpusname[i] == '-'
                || corpusname[i] == '^' /* should also check that there is only one implicit expansion at the end */
                ) )
      return NULL;
  }

  return (char *)mark;                  /* return pointer to local name */
}


/**
 * Touches a corpus, ie, marks it as changed.
 *
 * @param cp  The corpus to touch. This must be of type SUB.
 * @return    Boolean: true if the touch worked, otherwise false.
 */
int
touch_corpus(CorpusList *cp)
{
  if (!cp || cp->type != SUB)
    return 0;
  else {
    cp->saved = 0;
    cp->needs_update = 1;
    return 1;
  }
}


/**
 * Sets the current corpus (by pointer to the corpus).
 *
 * @param cp     Pointer to the corpus to set as current.
 *               cp may be NULL, which is legal.
 * @param force  If true, the current corpus is set to the specified
 *               corpus, even if it is ALREADY set to that corpus.
 * @return       Always 1.
 */
int
set_current_corpus(CorpusList *cp, int force)
{
  if (cp != current_corpus || force) {
    current_corpus = cp;

    if (cp) {
      AttributeInfo *ai;
      int nr_selected_attributes = 0;

      update_context_descriptor(cp->corpus, &CD);

      /* if no p-attributes are selected for output, try to switch on the default attribute */
      for (ai = CD.attributes->list; ai; ai = ai->next)
        if (ai->status > 0)
          nr_selected_attributes++;
      if (nr_selected_attributes == 0)
        if (NULL != (ai = FindInAL(CD.attributes, CWB_DEFAULT_ATT_NAME)))
          ai->status = 1;
    }
    else {
      DestroyAttributeList(&(CD.attributes));
      DestroyAttributeList(&(CD.strucAttributes));
    }
  }
  return 1;
}


/**
 * Sets the current corpus (by name).
 *
 * @param name   Name of the corpus to set as current.
 * @param force  If true, the current corpus is set to the specified
 *               corpus, even if it is ALREADY set to that corpus.
 * @return       True if the corpus was found and set, otherwise
 *               false if the corpus could not be found.
 */
int
set_current_corpus_name(char *name, int force)
{
  CorpusList *cp;

  if (!(cp = findcorpus(name, UNDEF, 1)))
    return 0;
  else
    return set_current_corpus(cp, force);
}


/**
 * Internal function for sorting list of corpus names.
 *
 * @see show_corpora
 */
static int
show_corpora_sort(const void *p1, const void *p2)
{
  char *name1 = *((char **) p1);
  char *name2 = *((char **) p2);
  int result = strcmp(name1, name2);
  return result;
}


/**
 * Function that does the work for show_corpora_files.
 *
 * @see show_corpora_files
 * @see show_corpus_active
 * @param ct                  Which type of corpus to show. Must be SUB or SYSTEM.
 * @param only_active_corpus  If true, then only the active corpus's info is printed.
 *                            If a subcorpus is active then CORPUS:SUBCORPUS is printed.
 *                            The ct argument is, in that case, ignored.
 */
static void
show_corpora_backend(CorpusType ct, int only_active_corpus)
{
  CorpusList *cl;
  char label[4];
  char **list;                  /* list of corpus names (for qsort) */
  int i, N;
  char initial = ' ';           /* inital character of corpus name
                                 * (for use in case of indented list;
                                 * insert <br> before new initial letter) */

  /* if we are only to show the active, then we need to use the type of the actual active cl_list. */
  if (only_active_corpus) {
    if (current_corpus)
      ct = current_corpus->type;
    else {
      Rprintf("\n");
      return;
    }
  }

  switch (ct) {

  case SYSTEM:
    if (only_active_corpus) {
      /* show either the name or mother name of the curr corpus */
      if (!current_corpus)
        return;
      N = 1;
      list = (char **)cl_malloc(sizeof(char *));
      if (current_corpus->type == SYSTEM)
        list[0] = current_corpus->name;
      else
        list[0] = current_corpus->mother_name;
    }
    else {
      /* make list of corpus names, then qsort() and print */
      /* count nr of system corpora before allocating list */
      for (cl = corpuslist, N = 0; cl; cl = cl->next)
        if (cl->type == SYSTEM)
          N++;
      list = (char **)cl_malloc(N * sizeof(char *)); /* allocate list */
      for (cl = corpuslist, i = 0; cl; cl = cl->next) /* compile list of corpus names */
        if (cl->type == SYSTEM)
          list[i++] = cl->name;
      qsort(list, N, sizeof(char *), show_corpora_sort);
    }

    if (pretty_print) {
      Rprintf(only_active_corpus ? "Active Corpus:\n" : "System corpora:\n");
      ilist_start(0,0,0);
    }

    /* now print sorted list */
    for (i = 0; i < N; i++) {
      if (pretty_print) {
        if (list[i][0] != initial) {
          initial = list[i][0];
          sprintf(label, " %c:", initial);
          ilist_print_break(label);
        }
        ilist_print_item(list[i]);
      }
      else
        Rprintf("%s\n", list[i]);
    }

    if (pretty_print)
      ilist_end();

    cl_free(list);
    break;

  case SUB:
    if (pretty_print)
      Rprintf(only_active_corpus ? "Active Subcorpus:\n" : "Named Query Results:\n");

    for (cl = corpuslist; cl; cl = cl->next) {
      if (only_active_corpus) {
        if (cl != current_corpus)
          continue;
        else if (!pretty_print) {
          Rprintf("%s:%s\n", cl->mother_name ? cl->mother_name : "???", cl->name);
          continue;
        }
        /* else: continue into the prettyprint branch below */
      }
      if (cl->type == SUB)
        Rprintf(pretty_print ? "   %c%c%c  %s:%s [%d]\n" : "%c%c%c\t%s:%s\t%d\n",
               cl->loaded       ? 'm' : '-',
               cl->saved        ? 'd' : '-',
               cl->needs_update ? '*' : '-',
               cl->mother_name  ? cl->mother_name : "???",
               cl->name,
               cl->size
               );
    }
    break;

  default:
    assert(0 && "Invalid argument in show_corpora_files()<corpmanag.c>.");
  }
}


/**
 * A function to print out a list of corpora currently available.
 *
 * Either system corpora (SYSTEM) or subcorpora (SUB) can be shown, depending on ct.
 * If type is UNDEF, both are shown.
 *
 * For subcorpora, a bundle of other information is shown too.
 *
 * @param type  Type of corpus to show (SUB, SYSTEM or UNDEF).
 */
void
show_corpora(CorpusType type)
{
  if (type == UNDEF) {
    show_corpora_backend(SYSTEM, 0);
    show_corpora_backend(SUB   , 0);
  }
  else
    show_corpora_backend(type  , 0);
}


/**
 * Displays the ID of the active corpus.
 * (Including any activated subcorpus/NQR.)
 * (Added for child mode mostly.)
 */
void
show_corpus_active(void)
{
  show_corpora_backend(ALL, 1);
}


