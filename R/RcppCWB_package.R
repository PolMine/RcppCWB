#' Rcpp bindings for the corpus library (CL).
#' 
#' The corpus library (CL) is a C library offering low-level access 
#' to CWB-indexed corpora. This package
#' offers wrappers for core functions of the corpus library, and 
#' functions for performance critial tasks in the polmineR package.
#' 
#' @author Andreas Blaette (andreas.blaette@@uni-due.de)
#' @references http://polmine.sowi.uni-due.de
#' @keywords package
#' @docType package
#' @rdname RcppCWB
#' @name RcppCWB
#' @useDynLib RcppCWB, .registration = TRUE
#' @importFrom Rcpp evalCpp
#' @exportPattern "^[[:alpha:]]+"
NULL


#' Higher-level Functions.
#' 
#' @param corpus character
#' @param s_attribute character
#' @param p_attribute character
#' @param registry character
#' @param strucs integer
#' @examples
#' Sys.setenv(CORPUS_REGISTRY = system.file(package = "RcppCWB", "extdata", "cwb", "registry"))
#' 
#' decode_s_attribute("REUTERS", "places", registry = Sys.getenv("CORPUS_REGISTRY"))
#' get_count_vector(
#'   "REUTERS", p_attribute = "word",
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
#' @section Functions:
#' \describe{
#'   \item{\code{decode_s_attribute(corpus, s_attribute, registry)}}{decode s-attributes}
#'   \item{\code{get_count_vector(corpus, p_attribute, registry)}}{count tokens in corpus}
#'   \item{\code{get_region_matrix(corpus, s_attribute, strucs, registry)}}{get region matrix for strucs}
#'   \item{\code{regions_to_count_matrix(corpus, p_attribute, registry, matrix)}}{get matrix with ids and counts}
#'   \item{\code{get_cbow_matrix(corpus, p_attribute, registry = registry, matrix, window = window)}}{get matrix with CBOW}
#'   \item{\code{regions_to_count_matrix(corpus, p_attribute, registry, matrix)}}{get counts for regions}
#'   \item{\code{regions_to_ids(corpus, p_attribute, registry, matrix)}}{get ids for regions}
#' }
#' @aliases get_count_vector decode_s_attribute get_region_matrix regions_to_count_matrix
#'   get_cbow_matrix regions_to_count_matrix regions_to_ids
#' @export get_count_vector decode_s_attribute get_region_matrix regions_to_count_matrix
#' @export get_cbow_matrix regions_to_count_matrix regions_to_ids
#' @rdname cl_wrapper_functions
#' @name cl_wrapper_functions
NULL


#' Functions of the Corpus Library (CL).
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param attribute name of a s- or p-attribute
#' @param attribute_type either "p" or "s"
#' @param registry path to the registry directory
#' @param p_attribute a p-attribute
#' @param s_attribute a s-attribute
#' @param cpos corpus positions (integer vector)
#' @param struc strucs (integer vector)
#' @param id id of a token
#' @param regex a regular expression
#' @param str a character string
#' @export attribute_size
#' @rdname cl_functions
#' @section Functions:
#' \describe{
#'   \item{\code{attribute_size(corpus, attribute, attribute_type, registry)}}{get size of an attribute}
#'   \item{\code{lexicon_size(corpus, p_attribute, registry)}}{get lexicon size}
#'   \item{\code{cpos2struc(corpus, s_attribute, cpos, registry)}}{turn corpus position to struc}
#'   \item{\code{cpos2str(corpus, p_attribute, registry, cpos)}}{get string value for corpus position}
#'   \item{\code{cpos2id(corpus, p_attribute, registry, cpos)}}{get token id for corpus position}
#'   \item{\code{struc2cpos(corpus, s_attribute, registry, struc)}}{turn struc to cpos}
#'   \item{\code{id2str(corpus, p_attribute, registry, id)}}{get string value for id}
#'   \item{\code{struc2str(corpus, s_attribute, struc, registry)}}{get string value for struc}
#'   \item{\code{regex2id(corpus, p_attribute, regex, registry)}}{get ids matching a regex}
#'   \item{\code{str2id(corpus, p_attribute, str, registry)}}{get id for string}
#'   \item{\code{id2freq(corpus, p_attribute, id, registry)}}{get frequency for id}
#'   \item{\code{id2cpos(corpus, p_attribute, id, registry)}}{get corpus positions for id}
#'   \item{\code{cpos2lbound(corpus, s_attribute, cpos, registry)}}{get left boundary of a struc for a corpus position}
#'   \item{\code{cpos2rbound(corpus, s_attribute, cpos, registry)}}{get right boundary of a struc for a corpus position}
#' }
#' @aliases attribute_size lexicon_size cpos2struc cpos2str cpos2id 
#'   struc2cpos id2str struc2str regex2id str2id id2freq id2cpos
#'   cpos2lbound cpos2rbound
#' @export attribute_size lexicon_size cpos2struc cpos2str cpos2id
#' @export struc2cpos id2str struc2str regex2id str2id id2freq id2cpos 
#' @export cpos2lbound cpos2rbound
#' @name cl_functions
NULL