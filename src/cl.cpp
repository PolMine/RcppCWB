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
using namespace Rcpp;
// [[Rcpp::interfaces(r, cpp)]]

char* cl_get_version();

// [[Rcpp::export(name=".cwb_version")]]
Rcpp::StringVector cwb_version(){
  Rcpp::StringVector result(1);
  result(0) = cl_get_version();
  return result;
}


Attribute* make_s_attribute(SEXP corpus, SEXP s_attribute, SEXP registry){
  
  char* reg_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* s_attr = strdup(Rcpp::as<std::string>(s_attribute).c_str());
  char* corpus_pointer  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  Corpus *corpus_obj = cl_new_corpus(reg_dir, corpus_pointer);
  Attribute* att = cl_new_attribute(corpus_obj, s_attr, ATT_STRUC);
  
  return att;
}

// [[Rcpp::export(name=".s_attr")]]
SEXP _s_attr(SEXP corpus, SEXP s_attribute, SEXP registry){
  Attribute* s_attr = make_s_attribute(corpus, s_attribute, registry);
  SEXP ptr = R_MakeExternalPtr(s_attr, R_NilValue, R_NilValue);
  return(ptr);
}

Attribute* make_p_attribute(SEXP corpus, SEXP p_attribute, SEXP registry){
  
  char* reg_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* p_attr = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char* corpus_pointer  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  Corpus *corpus_obj = cl_new_corpus(reg_dir, corpus_pointer);
  Attribute* att = cl_new_attribute(corpus_obj, p_attr, ATT_POS);
  
  return att;
}

// [[Rcpp::export(name=".p_attr")]]
SEXP _p_attr(SEXP corpus, SEXP p_attribute, SEXP registry){
  Attribute* p_attr = make_p_attribute(corpus, p_attribute, registry);
  SEXP ptr = R_MakeExternalPtr(p_attr, R_NilValue, R_NilValue);
  return(ptr);
}


/* these are the wrappers for the functions of the corpus library (CL) */


// [[Rcpp::export(name=".cl_attribute_size")]]
int _cl_attribute_size(SEXP corpus, SEXP attribute, SEXP attribute_type, SEXP registry) {
  int size;
  std::string atype = Rcpp::as<std::string>(attribute_type);
  if (atype == "p"){
    Attribute* att = make_p_attribute(corpus, attribute, registry);
    size = cl_max_cpos(att);
  } else {
    Attribute* att = make_s_attribute(corpus, attribute, registry);
    size = cl_max_struc(att);
  }
  return(size);
}

// [[Rcpp::export(name=".p_attr_size")]]
int _p_attr_size(SEXP p_attr) {
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  int size = cl_max_cpos(att);
  return(size);
}

// [[Rcpp::export(name=".s_attr_size")]]
int _s_attr_size(SEXP s_attr) {
  Attribute* att = (Attribute*)R_ExternalPtrAddr(s_attr);
  int size = cl_max_struc(att);
  return(size);
}


// [[Rcpp::export(name=".p_attr_lexicon_size")]]
int _lexicon_size(SEXP p_attr){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  int size = cl_max_id(att);
  return( size );
}


// [[Rcpp::export(name=".cl_lexicon_size")]]
int _cl_lexicon_size(SEXP corpus, SEXP p_attribute, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int size = cl_max_id(att);
  return( size );
}


Rcpp::IntegerVector _cl_cpos2struc(Attribute* att, Rcpp::IntegerVector cpos){
  int i;
  int len = cpos.length();
  Rcpp::IntegerVector strucs(len);
  for (i = 0; i < len; i++){
    strucs(i) = cl_cpos2struc(att, cpos(i));
  }
  return( strucs );
}


// [[Rcpp::export(name=".cl_cpos2struc")]]
Rcpp::IntegerVector _cl_cpos2struc(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  return(_cl_cpos2struc(att, cpos));
}

// [[Rcpp::export(name=".cpos_to_struc")]]
Rcpp::IntegerVector _cpos_to_struc(SEXP s_attr, Rcpp::IntegerVector cpos){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(s_attr);
  return(_cl_cpos2struc(att, cpos));
}

Rcpp::StringVector _cl_cpos2str(Attribute* att, Rcpp::IntegerVector cpos){
  int i;
  int len;
  len = cpos.length();
  Rcpp::StringVector result(len);
  for (i = 0; i < len; i++){
    result(i) = cl_cpos2str(att, cpos(i));
  }
  return(result);
}


