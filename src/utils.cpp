extern "C" {
  #include "cl_min.h"
  #include <attributes.h>
  #include "utils.h"
}


  
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export(name=".cwb_makeall")]]
int cwb_makeall(SEXP x, SEXP registry_dir, SEXP p_attribute){
  
  /* char *progname = "RcppCWB"; */
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char * corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  int validate = 1;

  ComponentID cid = CompLast;

  corpus = cl_new_corpus(registry_directory, corpus_id);
  
  Rprintf("=== Makeall: processing corpus %s ===\n", corpus_id);
  Rprintf("Registry directory: %s\n", corpus->registry_dir);
  
  Attribute *attribute = cl_new_attribute(corpus, attr_name, ATT_POS);
  do_attribute(attribute, cid, validate);

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


// [[Rcpp::export(name=".cwb_encode")]]
int cwb_encode(){
  return 0;                        /* to keep gcc from complaining */
}
