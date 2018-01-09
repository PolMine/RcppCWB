extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  #include <cl.h>
  #include <pcre.h>
}

#include <Rcpp.h>
#include <stdio.h>
#include <string.h>

int strcmp ( const char * str1, const char * str2 );
size_t strlen ( const char * str );
FILE *fdopen(int fd, const char *mode);

int region_matrix_to_size(Rcpp::IntegerMatrix matrix){
  int n;
  int size = 0;
  for (n = 0; n < matrix.nrow(); n++){
    size += matrix(n,1) - matrix(n,0) + 1;
  }
  return size;
}



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



// [[Rcpp::export]]
Rcpp::StringVector decode_s_attribute(SEXP corpus, SEXP s_attribute, SEXP registry) {
  
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  
  int n;
  int att_size = cl_max_struc(att);
  
  Rcpp::StringVector result(att_size);

  for (n = 0; n < att_size; n++) {
    result[n] = cl_struc2str(att, n);
  }
  
  return result;
}


// [[Rcpp::export]]
Rcpp::IntegerVector get_count_vector(SEXP corpus, SEXP p_attribute, SEXP registry){
  
  Attribute *att = make_p_attribute(corpus, p_attribute, registry);
  
  int n;
  int max_id = cl_max_id(att);
  Rcpp::IntegerVector count(max_id);
  
  for (n = 0; n < max_id; n++){
    count[n] = cl_id2freq(att, n);
  }
  return count;
}


// [[Rcpp::export]]
Rcpp::IntegerMatrix get_region_matrix(SEXP corpus, SEXP s_attribute, SEXP strucs, SEXP registry){
  
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  
  std::vector<int> strucs_int = Rcpp::as<std::vector<int> >(strucs);
  int strucs_length = strucs_int.size();
  int n, start, end;
  
  Rcpp::IntegerMatrix cpos_matrix(strucs_length,2);
  for (n = 0; n < strucs_length; n++){
    cl_struc2cpos(att, strucs_int[n], &start, &end);
    cpos_matrix(n,0) = start;
    cpos_matrix(n,1) = end;
  }
  return cpos_matrix;
}


// [[Rcpp::export]]
Rcpp::IntegerMatrix get_cbow_matrix(SEXP corpus, SEXP p_attribute, SEXP registry, SEXP matrix, SEXP window){
  
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  
  int window_size = Rcpp::as<int>(window);
  Rcpp::IntegerMatrix region_matrix(matrix);
  
  int ncol_window_matrix = window_size * 2 + 1;
  int nrow_window_matrix = region_matrix_to_size(region_matrix);

  Rcpp::IntegerMatrix window_matrix(nrow_window_matrix, ncol_window_matrix);
  std::fill(window_matrix.begin(), window_matrix.end(), -1);
  
  int cpos, col, id, region_size, row_to_fill, n;
  int row_base = 0;
  int row = 0;
  for (n = 0; n < region_matrix.nrow(); n++){
    region_size = region_matrix(n,1) - region_matrix(n,0) + 1;
    for (cpos = region_matrix(n,0); cpos <= region_matrix(n,1); cpos++){
      id = cl_cpos2id(att, cpos);
      for (col = 0; col < window_matrix.ncol(); col++){
        row_to_fill = row - col + window_size;
        if (row_to_fill >= row_base && row_to_fill < (row_base + region_size)){
          window_matrix(row_to_fill,col) = id;
        }
      }
      row++;
    }
    row_base = row;
  }
  return window_matrix;
}


// [[Rcpp::export]]
Rcpp::IntegerVector regions_to_ids(SEXP corpus, SEXP p_attribute, SEXP registry, SEXP matrix){
  
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  
  Rcpp::IntegerMatrix region_matrix(matrix);
  
  int size = region_matrix_to_size(region_matrix);
  Rcpp::IntegerVector ids(size);
  
  int n, cpos;
  int i = 0;
  for (n = 0; n < region_matrix.nrow(); n++){
    for (cpos = region_matrix(n,0); cpos <= region_matrix(n,1); cpos++){
      ids(i) = cl_cpos2id(att, cpos);
      i++;
    }
  }
  
  return ids;
}

