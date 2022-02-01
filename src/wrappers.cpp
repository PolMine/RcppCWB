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
  
  #include "_globalvars.h"
  #include "_eval.h"

  /* includes for utils */
  #include <attributes.h>
  #include "cwb/utils/globals.h"
  #include "cwb/utils/utils.h"
}

#include <Rcpp.h>
using namespace Rcpp;

char* cl_get_version();
int cqp_initialization_status = 0;

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
Rcpp::IntegerVector _cl_cpos2lbound(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  int lb, rb;
  int i;
  int struc;
  
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int len = cpos.length();
  Rcpp::IntegerVector result(len);
  
  for (i = 0; i < len; i++){
    struc = cl_cpos2struc(att, cpos(i));
    cl_struc2cpos(att, struc, &lb, &rb);
    result(i) = lb;
  }
  
  return( result );
}


// [[Rcpp::export(name=".cl_cpos2rbound")]]
Rcpp::IntegerVector _cl_cpos2rbound(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  int lb, rb;
  int i;
  int struc;
  
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int len = cpos.length();
  Rcpp::IntegerVector result(len);
  
  for (i = 0; i < len; i++){
    struc = cl_cpos2struc(att, cpos(i));
    cl_struc2cpos(att, struc, &lb, &rb);
    result(i) = rb;
  }
  
  return( result );
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
  
  /* is this necessary */
  char	*c, *sc;
  if (!split_subcorpus_spec(mother, &c, &sc)) {
    Rprintf("ERROR (function: split_subcorpus_spec)");
  }
  
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
  
  if (!cqp_parse_string(cqp_query)){
    Rprintf("ERROR: Cannot parse the CQP query.\n");
  } else {
    char *			full_child;
    CorpusList *	childcl;
    
    full_child = combine_subcorpus_spec(mother, child); /* c is the 'physical' part of the mother corpus */

    childcl = cqi_find_corpus(full_child);
    if ((childcl) == NULL) {
      Rprintf("subcorpus not found\n");
    } 
  }

  
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
  /*
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
      dropcorpus(cl);
    }
  }
  
  UNPROTECT(1);
  */
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
int cwb_encode(SEXP regfile, SEXP data_dir, SEXP vrt_dir, SEXP encoding, Rcpp::StringVector p_attributes, Rcpp::StringVector s_attributes_anno, Rcpp::StringVector s_attributes_noanno){
  
  directory = strdup(Rcpp::as<std::string>(data_dir).c_str());
  registry_file = strdup(Rcpp::as<std::string>(regfile).c_str());
  encoding_charset_name = strdup(Rcpp::as<std::string>(encoding).c_str());
  
  xml_aware++;
  skip_empty_lines++;
  strip_blanks++;

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

// [[Rcpp::export(name=".cwb_version")]]
Rcpp::StringVector cwb_version(){
  Rcpp::StringVector result(1);
  result(0) = cl_get_version();
  return result;
}
