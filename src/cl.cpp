extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  #include <cl.h>
  #include <pcre.h>
}

#include <Rcpp.h>


Attribute* make_s_attribute(SEXP corpus, SEXP s_attribute, SEXP registry){
  
  char* reg_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* s_attr = strdup(Rcpp::as<std::string>(s_attribute).c_str());
  char* corpus_pointer  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  Corpus *corpus_obj = cl_new_corpus(reg_dir, corpus_pointer);
  Attribute* att = cl_new_attribute(corpus_obj, s_attr, ATT_STRUC);
  
  return att;
}


Attribute* make_p_attribute(SEXP corpus, SEXP p_attribute, SEXP registry){
  
  char* reg_dir = strdup(Rcpp::as<std::string>(registry).c_str());
  char* p_attr = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char* corpus_pointer  = strdup(Rcpp::as<std::string>(corpus).c_str());
  
  Corpus *corpus_obj = cl_new_corpus(reg_dir, corpus_pointer);
  Attribute* att = cl_new_attribute(corpus_obj, p_attr, ATT_POS);
  
  return att;
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


// [[Rcpp::export(name=".cl_lexicon_size")]]
int _cl_lexicon_size(SEXP corpus, SEXP p_attribute, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int size = cl_max_id(att);
  return( size );
}


// [[Rcpp::export(name=".cl_cpos2struc")]]
Rcpp::IntegerVector _cl_cpos2struc(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int i;
  int len = cpos.length();
  Rcpp::IntegerVector strucs(len);
  for (i = 0; i < len; i++){
    strucs(i) = cl_cpos2struc(att, cpos(i));
  }
  return( strucs );
}


// [[Rcpp::export(name=".cl_cpos2str")]]
Rcpp::StringVector _cl_cpos2str(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int i;
  int len;
  len = cpos.length();
  Rcpp::StringVector result(len);
  for (i = 0; i < len; i++){
    result(i) = cl_cpos2str(att, cpos(i));
  }
  return(result);
}


// [[Rcpp::export(name=".cl_cpos2id")]]
Rcpp::IntegerVector _cl_cpos2id(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int i;
  int len = cpos.length();
  Rcpp::IntegerVector ids(len);
  for (i = 0; i < len; i++){
    ids(i) = cl_cpos2id(att, cpos(i));
  }
  return( ids );
}

// [[Rcpp::export(name=".cl_struc2cpos")]]
Rcpp::IntegerVector _cl_struc2cpos(SEXP corpus, SEXP s_attribute, SEXP registry, int struc){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  Rcpp::IntegerVector cpos(2);
  int lb, rb;
  cl_struc2cpos(att, struc, &lb, &rb);
  cpos(0) = lb;
  cpos(1) = rb;
  return( cpos );
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


// [[Rcpp::export(name=".cl_struc2str")]]
Rcpp::StringVector _cl_struc2str(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector struc, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int len = struc.length();
  Rcpp::StringVector result(len);
  if ( cl_struc_values(att) ){
    int i;
    for (i = 0; i < len; i++){
      result(i) = cl_struc2str(att, struc(i));
    }
  }
  return ( result );
}


// [[Rcpp::export(name=".cl_regex2id")]]
Rcpp::IntegerVector _cl_regex2id(SEXP corpus, SEXP p_attribute, SEXP regex, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
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


// [[Rcpp::export(name=".cl_str2id")]]
Rcpp::IntegerVector _cl_str2id(SEXP corpus, SEXP p_attribute, Rcpp::StringVector str, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int len = str.length();
  Rcpp::IntegerVector ids(len);
  int i;
  for (i = 0; i < len; i++){
    ids(i) = cl_str2id(att, str(i));
  }
  return( ids );
}

// [[Rcpp::export(name=".cl_id2freq")]]
Rcpp::IntegerVector _cl_id2freq(SEXP corpus, SEXP p_attribute, Rcpp::IntegerVector id, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int len = id.length();
  Rcpp::IntegerVector result(len);
  int i;
  for (i = 0; i < len; i++){
    result(i) = cl_id2freq(att, id(i));
  }
  return( result );
}


// [[Rcpp::export(name=".cl_id2cpos")]]
Rcpp::IntegerVector _cl_id2cpos(SEXP corpus, SEXP p_attribute, SEXP id, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
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


// [[Rcpp::export(name=".cl_cpos2lbound")]]
int _cl_cpos2lbound(SEXP corpus, SEXP s_attribute, SEXP cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int cpos_int = Rcpp::as<int>(cpos);
  int struc = cl_cpos2struc(att, cpos_int);
  int lb, rb;
  cl_struc2cpos(att, struc, &lb, &rb);
  return( lb );
}

// [[Rcpp::export(name=".cl_cpos2rbound")]]
int _cl_cpos2rbound(SEXP corpus, SEXP s_attribute, SEXP cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int cpos_int = Rcpp::as<int>(cpos);
  int struc = cl_cpos2struc(att, cpos_int);
  int lb, rb;
  cl_struc2cpos(att, struc, &lb, &rb);
  return( rb );
}
