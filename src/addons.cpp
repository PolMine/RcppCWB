extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  #include "cl.h"
  #include "cwb/cl/cwb-globals.h" 
  #include <pcre.h>
}

#include <Rcpp.h>

/* short quasi-header */
Attribute* make_s_attribute(SEXP corpus, SEXP s_attribute, SEXP registry);
Attribute* make_p_attribute(SEXP corpus, SEXP p_attribute, SEXP registry);

  
int region_matrix_to_size(Rcpp::IntegerMatrix matrix){
  int n;
  int size = 0;
  for (n = 0; n < matrix.nrow(); n++){
    size += matrix(n,1) - matrix(n,0) + 1;
  }
  return size;
}


// [[Rcpp::export(name=".decode_s_attribute")]]
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


// [[Rcpp::export(name=".get_count_vector")]]
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


// [[Rcpp::export(name=".get_region_matrix")]]
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


// [[Rcpp::export(name=".get_cbow_matrix")]]
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


// [[Rcpp::export(name=".region_matrix_to_ids")]]
Rcpp::IntegerVector region_matrix_to_ids(SEXP corpus, SEXP p_attribute, SEXP registry, SEXP matrix){
  
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


// [[Rcpp::export(name=".ranges_to_cpos")]]
Rcpp::IntegerVector ranges_to_cpos(SEXP ranges){
  
  Rcpp::IntegerMatrix m(ranges);
  int size = region_matrix_to_size(m);
  Rcpp::IntegerVector result(size);
  
  int n, cpos;
  int i = 0;
  for (n = 0; n < m.nrow(); n++){
    for (cpos = m(n,0); cpos <= m(n,1); cpos++){
      result(i) = cpos;
      i++;
    }
  }
  
  return result;
}


// [[Rcpp::export(name=".ids_to_count_matrix")]]
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


// [[Rcpp::export(name=".region_matrix_to_count_matrix")]]
Rcpp::IntegerVector region_matrix_to_count_matrix(SEXP corpus, SEXP p_attribute, SEXP registry, SEXP matrix){
  
  Rcpp::IntegerVector ids = region_matrix_to_ids(corpus, p_attribute, registry, matrix);
  Rcpp::IntegerMatrix count_matrix = ids_to_count_matrix(ids);

  return count_matrix;
}


// [[Rcpp::export(name=".region_matrix_context")]]
Rcpp::IntegerMatrix region_matrix_context(SEXP corpus, SEXP registry, Rcpp::IntegerMatrix region_matrix, SEXP p_attribute, SEXP s_attribute, SEXP boundary, int left, int right){
  
  int i, cpos, size;
  int ncpos = 0;
  int lb, rb;
  Rcpp::IntegerMatrix context_matrix(region_matrix.nrow(), 2);
  
  Attribute* p_attr = make_p_attribute(corpus, p_attribute, registry);

  if (boundary == R_NilValue){
    
    if (s_attribute == R_NilValue){
      
      size = cl_max_cpos(p_attr);
      int cpos_left, cpos_right;
      
      for (i = 0; i < region_matrix.nrow(); i++){
        
        cpos_left = region_matrix(i,0) - left;
        if (cpos_left >= 0){
          context_matrix(i,0) = cpos_left;
        } else {
          context_matrix(i,0) = 0;
        }
        ncpos += region_matrix(i,0) - context_matrix(i,0);
        
        cpos_right = region_matrix(i,1) + right;
        if (cpos_right < size){
          context_matrix(i,1) = cpos_right;
        } else {
          context_matrix(i,1) = size - 1;
        }
        ncpos += context_matrix(i,1) - region_matrix(i,1);
        ncpos += region_matrix(i,1) - region_matrix(i,0) + 1;
      }
      
    } else {
      
      Attribute* s_attr = make_s_attribute(corpus, s_attribute, registry);
      size = cl_max_struc(s_attr);
      int struc_node, struc_left, struc_right;
      
      for (i = 0; i < region_matrix.nrow(); i++){
        
        struc_node = cl_cpos2struc(s_attr, region_matrix(i, 0));

        struc_left = struc_node - left;
        if (struc_left >= 0){
          
          cl_struc2cpos(s_attr, struc_left, &lb, &rb);

          if (lb == region_matrix(i,0)){
            context_matrix(i,0) = NA_INTEGER;
          } else {
            context_matrix(i,0) = lb;
            ncpos += region_matrix(i,0) - lb;
          }
        } else {
          context_matrix(i,0) = NA_INTEGER;
        }
        
        struc_right = struc_node + right;
        if (struc_right <= size){
          
          cl_struc2cpos(s_attr, struc_right, &lb, &rb);

          if (rb == region_matrix(i,1)){
            context_matrix(i,1) = NA_INTEGER;
          } else {
            context_matrix(i,1) = rb;
            ncpos += rb - region_matrix(i,1);
          }
        } else {
          context_matrix(i,3) = NA_INTEGER;
        }
        
        ncpos += region_matrix(i,1) - region_matrix(i,0) + 1;
      }
      
    }
    
  } else {
    
    int struc_boundary_node, lb_boundary, rb_boundary;
    Attribute* limit = make_s_attribute(corpus, boundary, registry);

    if (s_attribute == R_NilValue){
      
      SEXP default_p_attr = Rf_mkChar(CWB_DEFAULT_ATT_NAME);
      Attribute* p_attr = make_p_attribute(corpus, default_p_attr, registry);
      size = cl_max_cpos(p_attr);
      int cpos_left, cpos_right;
      
      for (i = 0; i < region_matrix.nrow(); i++){
        
        cpos_left = region_matrix(i,0) - left;
        
        struc_boundary_node = cl_cpos2struc(limit, region_matrix(i, 0));
        if (cpos_left >= 0){
          cl_struc2cpos(limit, struc_boundary_node, &lb_boundary, &rb_boundary);
          if (lb_boundary == region_matrix(i,0)){
            context_matrix(i,0) = NA_INTEGER;
          } else if (lb_boundary > cpos_left){
            context_matrix(i,0) = lb_boundary;
            ncpos += region_matrix(i,0) - lb_boundary;
          } else {
            context_matrix(i,0) = cpos_left;
            ncpos += region_matrix(i,0) - cpos_left;
          }
        } else {
          context_matrix(i,0) = NA_INTEGER;
        }
        
        cpos_right = region_matrix(i,1) + right;
        if (cpos_right < size){
          cl_struc2cpos(limit, struc_boundary_node, &lb_boundary, &rb_boundary);
          if (rb_boundary == region_matrix(i,1)){
            context_matrix(i,1) = NA_INTEGER;
          } else if (rb_boundary < cpos_right){
            context_matrix(i,1) = rb_boundary;
            ncpos += rb_boundary - region_matrix(i,1);
          } else {
            context_matrix(i,1) = cpos_right;
            ncpos += cpos_right - region_matrix(i,1);
          }
        } else {
          context_matrix(i,1) = NA_INTEGER;
        }
        ncpos += region_matrix(i,1) - region_matrix(i,0) + 1;
      }
      
    } else {
      
      Attribute* s_attr = make_s_attribute(corpus, s_attribute, registry);
      size = cl_max_struc(s_attr);
      int struc_node, struc_left, struc_right;
      
      for (i = 0; i < region_matrix.nrow(); i++){
        
        struc_node = cl_cpos2struc(s_attr, region_matrix(i, 0));
        struc_boundary_node = cl_cpos2struc(limit, region_matrix(i, 0));
        
        struc_left = struc_node - left;
        if (struc_left >= 0){
          
          cl_struc2cpos(s_attr, struc_left, &lb, &rb);
          cl_struc2cpos(limit, struc_boundary_node, &lb_boundary, &rb_boundary);
          
          if (lb_boundary == region_matrix(i,0)){
            context_matrix(i,0) = NA_INTEGER;
          } else if (lb_boundary > lb){
            context_matrix(i,0) = lb_boundary;
            ncpos += region_matrix(i,0) - lb_boundary;
          } else {
            context_matrix(i,0) = lb;
            ncpos += region_matrix(i,0) - lb;
          }
        } else {
          context_matrix(i,0) = NA_INTEGER;
        }
        
        struc_right = struc_node + right;
        if (struc_right <= size){
          
          cl_struc2cpos(s_attr, struc_right, &lb, &rb);
          cl_struc2cpos(limit, struc_boundary_node, &lb_boundary, &rb_boundary);
          
          if (rb_boundary == region_matrix(i,1)){
            context_matrix(i,1) = NA_INTEGER;
          } else if (rb_boundary < rb){
            context_matrix(i,1) = rb_boundary;
            ncpos += rb_boundary - region_matrix(i,1);
          } else {
            context_matrix(i,1) = rb;
            ncpos += rb - region_matrix(i,1);
          }
        } else {
          context_matrix(i,3) = NA_INTEGER;
        }
        ncpos += region_matrix(i,1) - region_matrix(i,0) + 1;
      }
    }
  }

  Rcpp::IntegerMatrix cpos_matrix(ncpos, 4);
  int k = 0;
  
  for (i = 0; i < region_matrix.nrow(); i++){
    if (context_matrix(i,0) != NA_INTEGER){
      for (cpos = context_matrix(i,0); cpos < region_matrix(i,0); cpos++){
        cpos_matrix(k,0) = cpos - region_matrix(i,0);
        cpos_matrix(k,1) = cpos;
        cpos_matrix(k,2) = i + 1;
        cpos_matrix(k,3) = cl_cpos2id(p_attr, cpos);
        k++;
      }
    }
    for (cpos = region_matrix(i,0); cpos <= region_matrix(i,1); cpos++){
      cpos_matrix(k,0) = 0;
      cpos_matrix(k,1) = cpos;
      cpos_matrix(k,2) = i + 1;
      cpos_matrix(k,3) = cl_cpos2id(p_attr, cpos);
      k++;
    }
    if (context_matrix(i,1) != NA_INTEGER){
      for (cpos = region_matrix(i,1) + 1; cpos <= context_matrix(i,1); cpos++){
        cpos_matrix(k,0) = cpos - region_matrix(i,1);
        cpos_matrix(k,1) = cpos;
        cpos_matrix(k,2) = i + 1;
        cpos_matrix(k,3) = cl_cpos2id(p_attr, cpos);
        k++;
      }
    }
  }

  return cpos_matrix;
}
