#' Check registry directory
#' 
#' This utility functions checks whether registry is a character vector length
#' 1, exists and is a directory. The function offers a robustness checks for all
#' C functions called.
#' 
#' @param registry path to registry directory
#' @noRd
.check_registry <- function(registry){
  if (length(registry) != 1)
    stop("registry needs to be a character vector length 1")
  if (!is.character(registry))
    stop("registry needs to be a character vector (is.character not TRUE)")
  if(!file.exists(registry))
    stop("the registry directory provided does not exist")
  if(!file.info(registry)$isdir)
    stop("registry exists, but is not a directory")
  return( TRUE )
}

#' Check corpus
#' 
#' This utility functions checks whether corpus is a character vector length 1,
#' exists and that a file with the respective name exists. The function offers a
#' robustness checks for all C functions called.
#' 
#' @param registry path to registry directory
#' @noRd
.check_corpus <- function(corpus, registry){
  if (length(corpus) != 1)
    stop("corpus needs to be a vector of length 1")
  if (!is.character(corpus))
    stop("corpus needs to be a character vector")
  if (!tolower(corpus) %in% list.files(registry))
    stop("no file describing corpus in registry directory: Does corpus exists / check whether there is a typo.")
  return( TRUE )
}

.check_s_attribute <- function(s_attribute){
  if (length(s_attribute) != 1)
    stop("s_attribute needs to be a length 1 vector")
  if (!is.character(s_attribute))
    stop("s_attribute needs to be a character vector")
  # missing test! whether s_attribute is available
  return( TRUE )
}

.check_p_attribute <- function(p_attribute){
  if (length(p_attribute) != 1)
    stop("p_attribute needs to be a length 1 vector")
  if (!is.character(p_attribute))
    stop("p_attribute needs to be a character vector")
  return( FALSE )
  # missing test! whether s_attribute is available
}

.check_strucs <- function(corpus, s_attribute, strucs, registry){
  if (!is.integer(strucs))
    stop("strucs needs to be a integer vector")
  if (min(strucs) < 0)
    stop("all values of vector strucs need to be >= 0")
  if (max(strucs) > (cl_attribute_size(corpus, attribute = s_attribute, "s", registry = registry) - 1))
    stop("highest value of strucs may not be larger than size of structural attribute")
  return( TRUE )
}

.check_region_matrix <- function(matrix){
  if (!all(matrix[,2] - matrix[,1] >= 0))
    stop("check region matrix - all values of column 2 need to be equal or higher than values of column one. ",
         "This is not TRUE.")
}

.check_cqp_query <- function(query){
  if (!substr(query, start = length(query), stop = length(query)) == ";"){
    return( paste0(query, ";", sep = "") )
  } else {
    return( query )
  }
}
