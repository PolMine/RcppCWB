extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <cl.h>
#include <pcre.h>
#include "globals.h"
#include "context_descriptor.h"
#include "_options.h"
#include "cqp.h"
#include "corpmanag.h"
#include "server.h"
}

#include <Rcpp.h>
#include <stdio.h>
#include <string.h>


using namespace Rcpp;

int cqp_initialization_status = 0;

// [[Rcpp::export(name=".init_cqp")]]
void init_cqp() {
	int		ac = 1;
	char *		av[1];
	av[0] = (char *)"RcppCWB";
	which_app = cqp;
	silent = 1; 
	paging = 0;
	autoshow = 0;
	auto_save = 0;
	server_log = 0;
	enable_macros = 0;

	initialize_cqp(ac, av);
	cqp_initialization_status = 1;
	make_attribute_hash(16384);
}


// [[Rcpp::export(name=".cqp_get_registry")]]
Rcpp::StringVector cqp_get_registry(){
  Rcpp::StringVector result(1);
  result(0) = cl_standard_registry();
  return result;
}

// [[Rcpp::export(name=".cqp_get_status")]]
int cqp_get_status(){
  return cqp_initialization_status;
}


// [[Rcpp::export(name=".cqp_set_registry")]]
SEXP cqp_set_registry(SEXP registry_dir){
  char * registry_new;
  registry_new = (char*)CHAR(STRING_ELT(registry_dir,0));
  registry = cl_strdup(registry_new);
  int		ac = 1;
  char *		av[1];
  av[0] = (char *)"RcppCWB";
  set_current_corpus(NULL, 0); /* required to avoid crash! */ 
  
  initialize_cqp(ac, av);
  make_attribute_hash(16384);
  
  SEXP result = R_NilValue;
  return result;
}


// [[Rcpp::export(name=".cqp_list_corpora")]]
Rcpp::StringVector cqp_list_corpora(){
  
  CorpusList *	cl;
  int	i = 0, n = 0;
  
  /* First count corpora */
  for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
    if (cl->type == SYSTEM) n++;
  }
  Rcpp::StringVector result(n);

  /* Then build list of names */
  for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
    if (cl->type == SYSTEM) {
      result(i) = cl->name;
      i++;
    }
  }
  return result;
  
}


// [[Rcpp::export(name=".cqp_query")]]
SEXP cqp_query(SEXP corpus, SEXP subcorpus, SEXP query){
  
  char * mother = (char*)CHAR(STRING_ELT(corpus,0));
  char * child = (char*)CHAR(STRING_ELT(subcorpus,0));
  char * q = (char*)CHAR(STRING_ELT(query,0));
  char * cqp_query;
  CorpusList *cl;
  
  
  cl = cqi_find_corpus(mother);
  set_current_corpus(cl, 0);

  int len = strlen(child) + strlen(q) + 10;
  
  cqp_query = (char *) cl_malloc(len);
  sprintf(cqp_query, "%s = %s", child, q);
  
  if (!cqi_activate_corpus(mother)){
    Rprintf("activation failed");
  }
  if (!check_subcorpus_name(child)){
    Rprintf("checking subcorpus name failed \n");
  }
  
  cqp_parse_string(cqp_query);

  char *			full_child;
  CorpusList *	childcl;
  
  full_child = combine_subcorpus_spec(mother, child); /* c is the 'physical' part of the mother corpus */
  childcl = cqi_find_corpus(full_child);
  /* if ((childcl) == NULL) {
    printf("subcorpus not found\n");
  } */
  
  SEXP result = R_NilValue;
  return result;
}


// [[Rcpp::export(name=".cqp_subcorpus_size")]]
int cqp_subcorpus_size(SEXP scorpus)
{
  int result;
  char * subcorpus;
  CorpusList * cl;
  
  subcorpus = (char*)CHAR(STRING_ELT(scorpus,0));
  cl = cqi_find_corpus(subcorpus);
  
  if (cl == NULL) {
    result = 0;
  } else {
    result = cl->size;
  }
  return result;
}

// [[Rcpp::export(name=".cqp_list_subcorpora")]]
Rcpp::StringVector cqp_list_subcorpora(SEXP inCorpus)
{
  char * corpus;
  CorpusList *cl, *mother;
  int i = 0, n = 0;
  Rcpp::StringVector result;

  corpus = (char*)CHAR(STRING_ELT(inCorpus,0));
  
  mother = cqi_find_corpus(corpus);
  if (!check_corpus_name(corpus) || mother == NULL) {
    /* Rcpp::StringVector result(1); */
  } else {
    /* First count subcorpora */
    for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
      if (cl->type == SUB && cl->corpus == mother->corpus){
        n++;
      }
    }
    Rprintf("number of subcorpora: %d\n", n);
    Rcpp::StringVector result(n);
    
    /* Then build list of names */
    for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
      if (cl->type == SUB && cl->corpus == mother->corpus) {
        result(i) = cl->name;
        Rprintf("subcorpus name: %s\n", cl->name);
        /* printf("added to result: %s", result(i)); */
        i++;
      }
    }
  }
  return result;
}

// [[Rcpp::export(name=".cqp_dump_subcorpus")]]
Rcpp::IntegerMatrix cqp_dump_subcorpus(SEXP inSubcorpus)
{
  char * subcorpus;
  CorpusList * cl;
  int i;
  int nrows = cqp_subcorpus_size(inSubcorpus);

  subcorpus = (char*)CHAR(STRING_ELT(inSubcorpus,0));
  cl = cqi_find_corpus(subcorpus);
  if (cl == NULL) {
    Rprintf("subcorpus not found\n");
  }
  
  Rcpp::IntegerMatrix result(nrows,2);

  for (i = 0; i < nrows; i++) {
    result(i,0) = cl->range[i].start;
    result(i,1) = cl->range[i].end;
  }
  
  return result;
}

// [[Rcpp::export(name=".cqp_drop_subcorpus")]]
SEXP cqp_drop_subcorpus(SEXP inSubcorpus)
{
  SEXP			result = R_NilValue;
  char *			subcorpus;
  char 			*c, *sc;
  CorpusList *	cl;
  
  PROTECT(inSubcorpus);
  
  subcorpus = (char*)CHAR(STRING_ELT(inSubcorpus,0));
  
  /* Make sure it is a subcorpus, not a root corpus */
  if (!split_subcorpus_spec(subcorpus, &c, &sc)) {
    UNPROTECT(1);
    /* rcqp_error_code(cqi_errno); */
  } else if (sc == NULL) {
    free(c);
    UNPROTECT(1);
    /* error("can't drop a root corpus."); */
  } else {
    free(c); free(sc);
    cl = cqi_find_corpus(subcorpus);
    if (cl == NULL) {
      UNPROTECT(1);
      /* rcqp_error_code(cqi_errno); */
    } else {
      dropcorpus(cl);
    }
  }
  
  UNPROTECT(1);
  
  return result;
}


