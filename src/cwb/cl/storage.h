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

#ifndef _cl_storage_h_
#define _cl_storage_h_

#include <sys/types.h>
#include <stddef.h>

#ifdef __MINGW__
#include "windows-mmap.h"
#endif

/* data allocation methods for MemBlobs */
#define CL_MEMBLOB_UNALLOCATED 0 /**< Flag: indicates no memory has been allocated */
#define CL_MEMBLOB_MMAPPED  1    /**< Flag: indicates use of mmap() to allocate memory in a MemBlob */
#define CL_MEMBLOB_MALLOCED 2    /**< Flag: indicates use of malloc() to allocate memory in a MemBlob  */


/* macros defining the size of individual items within a MemBlob. */
#define SIZE_BIT   0   /* symbolic for "one-eighth" */
// the others aren't used anywhere
//#define SIZE_BYTE  sizeof(char)
//#define SIZE_SHINT sizeof(short)
//#define SIZE_INT   sizeof(int)
//#define SIZE_LONG  sizeof(long)


/**
 * The MemBlob object.
 *
 * This object, unsurprisingly, represents a blob of memory.
 */
typedef struct TMblob {
  size_t size;                  /**< the number of allocated bytes */

  int item_size;                /**< the size of one item */
  unsigned int nr_items;        /**< the number of items represented */

  int *data;                    /**< pointer to the data */
  int allocation_method;        /**< the allocation method */

  int writeable;                /**< can we write to the data? */
  int changed;                  /**< needs update? (not yet in use) */

  /* fields for paged memory -- not yet used */
  char *fname;
  off_t fsize, offset;
} MemBlob;



void NwriteInt(int val, FILE *fd);
void NreadInt(int *val, FILE *fd);

void NwriteInts(int *vals, int nr_vals, FILE *fd);
void NreadInts(int *vals, int nr_vals, FILE *fd);


void free_mblob(MemBlob *blob);
void init_mblob(MemBlob *blob);
int alloc_mblob(MemBlob *blob, int nr_items, int item_size, int clear_blob);


/* ================================================================ FILE IO */

int read_file_into_blob(char *filename,
                        int allocation_method,
                        int item_size,
                        MemBlob *blob);
int write_file_from_blob(char *filename,
                         MemBlob *blob,
                         int convert_to_nbo);

/* ==================================================== LOW LEVEL FUNCTIONS */

void *mmapfile(char *filename, size_t *len_ptr, char *mode);
void *mallocfile(char *filename, size_t *len_ptr, char *mode);


#endif
