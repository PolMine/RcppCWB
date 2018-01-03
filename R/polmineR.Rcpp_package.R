#' Rcpp extension for polmineR.
#' 
#' Based on the functionality of the cl corpus library, this package
#' offers wrappers for core functions of the corpus library, and 
#' functions for performance critial tasks in the polmineR package.
#' 
#' @author Andreas Blaette (andreas.blaette@@uni-due.de)
#' @references http://polmine.sowi.uni-due.de
#' @keywords package
#' @docType package
#' @rdname polmineR.Rcpp
#' @name polmineR.Rcpp
#' @useDynLib polmineR.Rcpp, .registration = TRUE
#' @aliases attribute_size lexicon_size cpos2struc cpos2str cpos2id struc2cpos
#' id2str struc2str regex2id str2id id2freq id2cpos cpos2lbound cpos2rbound
#' @importFrom Rcpp evalCpp
#' @exportPattern "^[[:alpha:]]+"
NULL

#' Decode s-attributes
#' 
#' Proof of concept - not faster than rcqp::cqi_struc2str.
#' 
#' @param corpus character
#' @param sAttribute character
#' @param registry character
#' @export decodeSAttribute
#' @aliases decode_s_attribute
decodeSAttribute <- function(corpus, sAttribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  decode_s_attribute(corpus, sAttribute, registry)
}

#' Count tokens in corpus.
#' 
#' Substantially faster than cqi_cpos2str/tabulate.
#' 
#' @param corpus character
#' @param pAttribute character
#' @param registry character
#' @export getCountVector
#' @aliases get_count_vector
getCountVector <- function(corpus, pAttribute = "word", registry = Sys.getenv("CORPUS_REGISTRY")){
  get_count_vector(corpus, pAttribute, registry)
}


#' Get Matrix with Regions.
#' 
#' Substantially faster than lapply(cqi_struc2cpos).
#' 
#' @param corpus character
#' @param sAttribute character
#' @param strucs integer
#' @param registry character
#' @export getRegionMatrix
#' @aliases get_region_matrix
getRegionMatrix <- function(corpus, sAttribute = "word", strucs, registry = Sys.getenv("CORPUS_REGISTRY")){
  get_region_matrix(corpus = corpus, s_attribute = sAttribute, strucs = strucs, registry = registry)
}

#' Get matrix with CBOW.
#' 
#' Substantially faster than lapply(cqi_struc2cpos).
#' 
#' @param corpus character
#' @param pAttribute character
#' @param registry character
#' @param matrix a matrix with regions
#' @param window intger, window size
#' @export getCbowMatrix
#' @aliases get_cbow_matrix
getCbowMatrix <- function(corpus, pAttribute = "word", registry = Sys.getenv("CORPUS_REGISTRY"), matrix, window){
  get_cbow_matrix(corpus = corpus, p_attribute = pAttribute, registry = registry, matrix = matrix, window = window)
}


#' Get matrix with ids and counts.
#' 
#' Reduces time by one third.
#' 
#' @param corpus character
#' @param pAttribute character
#' @param registry character
#' @param matrix a matrix with regions
#' @export regionsToCountMatrix
#' @aliases regions_to_count_matrix
regionsToCountMatrix <- function(corpus, pAttribute = "word", registry = Sys.getenv("CORPUS_REGISTRY"), matrix){
  regions_to_count_matrix(corpus = corpus, p_attribute = pAttribute, registry = registry, matrix = matrix)
}

#' Turn regions into ids.
#' 
#' @param corpus character
#' @param pAttribute character
#' @param registry character
#' @param matrix matrix (2 columns) with regions
#' @export regionsToIds
#' @aliases regions_to_ids
regionsToIds <- function(corpus, pAttribute, registry, matrix){
  regions_to_ids(corpus = corpus, p_attribute = pAttribute, registry = registry, matrix = matrix)
}