Rcpp::IntegerMatrix ids_to_count_matrix(Rcpp::IntegerVector ids){
  
  Rcpp::IntegerVector count(max(ids) + 1);
  
  int n;
  for (n = 0; n < ids.length(); n++){
    count(ids[n])++;
  }
  
  int filled = 0;
  for (n = 0; n < count.length(); n++){
    if (count[n] > 0){
      filled++;
    }
  }
  
  Rcpp::IntegerMatrix count_matrix(filled,2);
  int i = 0;
  for (n = 0; n < count.length(); n++){
    if (count[n] > 0){
      count_matrix(i,0) = n;
      count_matrix(i,1) = count[n];
      i++;
    }
  }
  
  return count_matrix;
  
}

// [[Rcpp::export]]
Rcpp::IntegerVector regions_to_count_matrix(SEXP corpus, SEXP p_attribute, SEXP registry, SEXP matrix){
  
  Rcpp::IntegerVector ids = regions_to_ids(corpus, p_attribute, registry, matrix);
  Rcpp::IntegerMatrix count_matrix = ids_to_count_matrix(ids);

  return count_matrix;
}


/* wrappers for cl */


// [[Rcpp::export]]
int cwb_attribute_size(SEXP corpus, SEXP attribute, SEXP attribute_type, SEXP registry) {
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


// [[Rcpp::export]]
int cwb_lexicon_size(SEXP corpus, SEXP p_attribute, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int size = cl_max_id(att);
  return( size );
}


// [[Rcpp::export]]
Rcpp::IntegerVector cwb_cpos2struc(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int i;
  int len = cpos.length();
  Rcpp::IntegerVector strucs(len);
  for (i = 0; i < len; i++){
    strucs(i) = cl_cpos2struc(att, cpos(i));
  }
  return( strucs );
}


// [[Rcpp::export]]
Rcpp::StringVector cwb_cpos2str(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
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


// [[Rcpp::export]]
Rcpp::IntegerVector cwb_cpos2id(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int i;
  int len = cpos.length();
  Rcpp::IntegerVector ids(len);
  for (i = 0; i < len; i++){
    ids(i) = cl_cpos2id(att, cpos(i));
  }
  return( ids );
}

// [[Rcpp::export]]
Rcpp::IntegerVector cwb_struc2cpos(SEXP corpus, SEXP s_attribute, SEXP registry, int struc){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  Rcpp::IntegerVector cpos(2);
  int lb, rb;
  cl_struc2cpos(att, struc, &lb, &rb);
  cpos(0) = lb;
  cpos(1) = rb;
  return( cpos );
}


// [[Rcpp::export]]
Rcpp::StringVector cwb_id2str(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector id){
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


// [[Rcpp::export]]
Rcpp::StringVector cwb_struc2str(SEXP corpus, SEXP s_attribute, Rcpp::IntegerVector struc, SEXP registry){
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


// [[Rcpp::export]]
Rcpp::IntegerVector cwb_regex2id(SEXP corpus, SEXP p_attribute, SEXP regex, SEXP registry){
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


// [[Rcpp::export]]
Rcpp::IntegerVector cwb_str2id(SEXP corpus, SEXP p_attribute, Rcpp::StringVector str, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int len = str.length();
  Rcpp::IntegerVector ids(len);
  int i;
  for (i = 0; i < len; i++){
    ids(i) = cl_str2id(att, str(i));
  }
  return( ids );
}

// [[Rcpp::export]]
Rcpp::IntegerVector cwb_id2freq(SEXP corpus, SEXP p_attribute, Rcpp::IntegerVector id, SEXP registry){
  Attribute* att = make_p_attribute(corpus, p_attribute, registry);
  int len = id.length();
  Rcpp::IntegerVector result(len);
  int i;
  for (i = 0; i < len; i++){
    result(i) = cl_id2freq(att, id(i));
  }
  return( result );
}


// [[Rcpp::export]]
Rcpp::IntegerVector cwb_id2cpos(SEXP corpus, SEXP p_attribute, SEXP id, SEXP registry){
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


// [[Rcpp::export]]
int cwb_cpos2lbound(SEXP corpus, SEXP s_attribute, SEXP cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int cpos_int = Rcpp::as<int>(cpos);
  int struc = cl_cpos2struc(att, cpos_int);
  int lb, rb;
  cl_struc2cpos(att, struc, &lb, &rb);
  return( lb );
}

// [[Rcpp::export]]
int cwb_cpos2rbound(SEXP corpus, SEXP s_attribute, SEXP cpos, SEXP registry){
  Attribute* att = make_s_attribute(corpus, s_attribute, registry);
  int cpos_int = Rcpp::as<int>(cpos);
  int struc = cl_cpos2struc(att, cpos_int);
  int lb, rb;
  cl_struc2cpos(att, struc, &lb, &rb);
  return( rb );
}
