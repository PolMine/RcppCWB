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
}

#include <Rcpp.h>
/* do not use namespace - would create conflict with Range type */
/* using namespace Rcpp; */

// [[Rcpp::interfaces(r, cpp)]]

int cqp_initialization_status = 0;
CorpusList *corpuslist = NULL;

// [[Rcpp::export(name=".init_cqp")]]
void init_cqp() {
	int		ac = 1;
	char *		av[1];
	av[0] = (char *)"RcppCWB";
	which_app = cqp;
	silent = 0; 
	verbose_parser = 0;
	paging = 0;
	autoshow = 0;
	auto_save = 0;
	server_log = 0;
	enable_macros = 0;

	initialize_cqp(ac, av);
	cqp_initialization_status = 1;
	make_attribute_hash(16384);
}

// [[Rcpp::export(name=".cqp_verbosity")]]
void cqp_verbosity(int quietly, int verbose) {
  silent = quietly; 
  verbose_parser = verbose;
  return;
}


// [[Rcpp::export(name=".cqp_get_registry")]]
Rcpp::StringVector cqp_get_registry(){
  Rcpp::StringVector result(1);
  /* result(0) = cl_standard_registry(); */
  result(0) = registry;
  return result;
}

// [[Rcpp::export(name=".cqp_get_status")]]
int cqp_get_status(){
  return cqp_initialization_status;
}


// [[Rcpp::export(name=".cqp_set_registry")]]
SEXP cqp_set_registry(SEXP registry_dir){
  
  registry = strdup(Rcpp::as<std::string>(registry_dir).c_str());
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
  SEXP result;
  
  /* is this necessary */
  char	*c, *sc;
  if (!split_subcorpus_spec(mother, &c, &sc)) {
    Rprintf("ERROR (function: split_subcorpus_spec)");
  }
  
  cl = cqi_find_corpus(mother);
  set_current_corpus(cl, 0);

  int len = strlen(child) + strlen(q) + 10;
  cqp_query = (char *) cl_malloc(len);
  
  snprintf(cqp_query, len, "%s = %s", child, q);

  if (!cqi_activate_corpus(mother)){
    Rprintf("activation failed");
  }
  if (!check_subcorpus_name(child)){
    Rprintf("checking subcorpus name failed \n");
  }
  
  if (!cqp_parse_string(cqp_query)){
    Rprintf("ERROR: Cannot parse the CQP query.\n");
    result = R_NilValue;
  } else {
    char *			full_child;
    CorpusList *	childcl;
    
    if (strlen(c) > 0){
      full_child = combine_subcorpus_spec(c, child);
    } else {
      full_child = combine_subcorpus_spec(mother, child);
    }

    childcl = cqi_find_corpus(full_child);
    if ((childcl) == NULL) {
      Rprintf("subcorpus not found\n");
      result = R_NilValue;
    } else {
      result = R_MakeExternalPtr(childcl, R_NilValue, R_NilValue);
    }
  }

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

  corpus = (char*)CHAR(STRING_ELT(inCorpus,0));
  
  mother = cqi_find_corpus(corpus);
  if (!check_corpus_name(corpus) || mother == NULL) {
    Rcpp::StringVector result;
    return result;
  } else {
    /* First count subcorpora */
    for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
      if (cl->type == SUB && cl->corpus == mother->corpus){
        n++;
      }
    }
    Rcpp::StringVector result(n);
    
    /* Then build list of names */
    for (cl = FirstCorpusFromList(); cl != NULL; cl = NextCorpusFromList(cl)) {
      if (cl->type == SUB && cl->corpus == mother->corpus) {
        result(i) = cl->name;
        i++;
      }
    }
    return result;
  }
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


// [[Rcpp::export(name=".cqp_subcorpus_regions")]]
Rcpp::IntegerMatrix cqp_subcorpus_regions(SEXP subcorpus)
{
  CorpusList * cl = (CorpusList*)R_ExternalPtrAddr(subcorpus);
  int nrows = cl->size;
  int i;

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
  
  if (!split_subcorpus_spec(subcorpus, &c, &sc)) {
    UNPROTECT(1);
  } else if (sc == NULL) {
    free(c);
    UNPROTECT(1);
  } else {
    free(c); free(sc);
    cl = cqi_find_corpus(subcorpus);
    if (cl == NULL) {
      UNPROTECT(1);
    } else {
      dropcorpus(cl, corpuslist);
    }
  }
  
  UNPROTECT(1);
  return result;
}



// [[Rcpp::export(name=".check_corpus")]]
int check_corpus(SEXP corpus){
  
  char * c;
  CorpusList * cl;
  
  c = (char*)CHAR(STRING_ELT(corpus,0));
  cl = findcorpus(c, SYSTEM, 0);
  
  if (cl == NULL || !access_corpus(cl)) {
    return 0;
  } else {
    return 1;
  }
}



// [[Rcpp::export(name=".cqp_load_corpus")]]
int cqp_load_corpus(SEXP corpus, SEXP registry){
  char          *entry;
  char          *dirname;
  CorpusList * cl;

  entry = strdup(Rcpp::as<std::string>(corpus).c_str());
  dirname = strdup(Rcpp::as<std::string>(registry).c_str());
  
  cl = ensure_syscorpus(dirname, entry);
  if (cl == NULL) return 0;
  return 1;
}


// [[Rcpp::export(name=".region_matrix_to_subcorpus")]]
SEXP region_matrix_to_subcorpus(Rcpp::IntegerMatrix region_matrix, SEXP corpus, SEXP subcorpus){
  
  int i, n, corpus_size;
  SEXP sc;
  char* id;
  Corpus* c;
  Attribute *attr;
  CorpusList *cl;
  
  c = (Corpus*)R_ExternalPtrAddr(corpus);
  id = strdup(Rcpp::as<std::string>(subcorpus).c_str());
  n = region_matrix.nrow();
  
  cl = (CorpusList *)cl_malloc(sizeof(CorpusList));
  
  cl->name = id;
  
  char* mum = cl_strdup(c->registry_name);
  cl_id_toupper(mum);
  cl->mother_name = mum;
  
  attr = cl_new_attribute(c, CWB_DEFAULT_ATT_NAME, ATT_POS);
  corpus_size = cl_max_cpos(attr);
  cl->mother_size = corpus_size;
  
  cl->registry = c->registry_dir;
  cl->abs_fn = NULL;
  cl->type = SUB;
  
  cl->local_dir = NULL;
  
  cl->query_corpus = NULL;
  cl->query_text = NULL;
  
  cl->saved = False;
  cl->loaded = True;
  cl->needs_update = False;
  cl->corpus = c;
  
  cl->range = (Range *)cl_malloc(sizeof(Range) * n);
  for (i = 0; i < n; i++) {
    cl->range[i].start = region_matrix(i,0);
    cl->range[i].end   = region_matrix(i,1);
  }
  
  cl->size = n;
  cl->sortidx = NULL;
  cl->targets = NULL;
  cl->keywords = NULL;
  
  cl->cd = NULL;
  cl->next = NULL;
  cl->next = corpuslist;
  corpuslist = cl;

  sc = R_MakeExternalPtr(cl, R_NilValue, R_NilValue);
  return(sc);
}
