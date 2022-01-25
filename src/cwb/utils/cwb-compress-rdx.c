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

#include <math.h>

#include "../cl/cl.h"
#include "../cl/cwb-globals.h"
#include "../cl/corpus.h"
#include "../cl/attributes.h"
#include "../cl/storage.h"
#include "../cl/bitio.h"
#include "../cl/compression.h"

/* ---------------------------------------------------------------------- */

/** Name of the program */
/* char *progname = NULL; */

/** CWB id of the corpus we are working on */
/* char *corpus_id = NULL; */

/** Record for the corpus we are working on */
extern Corpus *corpus;

void compressrdx_usage(char *msg, int error_code);
void compressrdx_cleanup(int error_code);

/** debug level */
/* extern int debug = 0; */

/** where debug messages are to be sent to (stderr) */
/* FILE *debug_output; */ /* " = stderr;" init moved to main() for Gnuwin32 compatibility */

/** stores current position in a bit-write-file */
int codepos = 0;

#if 0

/* ------------- THIS VARIANT OF THE COMPRESSION CODE NOT USED !! ------- */

/* ALISTAIR MOFFAT'S COMMENTS WORKED IN ------------------------------ */

void write_golomb_code_am(int x, int b, BFile *bf)
{
  int q, res, lb, ub, nr_sc, nr_lc;
  int r, lr;

  int i;
  double ldb;

  unsigned char bit1 = '\1';
  unsigned char bit0 = '\0';

  q = x / b;
  res = x - q * b;

  ldb = log2(b * 1.0);

  ub = nint(ceil(ldb));
  lb = ub - 1;

  /* write the unary part q */

  for (i = 0; i < q; i++)
    BFwrite(bit1, 1, bf);
  BFwrite(bit0, 1, bf);


  /* write the binary part */

  nr_sc = (1 << ub) - b;

  if (debug)
    Rprintf(" res=%5d CL [%3d/%3d] #sc %4d "
            "writing %5d/%d\n",
            res, lb, ub, nr_sc,
            (res < nr_sc) ? res : res + nr_sc,
            (res < nr_sc) ? lb : ub);

  if (res < nr_sc) {
    BFwriteWord((unsigned int)res, lb, bf);
  }
  else {
    BFwriteWord((unsigned int)(res + nr_sc), ub, bf);
    if (res + nr_sc >= (1 << ub))
      Rprintf("Warning: can't encode %d in %d bits\n",
              res + nr_sc, ub);
  }

}

int read_golomb_code_am(int b, BFile *bf)
{
  int q, i, nr_sc, lb, ub;

  unsigned int r;
  unsigned char bit;

  double ldb;

  ldb = log2(b * 1.0);
  ub = nint(ceil(ldb));
  lb = ub - 1;

  /* read unary part */

  q = 0;
  do {
    BFread(&bit, 1, bf);
    if (bit)
      q++;
  } while (bit);

  nr_sc = (1 << ub) - b;

  /* read binary part, bitwise */

  r = 0;
  for (i = 0; i < lb; i++) {
    r <<= 1;
    BFread(&bit, 1, bf);
    r |= bit;
  }

  if (debug)
    Rprintf("%8d:  Read r=%5d [%3d/%3d]  #sc=%4d, ",
            codepos, r, lb, ub, nr_sc);

  if (r >= nr_sc) {
    r <<= 1;
    BFread(&bit, 1, bf);
    r |= bit;
    r -= nr_sc;
  }

  if (debug)
    Rprintf("final r=%d\tgap=%d\n",
            r, r+q*b);

  return r + q * b;
}

#endif

/* -------------- END OF UNUSED CODE ------------------------------------ */



/* ================================================== COMPRESSION */

/**
 * Compresses the reversed index of a p-attribute.
 *
 * @param attr      The attribute to compress the index of.
 * @param output_fn Base name for the compressed RDX files to be written
 *                  (if this is null, filenames will be taken from the
 *                  attribute).
 */