// [[Rcpp::export(name=".cl_cpos2str")]]
Rcpp::StringVector _cl_cpos2str(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  return(_cl_cpos2str(att, cpos));
}

// [[Rcpp::export(name=".cpos_to_str")]]
Rcpp::StringVector _cpos_to_str(SEXP p_attr, Rcpp::IntegerVector cpos){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  return(_cl_cpos2str(att, cpos));
}


/* this is the worker */
Rcpp::IntegerVector _cl_cpos2id(Attribute * att, Rcpp::IntegerVector cpos){
  int i;
  int len = cpos.length();
  Rcpp::IntegerVector ids(len);
  
  for (i = 0; i < len; i++){
    ids(i) = cl_cpos2id(att, cpos(i));
  }
  return( ids );
}

// [[Rcpp::export(name=".cl_cpos2id")]]
Rcpp::IntegerVector _cl_cpos2id(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  return(_cl_cpos2id(att, cpos));
}


// [[Rcpp::export(name=".cpos_to_id")]]
Rcpp::IntegerVector _cpos_to_id(SEXP p_attr, Rcpp::IntegerVector cpos){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  return(_cl_cpos2id(att, cpos));
}


Rcpp::IntegerVector _cl_struc2cpos(Attribute * att, int struc){
  Rcpp::IntegerVector cpos(2);
  int lb, rb;
  if (struc >= 0){
    cl_struc2cpos(att, struc, &lb, &rb);
    cpos(0) = lb;
    cpos(1) = rb;
  } else {
    cpos(0) = NA_INTEGER;
    cpos(1) = NA_INTEGER;
  }
  return( cpos );
}


// [[Rcpp::export(name=".cl_struc2cpos")]]
Rcpp::IntegerVector _cl_struc2cpos(SEXP corpus, SEXP s_attribute, SEXP registry, int struc){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  return(_cl_struc2cpos(att, struc));
}

// [[Rcpp::export(name=".struc_to_cpos")]]
Rcpp::IntegerVector _struc_to_cpos(SEXP s_attr, int struc){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(s_attr);
  return(_cl_struc2cpos(att, struc));
}


// [[Rcpp::export(name=".cl_id2str")]]
Rcpp::StringVector _cl_id2str(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector id){
  /* potentially cpos > max cpos causing a crash */
  int len = id.length();
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  Rcpp::StringVector result(len);
  int i;
  for (i = 0; i < len; i++){
    result(i) = cl_id2str(att, id(i));
  }
  return ( result );
}


Rcpp::StringVector _cl_struc2str(Attribute* att, Rcpp::IntegerVector struc){
  int len = struc.length();
  Rcpp::StringVector result(len);
  if ( cl_struc_values(att) ){
    int i;
    for (i = 0; i < len; i++){
      if (struc(i) >= 0){
        result(i) = cl_struc2str(att, struc(i));
      } else {
        result(i) = NA_STRING;
      }
    }
  }
  return ( result );
}


// [[Rcpp::export(name=".cl_struc2str")]]
Rcpp::StringVector _cl_struc2str(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector struc, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  return (_cl_struc2str(att, struc));
}

// [[Rcpp::export(name=".struc_to_str")]]
Rcpp::StringVector _struc_to_str(SEXP s_attr, Rcpp::IntegerVector struc){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(s_attr);
  return (_cl_struc2str(att, struc));
}


Rcpp::IntegerVector _cl_regex2id(Attribute* att, SEXP regex){
  char *r = strdup(Rcpp::as<std::string>(regex).c_str());
  int *idlist;
  int len;
  int i;
  idlist = collect_matching_ids(att, r, 0, &len);
  Rcpp::IntegerVector result(len);
  for (i = 0; i < len; i++){
    result(i) = idlist[i];
  }
  return( result );
}

// [[Rcpp::export(name=".cl_regex2id")]]
Rcpp::IntegerVector _cl_regex2id(SEXP corpus, SEXP p_attribute, SEXP regex, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  return(_cl_regex2id(att, regex));
}

// [[Rcpp::export(name=".regex_to_id")]]
Rcpp::IntegerVector _regex_to_id(SEXP p_attr, SEXP regex){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  return (_cl_regex2id(att, regex));
}


Rcpp::IntegerVector _cl_str2id(Attribute * att, Rcpp::StringVector str){
  int len = str.length();
  Rcpp::IntegerVector ids(len);
  int i;
  for (i = 0; i < len; i++){
    ids(i) = cl_str2id(att, str(i));
  }
  return( ids );
}


