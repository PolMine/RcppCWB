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