void
compress_reversed_index(Attribute *attr, char *output_fn, char *corpus_id, int debug)
{
  char *s;
  char data_fname[CL_MAX_FILENAME_LENGTH];
  char index_fname[CL_MAX_FILENAME_LENGTH];

  int nr_elements;
  int element_freq;
  int corpus_size;
  int last_pos, gap, fpos;

  int b;
  int i, k;

  BFile data_file;
  FILE *index_file = NULL;

  ClPositionStream PStream;
  int new_pos;


  Rprintf("COMPRESSING INDEX of %s.%s\n", corpus_id, attr->any.name);

  /* ensure that we do NOT use the compressed index while building the
   * compressed index (yeah, a nasty thing that). That is, load the
   * .corpus.rev and .corpus.rdx components in order to force
   * subsequent CL calls to use the uncompressed data.
   */
  {
    Component *comp;

    if ((comp = ensure_component(attr, CompRevCorpus, 0)) == NULL) {
      Rprintf("Index compression requires the REVCORP component\n");
      compressrdx_cleanup(1);
    }

    if ((comp = ensure_component(attr, CompRevCorpusIdx, 0)) == NULL) {
      Rprintf("Index compression requires the REVCIDX component\n");
      compressrdx_cleanup(1);
    }
  }

  nr_elements = cl_max_id(attr);
  if ((nr_elements <= 0) || (cl_errno != CDA_OK)) {
    cl_error("(aborting) cl_max_id() failed");
    compressrdx_cleanup(1);
  }

  corpus_size = cl_max_cpos(attr);
  if ((corpus_size <= 0) || (cl_errno != CDA_OK)) {
    cl_error("(aborting) cl_max_cpos() failed");
    compressrdx_cleanup(1);
  }

  if (output_fn) {
    sprintf(data_fname, "%s.crc", output_fn);
    sprintf(index_fname, "%s.crx", output_fn);
  }
  else {
    s = component_full_name(attr, CompCompRF, NULL);
    assert(s && (cl_errno == CDA_OK));
    strcpy(data_fname, s);

    s = component_full_name(attr, CompCompRFX, NULL);
    assert(s && (cl_errno == CDA_OK));
    strcpy(index_fname, s);
  }

  if (! BFopen(data_fname, "w", &data_file)) {
    Rprintf("ERROR: can't create file %s\n", data_fname);
    perror(data_fname);
    compressrdx_cleanup(1);
  }
  Rprintf("- writing compressed index to %s\n", data_fname);

  if ((index_file = fopen(index_fname, "wb")) == NULL) {
    Rprintf("ERROR: can't create file %s\n", index_fname);
    perror(index_fname);
    compressrdx_cleanup(1);
  }
  Rprintf("- writing compressed index offsets to %s\n", index_fname);

  for (i = 0; i < nr_elements; i++) {

    element_freq = cl_id2freq(attr, i);
    if ((element_freq == 0) || (cl_errno != CDA_OK)) {
      cl_error("(aborting) token frequency == 0\n");
      compressrdx_cleanup(1);
    }

    PStream = cl_new_stream(attr, i);
    if ((PStream == NULL) || (cl_errno != CDA_OK)) {
      cl_error("(aborting) index read error");
      compressrdx_cleanup(1);
    }

    b = compute_ba(element_freq, corpus_size);

    fpos = BFposition(&data_file);
    NwriteInt(fpos, index_file);

    if (debug)
      Rprintf("------------------------------ ID %d (f: %d, b: %d)\n",
              i, element_freq, b);

    last_pos = 0;
    for (k = 0; k < element_freq; k++) {
      if (1 != cl_read_stream(PStream, &new_pos, 1)) {
        cl_error("(aborting) index read error\n");
        compressrdx_cleanup(1);
      }

      gap = new_pos - last_pos;
      last_pos = new_pos;

      if (debug)
        Rprintf("%8d:  gap=%4d, b=%4d\n", codepos, gap, b);

      write_golomb_code(gap, b, &data_file);
      codepos++;
    }

    cl_delete_stream(&PStream);
    BFflush(&data_file);
  }

  fclose(index_file);
  BFclose(&data_file);

  return;
}


/* ================================================== DECOMPRESSION & ERROR CHECKING */


/**
 * Checks a compressed reversed index for errors by decompressing it.
 *
 * This function this assumes that compress_reversed_index() has been called
 * beforehand and made sure that the _uncompressed_ index is usable by CL
 * access functions.
 *
 * @param attr      The attribute to check the index of.
 * @param output_fn Base name for the compressed RDX files to be read
 *                  (if this is null, filenames will be taken from the
 *                  attribute).
 */
