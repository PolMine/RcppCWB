#' Rcpp bindings for the corpus library (CL).
#' 
#' The corpus library (CL) is a C library offering low-level access 
#' to CWB-indexed corpora. This package
#' offers wrappers for core functions of the corpus library, and 
#' functions for performance critical tasks in the polmineR package.
#' 
#' The work of the all contributors to the Corpus Workbench is
#' gratefully acknowledged. There is a huge intellectual debt to 
#' Bernard Desgraupes and Sylvain Loiseau, and the package 'rcqp'
#' they developed as a R wrapper for the functionality of the CWB.
#' The intention behind using Rcpp for writing the wrapper library
#' is to make it easier to maintain the package, to provide some 
#' extra functionality, and to make the package work on Windows
#' systems. 
#' 
#' @author Andreas Blaette (andreas.blaette@@uni-due.de)
#' @references CWB (http://cwb.sourceforge.net)
#' @keywords package
#' @docType package
#' @rdname RcppCWB
#' @name RcppCWB
#' @useDynLib RcppCWB, .registration = TRUE
#' @importFrom Rcpp evalCpp
#' @exportPattern "^[[:alpha:]]+"
NULL


#' Higher-level functions using the corpus library (CL).
#' 
#' @param corpus character
#' @param s_attribute character
#' @param p_attribute character
#' @param registry character
#' @param strucs integer
#' @examples
#' Sys.setenv(CORPUS_REGISTRY = system.file(package = "RcppCWB", "extdata", "cwb", "registry"))
#' 
#' decode_s_attribute(corpus = "REUTERS", s_attribute = "places", registry = Sys.getenv("CORPUS_REGISTRY"))
#' 
#' get_count_vector(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = Sys.getenv("CORPUS_REGISTRY")
#'   )
#' get_cbow_matrix(
#'   "REUTERS", p_attribute = "word", matrix = matrix(c(1,10), nrow = 1),
#'   window = 5L, registry = Sys.getenv("CORPUS_REGISTRY")
#'   )
#' regions_to_count_matrix(
#'   "REUTERS", p_attribute = "word", matrix = matrix(c(1,10), ncol = 2),
#'   registry = Sys.getenv("CORPUS_REGISTRY")
#'   )
#' @rdname cl_wrapper_functions
#' @name cl_wrapper_functions
NULL