// [[Rcpp::export(name=".cl_str2id")]]
Rcpp::IntegerVector _cl_str2id(SEXP corpus, SEXP p_attribute, Rcpp::StringVector str, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  return(_cl_str2id(att, str));
}

// [[Rcpp::export(name=".str_to_id")]]
Rcpp::IntegerVector _str_to_id(SEXP p_attr, Rcpp::StringVector str){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  return (_cl_str2id(att, str));
}

Rcpp::IntegerVector _cl_id2freq(Attribute* att, Rcpp::IntegerVector id){
  int len = id.length();
  Rcpp::IntegerVector result(len);
  int i;
  for (i = 0; i < len; i++){
    result(i) = cl_id2freq(att, id(i));
  }
  return( result );
}


// [[Rcpp::export(name=".cl_id2freq")]]
Rcpp::IntegerVector _cl_id2freq(SEXP corpus, SEXP p_attribute, Rcpp::IntegerVector id, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  return(_cl_id2freq(att, id));
}


// [[Rcpp::export(name=".id_to_freq")]]
Rcpp::IntegerVector _id_to_freq(SEXP p_attr, Rcpp::IntegerVector id){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  return (_cl_id2freq(att, id));
}


Rcpp::IntegerVector _cl_id2cpos(Attribute* att, SEXP id){
  int *cposlist;
  int len;
  int idx = Rcpp::as<int>(id);
  cposlist = cl_id2cpos(att, idx, &len);
  Rcpp::IntegerVector cpos(len);
  int i;
  for (i = 0; i < len; i++){
    cpos(i) = cposlist[i];
  }
  return( cpos );
}


// [[Rcpp::export(name=".cl_id2cpos")]]
Rcpp::IntegerVector _cl_id2cpos(SEXP corpus, SEXP p_attribute, SEXP id, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  return(_cl_id2cpos(att, id));
}

// [[Rcpp::export(name=".id_to_cpos")]]
Rcpp::IntegerVector _id_to_cpos(SEXP p_attr, Rcpp::IntegerVector id){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(p_attr);
  return (_cl_id2cpos(att, id));
}


Rcpp::IntegerVector _cl_cpos2lbound(Attribute * att, Rcpp::IntegerVector cpos){
  int lb, rb;
  int i;
  int struc;
  int len = cpos.length();
  Rcpp::IntegerVector result(len);
  
  for (i = 0; i < len; i++){
    struc = cl_cpos2struc(att, cpos(i));
    cl_struc2cpos(att, struc, &lb, &rb);
    result(i) = lb;
  }
  
  return( result );
}


// [[Rcpp::export(name=".cl_cpos2lbound")]]
Rcpp::IntegerVector _cl_cpos2lbound(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  return(_cl_cpos2lbound(att, cpos));
}

// [[Rcpp::export(name=".cpos_to_lbound")]]
Rcpp::IntegerVector _cpos_to_lbound(SEXP s_attr, Rcpp::IntegerVector cpos){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(s_attr);
  return (_cl_cpos2lbound(att, cpos));
}


Rcpp::IntegerVector _cl_cpos2rbound(Attribute* att, Rcpp::IntegerVector cpos){
  int lb, rb;
  int i;
  int struc;
  
  int len = cpos.length();
  Rcpp::IntegerVector result(len);
  
  for (i = 0; i < len; i++){
    struc = cl_cpos2struc(att, cpos(i));
    cl_struc2cpos(att, struc, &lb, &rb);
    result(i) = rb;
  }
  
  return( result );
}


// [[Rcpp::export(name=".cl_cpos2rbound")]]
Rcpp::IntegerVector _cl_cpos2rbound(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  return(_cl_cpos2rbound(att, cpos));
}

// [[Rcpp::export(name=".cpos_to_rbound")]]
Rcpp::IntegerVector _cpos_to_rbound(SEXP s_attr, Rcpp::IntegerVector cpos){
  Attribute* att = (Attribute*)R_ExternalPtrAddr(s_attr);
  return (_cl_cpos2rbound(att, cpos));
}


// [[Rcpp::export(name=".cl_find_corpus")]]
SEXP _cl_find_corpus(SEXP corpus, SEXP registry){
  
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* registry_name  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  Corpus * c;
  c = find_corpus(registry_dir, registry_name); 
  
  if (c){
    SEXP p = R_MakeExternalPtr(c, R_NilValue, R_NilValue);
    return p;
  } else {
    return R_NilValue;
  }
  return( R_NilValue );
}

