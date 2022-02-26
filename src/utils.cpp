#define _sugar_

extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  #include "cl.h"
  #include "cqp.h"
  #include <pcre.h>
  #include "server.h"
  
#include "cwb/cqp/corpmanag.h"
  
  #include "_globalvars.h"
  #include "_eval.h"

  /* includes for utils */
  #include <attributes.h>
  #include "cwb/utils/globals.h"
  #include "cwb/utils/utils.h"
}

#include <Rcpp.h>
using namespace Rcpp;
// [[Rcpp::interfaces(r, cpp)]]


// [[Rcpp::export(name=".cwb_makeall")]]
int cwb_makeall(SEXP x, SEXP registry_dir, SEXP p_attribute){
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char * corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  int validate = 1;
  
  ComponentID cid = CompLast;
  
  corpus = cl_new_corpus(registry_directory, corpus_id);
  
  Rprintf("=== Makeall: processing corpus %s ===\n", corpus_id);
  Rprintf("Registry directory: %s\n", corpus->registry_dir);
  
  Attribute *attribute = cl_new_attribute(corpus, attr_name, ATT_POS);
  makeall_do_attribute(attribute, cid, validate);
  
  Rprintf("========================================\n");
  return 0;
}


// [[Rcpp::export(name=".cwb_huffcode")]]
int cwb_huffcode(SEXP x, SEXP registry_dir, SEXP p_attribute) {
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char * corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  
  char *output_fn = NULL;
  Attribute *attr;
  
  HCD hc;
  
  int i_want_to_believe = 0;        /* skip error checks? */
  /* int all_attributes = 0; */
  
  /* protocol = stdout;   */             /* 'delayed' init (see top of file) */
  
  
  if ((corpus = cl_new_corpus(registry_directory, corpus_id)) == NULL) {
    Rprintf("Corpus %s not found in registry %s . Aborted.\n", 
            corpus_id,
            (registry_directory ? registry_directory
               : central_corpus_directory()));
    return 1;
  }
  
  if ((attr = cl_new_attribute(corpus, attr_name, ATT_POS)) == NULL) {
    Rprintf("Attribute %s.%s doesn't exist. Aborted.\n",  corpus_id, attr_name);
    return 1;
  }
  
  compute_code_lengths(attr, &hc, output_fn);
  if (! i_want_to_believe) decode_check_huff(attr, corpus_id, output_fn);
  
  cl_delete_corpus(corpus);
  
  return 0;
}


// [[Rcpp::export(name=".cwb_compress_rdx")]]
int cwb_compress_rdx(SEXP x, SEXP registry_dir, SEXP p_attribute) {
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char *corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  
  Attribute *attr;
  
  char *output_fn = NULL;
  
#ifdef _WIN32
  int i_want_to_believe = 1;        /* skip error on Windows, for the time being */
#else
  int i_want_to_believe = 0;        /* do not skip error checks on macOS and Linux */
#endif
  
  
  
  
  /* debug_output = stderr; */        /* 'delayed' init (see top of file) */
  int debug = 0;
  
  if ((corpus = cl_new_corpus(registry_directory, corpus_id)) == NULL) {
    Rprintf("Corpus %s not found in registry %s . Aborted.\n", 
            corpus_id,
            (registry_directory ? registry_directory
               : central_corpus_directory()));
    compressrdx_cleanup(1);
  }
  
  if ((attr = find_attribute(corpus, attr_name, ATT_POS, NULL)) == NULL) {
    Rprintf("Attribute %s.%s doesn't exist. Aborted.\n", corpus_id, attr_name);
    compressrdx_cleanup(1);
  }
  
  compress_reversed_index(attr, output_fn, corpus_id, debug);
  if (! i_want_to_believe) decompress_check_reversed_index(attr, output_fn, corpus_id, debug);
  
  /* compressrdx_cleanup(1);  */
  return 0;                        /* to keep gcc from complaining */
}




// [[Rcpp::export(name=".cwb_encode")]]
int cwb_encode(
    SEXP regfile, SEXP data_dir, SEXP vrt_dir, SEXP encoding, Rcpp::StringVector p_attributes,
    Rcpp::StringVector s_attributes_anno, Rcpp::StringVector s_attributes_noanno,
    int skip_blank_lines, int strip_whitespace, int xml, int quiet, int verbosity){
  
  directory = strdup(Rcpp::as<std::string>(data_dir).c_str());
  registry_file = strdup(Rcpp::as<std::string>(regfile).c_str());
  encoding_charset_name = strdup(Rcpp::as<std::string>(encoding).c_str());
  
  /* reset global variables to initial state */
  p_encoder_ix = 0;
  s_encoder_ix = 0;
  nr_input_files = 0;
  current_input_file = 0;
  current_input_file_name = NULL; 
  
  /* configure encoder */
  xml_aware = xml;
  skip_empty_lines = skip_blank_lines;
  strip_blanks = strip_whitespace;
  
  verbose = verbosity;
  quietly = quiet;

  /* declare p-attributes */
  
  int p_attrs_n = p_attributes.length();
  int m;
  for (m = 0; m < p_attrs_n; m++){
    /* wattr_declare(p_attributes(m), directory, 0); */
    p_att_declare(p_attributes(m), directory, 0);
  }
  
  /* declare s-attribute with annotations */
  
  int s_attrs_len = s_attributes_anno.length();
  for (m = 0; m < s_attrs_len; m++){
    /* range_declare(s_attributes_anno(m), directory, 1, 0); */
    s_att_declare(s_attributes_anno(m), directory, 1, 0);
  }
  
  /* declare s-attribute without annotations */
  s_attrs_len = s_attributes_noanno.length();
  for (m = 0; m < s_attrs_len; m++){
    /* range_declare(s_attributes_noanno(m), directory, 0, 0); */
    s_att_declare(s_attributes_noanno(m), directory, 0, 0);
  }
  
  
  input_files = cl_new_string_list();
  cl_string_list vrt_files = encode_scan_directory(strdup(Rcpp::as<std::string>(vrt_dir).c_str()));
  
  int i, len;
  len = cl_string_list_size(vrt_files);
  for (i = 0; i < len; i++)
    cl_string_list_append(input_files, cl_string_list_get(vrt_files, i));
  cl_delete_string_list(vrt_files); /* allocated strings have been moved into input_files, so don't free() them */
    
  
  nr_input_files = cwb_encode_worker(input_files);
  return nr_input_files;
}

