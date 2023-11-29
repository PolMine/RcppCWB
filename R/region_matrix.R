#' Get Matrix with Regions for Strucs.
#' 
#' The return value is an `integer` matrix with the left and right corpus
#' positions of the strucs in columns one and two, respectively. For negative
#' struc values in the input vector, the matrix reports `NA` values.
#' 
#' @param corpus A CWB corpus (length-one `character` vector).
#' @param s_attribute A structural attribute (length-one `character` vector).
#' @param strucs Integer vector with strucs.
#' @param registry Registry directory with registry file.
#' @rdname get_region_matrix
#' @export get_region_matrix
#' @return A matrix with integer values indicating left and right corpus
#'   positions (columns 1 and 2, respectively).
#' @examples 
#' y <- get_region_matrix(
#'   corpus = "REUTERS",
#'   s_attribute = "id",
#'   strucs = 0L:5L,
#'   registry = get_tmp_registry()
#' )
get_region_matrix <- function(corpus, s_attribute, strucs, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_strucs(corpus = corpus, s_attribute = s_attribute, strucs = strucs, registry = registry)
  .get_region_matrix(corpus = corpus, s_attribute = s_attribute, strucs = strucs, registry = registry)
}


#' Get IDs and Counts for Region Matrices.
#' 
#' @param corpus a CWB corpus
#' @param p_attribute a positional attribute
#' @param registry registry directory
#' @param matrix a regions matrix
#' @rdname region_matrix_ops
#' @name region_matrix_ops
#' @export region_matrix_to_ids
#' @examples
#' # Scenario 1: Get full text for a subcorpus defined by regions
#' m <- get_region_matrix(
#'   corpus = "REUTERS", s_attribute = "places",
#'   strucs = 4L:5L, registry = get_tmp_registry()
#'   )
#' ids <- region_matrix_to_ids(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = get_tmp_registry(), matrix = m
#'   )
#' tokenstream <- cl_id2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = get_tmp_registry(), id = ids
#'   )
#' txt <- paste(tokenstream, collapse = " ")
#' txt
#' 
#' # Scenario 2: Get data.frame with counts for region matrix
#' y <- region_matrix_to_count_matrix(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = get_tmp_registry(), matrix = m
#'   )
#' df <- as.data.frame(y)
#' colnames(df) <- c("token_id", "count")
#' df[["token"]] <- cl_id2str(
#'   "REUTERS", p_attribute = "word",
#'   registry = get_tmp_registry(), id = df[["token_id"]]
#'   )
#' df[order(df[["count"]], decreasing = TRUE),]
#' head(df)
region_matrix_to_ids <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), matrix){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_p_attribute(p_attribute = p_attribute, corpus = corpus, registry = registry)
  check_region_matrix(region_matrix = matrix)
  .region_matrix_to_ids(corpus = corpus, p_attribute = p_attribute, registry = registry, matrix = matrix)
}


#' @rdname region_matrix_ops
#' @export region_matrix_to_count_matrix
region_matrix_to_count_matrix <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), matrix){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_p_attribute(p_attribute = p_attribute, corpus = corpus, registry = registry)
  stopifnot(is.matrix(matrix))
  .region_matrix_to_count_matrix(corpus = corpus, p_attribute = p_attribute, registry = registry, matrix = matrix)
}

#' @rdname region_matrix_ops
#' @param s_attribute If not `NULL`, a structural attribute (length-one
#'   `character` vector), typically indicating a sentence ("s").
#' @param boundary Structural attribute (length-one `character` vector) that
#'   serves as a boundary and that shall not be transgressed.
#' @param left An `integer` value, number of strucs to move to the left.
#' @param right An `integer` value, number of strucs to move to the right.
#' @export
region_matrix_context <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY"), matrix, p_attribute, s_attribute, boundary, left, right){
  
  if (!is.null(s_attribute)){
    check_s_attribute(s_attribute = s_attribute, corpus = corpus, registry = registry)
  }
  
  if (!is.null(boundary)){
    check_s_attribute(s_attribute = boundary, corpus = corpus, registry = registry)
  }

  .region_matrix_context(
    corpus = corpus,
    registry = registry,
    region_matrix = matrix,
    p_attribute = p_attribute,
    s_attribute = s_attribute,
    boundary = boundary,
    left = as.integer(left),
    right = as.integer(right)
  )
}

#' @details `ranges_to_cpos()` will turn a `matrix` of ranges into an `integer` 
#'   vector with the individual corpus positions covered by the ranges.
#' @rdname region_matrix_ops
#' @param ranges A two-column integer `matrix` of ranges (left and right corpus
#'   positions in first and second column, respectively).
#' @export
ranges_to_cpos <- function(ranges){
  stopifnot(is.integer(ranges), is.matrix(ranges))
  
  if (any(is.na(ranges))){
    drop <- unique(c(which(is.na(ranges[,1])), which(is.na(ranges[,2]))))
    warning(
      sprintf("matrix includes NA values, dropping %d rows", length(drop))
    )
    ranges <- ranges[-drop,]
    # if only one row left, we have a vector to be turned into matrix again
    if (length(ranges) == 2L) ranges <- matrix(ranges, nrow = 1L)
  }
  
  if (nrow(ranges) == 0L) return(integer())
  
  stopifnot(all(ranges[,2] >= ranges[,1]))
  
  if (ncol(ranges) != 2L){
    warning(
      "ranges_to_cpos() requires two-column integer matrix as input ",
      sprintf("but input matrix has %d columns!", ncol(ranges))
    )
  }
  .ranges_to_cpos(ranges)
}