// [[Rcpp::export(name=".cl_new_attribute")]]
SEXP _cl_new_attribute(SEXP corpus_pointer, SEXP s_attribute, int type){
  
  Corpus * c = (Corpus*)R_ExternalPtrAddr(corpus_pointer);
  char* s_attr = strdup(Rcpp::as<std::string>(s_attribute).c_str());

  Attribute* att = cl_new_attribute(c, s_attr, type);
  
  if (att){
    SEXP a = R_MakeExternalPtr(att, R_NilValue, R_NilValue);
    return a;
  } else {
    return R_NilValue;
  }
  return( R_NilValue );
}



// [[Rcpp::export(name=".cl_delete_corpus")]]
int _cl_delete_corpus(SEXP corpus, SEXP registry){
  
  Corpus * c;
  static char *canonical_name = NULL;
  int retval;
  
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* registry_name  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  /* code copied from cl_new_corpus in corpus.c */
  cl_free(canonical_name);
  canonical_name = cl_strdup(registry_name);
  cl_id_tolower(canonical_name);
  if (!cl_id_validate(canonical_name)) {
    Rprintf("cl_new_corpus: <%s> is not a valid corpus name\n", registry_name);
  }
  
  c = find_corpus(registry_dir, canonical_name); 
  if (c){
    c->nr_of_loads = 1;
    cl_delete_corpus(c);
    retval = 1;
  } else {
    retval = 0;
  }
  
  return( retval );
}


// [[Rcpp::export(name=".corpus_is_loaded")]]
int _corpus_is_loaded(SEXP corpus, SEXP registry){
  
  Corpus * c;
  static char *canonical_name = NULL;
  int retval;
  
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* registry_name  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  /* code copied from cl_new_corpus in corpus.c */
  cl_free(canonical_name);
  canonical_name = cl_strdup(registry_name);
  cl_id_tolower(canonical_name);
  if (!cl_id_validate(canonical_name)) {
    Rprintf("cl_new_corpus: <%s> is not a valid corpus name\n", registry_name);
  }
  
  c = find_corpus(registry_dir, canonical_name); 
  if (c){
    retval = 1;
  } else {
    retval = 0;
  }
  
  return( retval );
}


// [[Rcpp::export(name=".cl_charset_name")]]
Rcpp::StringVector _cl_charset_name(SEXP corpus, SEXP registry){
  
  char* corpus_pointer  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* reg_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  Corpus *corpus_obj = cl_new_corpus(reg_dir, corpus_pointer);
  
  Rcpp::StringVector result(1);
  
  result(0) = cl_charset_name(cl_corpus_charset(corpus_obj));
  
  return( result );
}


// [[Rcpp::export(name=".cl_struc_values")]]
int _cl_struc_values(SEXP corpus, SEXP s_attribute, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int y = cl_struc_values(att);
  return y;
}
  

// [[Rcpp::export(name=".corpus_data_dir")]]
Rcpp::StringVector _corpus_data_dir(SEXP corpus, SEXP registry){
  
  Corpus * c;
  Rcpp::StringVector result(1);
  
  char* corpus_id  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  
  c = cl_new_corpus(registry_dir, corpus_id);
  
  if (c){
    result(0) = c->path;
  } else {
    result(0) = NA_STRING;
  }
  
  return( result );

}

// [[Rcpp::export(name=".corpus_info_file")]]
Rcpp::StringVector _corpus_info_file(SEXP corpus, SEXP registry){
  
  Corpus * c;
  Rcpp::StringVector result(1);
  
  char* corpus_id  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  
  c = cl_new_corpus(registry_dir, corpus_id);
  
  if (c){
    result(0) = c->info_file;
  } else {
    result(0) = NA_STRING;
  }
  
  return( result );
}

// [[Rcpp::export(name=".corpus_full_name")]]
Rcpp::StringVector _corpus_full_name(SEXP corpus, SEXP registry){
  
  Corpus * c;
  Rcpp::StringVector result(1);
  
  char* corpus_id  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  
  c = cl_new_corpus(registry_dir, corpus_id);
  
  if (c){
    result(0) = c->name;
  } else {
    result(0) = NA_STRING;
  }
  
  return( result );
}


