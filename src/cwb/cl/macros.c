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


void Rprintf(const char *, ...);
#include "globals.h"

#include <time.h>


/**
 * @file
 *
 * This file contains: memory  management (better malloc, free, etc.); and random number generator
 */





/*
 * memory allocation functions with integrated success test
 * (to be used as hooks for the CL MMU)
 */

/**
 * Safely allocates memory malloc-style.
 *
 * This function allocates a block of memory of the requested size,
 * and does a test for malloc() failure which aborts the program and
 * prints an error message if the system is out of memory.
 * So the return value of this function can be used without further
 * testing for malloc() failure.
 *
 * @param bytes  Number of bytes to allocate
 * @return       Pointer to the block of allocated memory
 */
void *
cl_malloc(size_t bytes)
{
  void *block;

  if (NULL == (block = malloc(bytes))) {
    Rprintf("CL: Out of memory. (killed)\n");
    Rprintf("CL: [cl_malloc(%ld)]\n", bytes);
    Rprintf("\n");		/* for CQP's child mode */
    exit(1);
  }
  return block;
}

/**
 * Safely allocates memory calloc-style.
 *
 * @see cl_malloc
 * @param nr_of_elements  Number of elements to allocate
 * @param element_size    Size of each element
 * @return                Pointer to the block of allocated memory
 */
void *
cl_calloc(size_t nr_of_elements, size_t element_size)
{
  void *block;

  if (!(block = calloc(nr_of_elements, element_size))) {
    Rprintf("CL: Out of memory. (killed)\n");
    Rprintf("CL: [cl_calloc(%ld*%ld bytes)]\n", nr_of_elements, element_size);
    Rprintf("\n");		/* for CQP's child mode */
    exit(1);
  }
  return block;
}

/**
 * Safely reallocates memory.
 *
 * @see cl_malloc
 * @param block  Pointer to the block to be reallocated
 * @param bytes  Number of bytes to allocate to the resized memory block
 * @return       Pointer to the block of reallocated memory
 */
void *
cl_realloc(void *block, size_t bytes)
{
  void *new_block;

  if (!block)
    new_block = malloc(bytes);	/* some OSs don't fall back to malloc() if block == NULL */
  else
    new_block = realloc(block, bytes);

  if (!new_block) {
    /* only warn if more than 0 bytes were requested. If we got NULL from submitting 0 to m/realloc, no problem. */
    if (bytes != 0) {
      Rprintf("CL: Out of memory. (killed)\n");
      Rprintf("CL: [cl_realloc(block at %p to %ld bytes)]\n", block, bytes);
      Rprintf("\n");		/* for CQP's child mode */
      exit(1);
    }
  }
  return new_block;
}

/**
 * Safely duplicates a string.
 *
 * @see cl_malloc
 * @param string  Pointer to the original string
 * @return        Pointer to the newly duplicated string
 */
char *
cl_strdup(const char *string)
{
  char *new_string;

  if (!(new_string = strdup(string))) {
    Rprintf("CL: Out of memory. (killed)\n");
    Rprintf("CL: [cl_strdup(addr=%p, len=%ld)]\n", string, strlen(string));
    Rprintf("\n");		/* for CQP's child mode */
    exit(1);
  }
  return new_string;
}


/**
 * Safely duplicates string [str] up to (but not including) the beginning of [end]
 * (where [end] is a substr of [str]) just as if there were a \0 at [end].
 *
 * @return  Newly allocated string; caller must free.
 */
char *
cl_strdup_to(char *str, char *end)
{
  int len = end - str;
  char *result, *p;

  result = (char *)cl_malloc(len+1);
  for (p = result; len > 0; len--)
    *p++ = *str++;
  *p = '\0';
  return result;
}




/*
 * built-in random number generator (avoid dependence on quality of system's rand() function)
 *
 * this random number generator is a version of Marsaglia-multicarry which is one of the RNGs used by R
 */

static unsigned int RNG_I1=1234, RNG_I2=5678;

/**
 * Restores the state of the CL-internal random number generator.
 *
 * @param i1  The value to set the first RNG integer to (if zero, resets it to 1)
 * @param i2  The value to set the second RNG integer to (if zero, resets it to 1)
 */
void
cl_set_rng_state(unsigned int i1, unsigned int i2)
{
  RNG_I1 = (i1) ? i1 : 1; 	/* avoid zero values as seeds */
  RNG_I2 = (i2) ? i2 : 1;
}


/**
 * Reads current state of CL-internal random number generator.
 *
 * The (unsigned, 32-bit) integers currently held in RNG_I1 and RNG_I2
 * are written to the two memory locations supplied as arguments.
 *
 * @param i1  Target location for the value of RNG_I1
 * @param i2  Target location for the value of RNG_I2
 */
void
cl_get_rng_state(unsigned int *i1, unsigned int *i2)
{
  *i1 = RNG_I1;
  *i2 = RNG_I2;
}

/**
 * Initialises the CL-internal random number generator.
 *
 * @param seed  A single 32bit number to use as the seed
 */
void
cl_set_seed(unsigned int seed)
{
  cl_set_rng_state(seed, 69069 * seed + 1); /* this is the way that R does it */
}

/**
 *  Initialises the CL-internal random number generator from the current system time.
 */
void
cl_randomize(void)
{
  cl_set_seed(time(NULL));
}

/**
 * Gets a random 32 bit integer.
 *
 * Part of the CL-internal random number generator.
 *
 * @return  The random number, an unsigned 32-bit integer with uniform distribution
 */
unsigned int
cl_random(void)
{
  RNG_I1 = 36969*(RNG_I1 & 0177777) + (RNG_I1 >> 16);
  RNG_I2 = 18000*(RNG_I2 & 0177777) + (RNG_I2 >> 16);
  return((RNG_I1 << 16) ^ (RNG_I2 & 0177777));
}

/**
 * Gets a random floating-point number in the range [0,1] with uniform distribution.
 *
 * Part of the CL-internal random number generator.
 *
 * @return  The generated random number.
 */
double
cl_random_fraction(void)
{
  return cl_random() * 2.328306437080797e-10; /* = cl_random / (2^32 - 1) */
}







