#' Check Input to Rcpp Functions.
#' 
#' A set of functions to check whether the input values to the Rcpp
#' wrappers for the C functions of the Corpus Workbench potentially causing
#' crashes are valid. These auxiliary functions are called by the cl_ and cqp_
#' functions.
#' @param corpus name of a CWB corpus
#' @param s_attribute a structural attribute
#' @param p_attribute a positional attribute
#' @param strucs strucs (indices of structural attributes)
#' @param cpos vector of corpus positions
#' @param id id (encoded p-attribute), integer value
#' @param query a CQP query
#' @param region_matrix a region matrix
#' @param registry path to registry directory
#' @rdname checks
#' @name check
#' @export check_registry
check_registry <- function(registry){
  if (length(registry) != 1)
    stop("registry needs to be a character vector length 1")
  if (!is.character(registry))
    stop("registry needs to be a character vector (is.character not TRUE)")
  if(!file.exists(registry))
    stop("the registry directory provided does not exist")
  if (!file.info(registry)$isdir)
    stop("registry exists, but is not a directory")
  return( TRUE )
}

#' @rdname checks
#' @export check_corpus
check_corpus <- function(corpus, registry){
  if (length(corpus) != 1)
    stop("corpus needs to be a vector of length 1")
  if (!is.character(corpus))
    stop("corpus needs to be a character vector")
  if (!tolower(corpus) %in% list.files(registry))
    stop("no file describing corpus in registry directory: Does corpus exists / check whether there is a typo.")
  return( TRUE )
}

#' @export check_s_attribute
#' @rdname checks
check_s_attribute <- function(s_attribute, corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  if (length(s_attribute) != 1)
    stop("s_attribute needs to be a length 1 vector")
  if (!is.character(s_attribute))
    stop("s_attribute needs to be a character vector")
  registry_file <- readLines(file.path(registry, tolower(corpus)))
  sattr_lines <- registry_file[grep("^STRUCTURE", registry_file)]
  sattrs_declared <- gsub("^STRUCTURE\\s+(.*?)\\s*.*?", "\\1", sattr_lines)
  if (!s_attribute %in% sattrs_declared)
    stop(sprintf("s_attribute '%s' is not declared in registry file of corpus '%s'", s_attribute, corpus))
  return( TRUE )
}

#' @export check_p_attribute
#' @rdname checks
check_p_attribute <- function(p_attribute, corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  if (length(p_attribute) != 1)
    stop("p_attribute needs to be a length 1 vector")
  if (!is.character(p_attribute))
    stop("p_attribute needs to be a character vector")
  registry_file <- readLines(file.path(registry, tolower(corpus)))
  pattr_lines <- registry_file[grep("^ATTRIBUTE", registry_file)]
  pattrs_declared <- gsub("^ATTRIBUTE\\s+(.*?)\\s*.*?", "\\1", pattr_lines)
  if (!p_attribute %in% pattrs_declared)
    stop(sprintf("p_attribute '%s' is not declared in registry file of corpus '%s'", p_attribute, corpus))
  return( TRUE )
}

#' @export check_strucs
#' @rdname checks
check_strucs <- function(corpus, s_attribute, strucs, registry){
  if (!is.numeric(strucs))
    stop("strucs needs to be a integer vector")
  if (min(strucs) < 0)
    stop("all values of vector strucs need to be >= 0")
  if (max(strucs) > (cl_attribute_size(corpus, attribute = s_attribute, "s", registry = registry) - 1))
    stop("highest value of strucs may not be larger than size of structural attribute")
  return( TRUE )
}

#' @export check_region_matrix
#' @rdname checks
check_region_matrix <- function(region_matrix){
  if (!all(region_matrix[,2] - region_matrix[,1] >= 0))
    stop("check region matrix - all values of column 2 need to be equal or higher than values of column one. ",
         "This is not TRUE.")
  return( TRUE )
}

#' @export check_cqp_query
#' @rdname checks
check_cqp_query <- function(query){
  if (!substr(query, start = length(query), stop = length(query)) == ";"){
    return( paste0(query, ";", sep = "") )
  } else {
    return( query )
  }
}

#' @export check_cpos
#' @rdname checks
check_cpos <- function(corpus, p_attribute = "word", cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  attr_max <- cl_attribute_size(corpus = corpus, attribute = p_attribute, attribute_type = "p", registry = registry)
  if (min(cpos) < 0){
    stop("all corpus positions (cpos) need to be >= 0, not TRUE")
  }
  if (max(cpos) > attr_max){
    stop("all corpus positions (cpos) need to be <= attribute size, not TRUE")
  }
  return( TRUE )
}

#' @export check_id
#' @rdname checks
check_id <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  lexicon_size <- cl_lexicon_size(corpus = corpus, p_attribute = p_attribute, registry = registry)
  if (min(id) < 0){
    stop("all corpus positions (cpos) need to be >= 0, not TRUE")
  }
  if (max(id) > lexicon_size){
    stop("all corpus positions (cpos) need to be <= attribute size, not TRUE")
  }
  return( TRUE )
}