Rcpp::StringVector corpus_attributes(SEXP corpus, SEXP registry, int attribute_type){
  
  Corpus * c;
  Attribute *att;
  int n, i;
  
  char* corpus_id  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  
  c = cl_new_corpus(registry_dir, corpus_id);
  
  if (c){
    n = 0;
    for (att = c->attributes ; att != NULL ; att = (Attribute *)att->any.next){
      if (0 != (att->any.type & attribute_type)) n++;
    }

    Rcpp::StringVector result(n);
    
    i = 0;
    for (att = c->attributes ; att != NULL ; att = (Attribute *)att->any.next){
      if (0 != (att->any.type & attribute_type)){
        result(i) = cl_strdup(att->any.name);
        i++;
      }
    }
    return(result);
  } else {
    Rcpp::StringVector result(1);
    result(0) = NA_STRING;
    return(result);
  }
}

// [[Rcpp::export(name=".corpus_p_attributes")]]
Rcpp::StringVector corpus_p_attributes(SEXP corpus, SEXP registry){
  Rcpp::StringVector result = corpus_attributes(corpus, registry, ATT_POS);
  return(result);
}


// [[Rcpp::export(name=".corpus_s_attributes")]]
Rcpp::StringVector corpus_s_attributes(SEXP corpus, SEXP registry){
  Rcpp::StringVector result = corpus_attributes(corpus, registry, ATT_STRUC);
  return(result);
}

// [[Rcpp::export(name=".corpus_properties")]]
Rcpp::StringVector corpus_properties(SEXP corpus, SEXP registry){
  
  Corpus * c;
  CorpusProperty p;
  int n, i;

  char* corpus_id  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  c = cl_new_corpus(registry_dir, corpus_id);
  
  p = cl_first_corpus_property(c);
  
  
  n = 0;
  while (p != NULL){
    p = cl_next_corpus_property(p);
    n++;
  }
  Rcpp::StringVector properties(n);
  
  p = cl_first_corpus_property(c);
  i = 0;
  while (p != NULL){
    properties(i) = cl_strdup(p->property);
    p = cl_next_corpus_property(p);
    i++;
  }
  
  return(properties);
}

// [[Rcpp::export(name=".corpus_property")]]
Rcpp::StringVector corpus_property(SEXP corpus, SEXP registry, SEXP property){
  
  Rcpp::StringVector result(1);
  
  Corpus * c;
  char* corpus_id  = strdup(Rcpp::as<std::string>(corpus).c_str());
  char* registry_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  c = cl_new_corpus(registry_dir, corpus_id);
  
  char* prop = strdup(Rcpp::as<std::string>(property).c_str());

  CorpusProperty p = cl_first_corpus_property(c);
    
  while (p != NULL && strcmp(prop, p->property)) p = cl_next_corpus_property(p);
  
  if (p != NULL)
    result(0) =  p->value;
  else
    result(0) = NA_STRING;

  return(result);
}


// [[Rcpp::export(name=".cl_load_corpus")]]
int cl_load_corpus(SEXP corpus, SEXP registry) {
  
  char *corpus_id = strdup(Rcpp::as<std::string>(corpus).c_str());
  char *registry_directory = strdup(Rcpp::as<std::string>(registry).c_str());

  Corpus * c;
  
  c = cl_new_corpus(registry_directory, corpus_id);

  if (c == NULL) {
    return 0;
  } else {
    return 1;
  }
  return 0;
}


// [[Rcpp::export(name=".cl_list_corpora")]]
Rcpp::StringVector cl_list_corpora(){
  
  int n = 0;
  Corpus *c;
  for (c = loaded_corpora; c != NULL; c = c->next) n++;
  
  Rcpp::StringVector result(n);
  int i = 0;
  for (c = loaded_corpora; c != NULL; c = c->next){
    result(i) = c->registry_name;
    i++;
  };
  
  return result;
}


// [[Rcpp::export(name=".corpus_registry_dir")]]
Rcpp::StringVector corpus_registry_dir(SEXP corpus){
  
  char* registry_name = strdup(Rcpp::as<std::string>(corpus).c_str());
  Corpus *c;
  int i, n;

  n = 0;
  for (c = loaded_corpora; c != NULL; c = c->next){
    if (cl_streq(registry_name, c->registry_name)) n++;
  }
  
  if (n > 0){
    
    Rcpp::StringVector reg(n);
    
    i = 0;
    for (c = loaded_corpora; c != NULL; c = c->next) {
      if (cl_streq(registry_name, c->registry_name)){
        reg[i] = c->registry_dir;
        i++;
        if (i == n) break; /* stop early when work is done */
      };
    }
    return reg;
  }
  
  Rcpp::StringVector reg(1);
  reg[0] = NA_STRING;
  return reg;
}

