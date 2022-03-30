#' Explore XML structure of CWB corpus
#' 
#' The data format of the Corpus Workbench (CWB) allows nested XML as import
#' data. Auxiliary functions assist detecting whether two structural attributes
#' are nested or at the same level (i.e. defining the same regions).
#' 
#' @details `s_attr_is_descendent()` will evaluate whether s_attribute `x` is
#'   a child of s_attribute `y`. The return value is `TRUE` (a single `logical`
#'   value)  if all regions defined by `x` are within the regions defined by `y`.
#'   If not, `FALSE` is returned. The return values is also `FALSE` if all regions
#'   of `x` and `y` are idential. Attributes will be siblings in this case, 
#'   and not in an ancestor-sibling relationship.
#' @examples 
#' s_attr_is_descendent("id", "places", corpus = "REUTERS", registry = get_tmp_registry())
#' @export
#' @rdname xml
#' @param x A structural attribute, stated as length-one `character` vector.
#' @param y Another structural attribute, stated as length-one `character` vector.
#' @param corpus A corpus ID (length-one `character` vector).
#' @param sample An `integer` vector with a sample number of strucs to evaluate.
#'   Evaluating only a sample may be an efficient choice for large corpora. If `NULL`
#'   (default), all strucs are evaluated.
#' @param registry The directory with the registry file for the corpus.
s_attr_is_descendent <- function(x, y, corpus, registry = Sys.getenv("CORPUS_REGISTRY"), sample = NULL){
  
  if (!is.null(sample)){
    sample <- as.integer(sample)
    if (is.na(sample)){
      warning(
        "Argument 'sample' is required to be an integer vector ",
        "or a numeric vector that can be coerced to integer vector. ",
        "Coercion not possible (yields NA)."
      )
    }
    n_descendents <- cl_attribute_size(corpus = corpus, attribute = x, attribute_type = "s", registry = registry)
    struc_sample <- sample.int(n = n_descendents - 1L, size = sample)
    descendent_regions <- get_region_matrix(corpus = corpus, s_attribute = x, strucs = struc_sample, registry = registry)
  } else {
    descendent_regions <- s_attr_regions(s_attr = x, corpus = corpus, registry = registry)
  }
  
  s <- cl_cpos2struc(corpus = corpus, s_attribute = y, cpos = descendent_regions[,1], registry = registry)
  ancestor_regions <- get_region_matrix(corpus = corpus, s_attribute = y, strucs = s, registry = registry)

  if (identical(descendent_regions, ancestor_regions)){
    FALSE
  } else if (all(descendent_regions[,1] >= ancestor_regions[,1]) && all(descendent_regions[,2] <= ancestor_regions[,2])){
    TRUE
  } else {
    FALSE
  }
}

#' @details `s_attr_is_sibling()` will test whether the regions defined for
#'   structural attribute `x` and structural attribute `y` are identical. If
#'   yes, `TRUE` is returned, assuming that both attributes are at the same
#'   level (siblings). If not, `FALSE` is returned.
#' @rdname xml
#' @export
#' @examples
#' s_attr_is_sibling(x = "id", y = "places", corpus = "REUTERS", registry = get_tmp_registry())
s_attr_is_sibling <- function(x, y, corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  
  x_regions <- s_attr_regions(s_attr = x, corpus = corpus, registry = registry)
  y_regions <- s_attr_regions(s_attr = y, corpus = corpus, registry = registry)
  
  if (identical(x_regions, y_regions)) TRUE else FALSE
}


#' @details `s_attr_relationship()` will return `0` if s-attributes `x` and `y`
#'   are siblings in the sense that they define identical regions. The return
#'   value is `0` if `x` is an ancestor of `y` and `1` if `x` is a descencdent
#'   of `y`.
#' @rdname xml
#' @export
#' @examples
#' s_attr_is_sibling(x = "id", y = "places", corpus = "REUTERS", registry = get_tmp_registry())
s_attr_relationship <- function(x, y, corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  
  data_dir <- corpus_data_dir(corpus = corpus, registry = registry)
  x_regions <- s_attr_regions(s_attr = x, corpus = corpus, data_dir = data_dir, registry = registry)
  y_regions <- s_attr_regions(s_attr = y, corpus = corpus, data_dir = data_dir, registry = registry)
  
  if (identical(x_regions, y_regions)) return(0L)

  y_strucs <- cl_cpos2struc(corpus = corpus, s_attribute = y, cpos = x_regions[,1], registry = registry)
  y_regions <- get_region_matrix(corpus = corpus, s_attribute = y, strucs = y_strucs, registry = registry)
  
  if (all(x_regions[,1] >= y_regions[,1]) && all(x_regions[,2] <= y_regions[,2])){
    return(-1L)
  } else {
    return(1L)
  }
  NA_integer_
}