void
decompress_check_reversed_index(Attribute *attr, char *output_fn, char *corpus_id, int debug)
{
  char *s;
  char data_fname[CL_MAX_FILENAME_LENGTH];
  char index_fname[CL_MAX_FILENAME_LENGTH];

  int nr_elements;
  int element_freq;
  int corpus_size;
  int pos, gap;

  int b;
  int i, k;

  BFile data_file;
  FILE *index_file;

  PositionStream PStream;
  int true_pos;


  Rprintf("VALIDATING %s.%s\n", corpus_id, attr->any.name);

  nr_elements = cl_max_id(attr);
  if ((nr_elements <= 0) || (cl_errno != CDA_OK)) {
    cl_error("(aborting) cl_max_id() failed");
    compressrdx_cleanup(1);
  }

  corpus_size = cl_max_cpos(attr);
  if ((corpus_size <= 0) || (cl_errno != CDA_OK)) {
    cl_error("(aborting) cl_max_cpos() failed");
    compressrdx_cleanup(1);
  }

  if (output_fn) {
    sprintf(data_fname, "%s.crc", output_fn);
    sprintf(index_fname, "%s.crx", output_fn);
  }
  else {
    s = component_full_name(attr, CompCompRF, NULL);
    assert(s && (cl_errno == CDA_OK));
    strcpy(data_fname, s);

    s = component_full_name(attr, CompCompRFX, NULL);
    assert(s && (cl_errno == CDA_OK));
    strcpy(index_fname, s);
  }

  if (! BFopen(data_fname, "r", &data_file)) {
    Rprintf("ERROR: can't open file %s\n", data_fname);
    perror(data_fname);
    compressrdx_cleanup(1);
  }
  Rprintf("- reading compressed index from %s\n", data_fname);

  if ((index_file = fopen(index_fname, "r")) == NULL) {
    Rprintf("ERROR: can't open file %s\n", index_fname);
    perror(index_fname);
    compressrdx_cleanup(1);
  }
  Rprintf("- reading compressed index offsets from %s\n", index_fname);


  for (i = 0; i < nr_elements; i++) {

    element_freq = cl_id2freq(attr, i);
    if ((element_freq == 0) || (cl_errno != CDA_OK)) {
      cl_error("(aborting) token frequency == 0\n");
      compressrdx_cleanup(1);
    }

    PStream = cl_new_stream(attr, i);
    if ((PStream == NULL) || (cl_errno != CDA_OK)) {
      cl_error("(aborting) index read error");
      compressrdx_cleanup(1);
    }

    b = compute_ba(element_freq, corpus_size);

    if (debug)
      Rprintf("------------------------------ ID %d (f: %d, b: %d)\n",
              i, element_freq, b);

    pos = 0;
    for (k = 0; k < element_freq; k++) {

      gap = read_golomb_code_bf(b, &data_file);
      pos += gap;

      if (1 != cl_read_stream(PStream, &true_pos, 1)) {
        cl_error("(aborting) index read error\n");
        compressrdx_cleanup(1);
      }
      if (pos != true_pos) {
        Rprintf("ERROR: wrong occurrence of type #%d at cpos %d (correct cpos: %d) (on attribute: %s). Aborted.\n",
                i, pos, true_pos, attr->any.name);
        compressrdx_cleanup(1);
      }

    }

    cl_delete_stream(&PStream);
    BFflush(&data_file);
  }

  fclose(index_file);
  BFclose(&data_file);

  /* tell the user it's safe to delete the REVCORP and REVCIDX components now */
  Rprintf("!! You can delete the file <%s> now.\n", component_full_name(attr, CompRevCorpus, NULL));
  Rprintf("!! You can delete the file <%s> now.\n", component_full_name(attr, CompRevCorpusIdx, NULL));

  return;
}

/**
 * Cleans up memory prior to an (error-prompted or normal) exit.
 *
 * @param error_code  Value to be returned by the program when it exits.
 */
void
compressrdx_cleanup(int error_code)
{
  if (corpus)
    cl_delete_corpus(corpus);

  /* if (debug_output != stderr) */
    /* fclose(debug_output) */

  return;
}



