#' Decode Structural Attribute.
#' 
#' Return values for all strucs of a structural attribute.
#' 
#' @param corpus a CWB corpus
#' @param s_attribute a structural attribute
#' @param registry registry directory
#' @export decode_s_attribute
#' @rdname decode_s_attribute
#' @return a character vector
#' @examples 
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' decode_s_attribute(corpus = "REUTERS", s_attribute = "id", registry = registry)
#' Sys.setenv(CORPUS_REGISTRY = registry)
#' decode_s_attribute(
#'   corpus = "REUTERS", s_attribute = "places",
#'   registry = registry
#'   )
decode_s_attribute <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .check_registry(registry = registry)
  .check_corpus(corpus = corpus, registry = registry)
  .check_s_attribute(s_attribute = s_attribute)
  .decode_s_attribute(corpus = corpus, s_attribute = s_attribute, registry = registry)
}


#' Get Vector with Counts for Positional Attribute.
#' 
#' The return value is an integer vector. The length of the vector is the number of 
#' unique tokens in the corpus / the number of unique ids. The order of the counts
#' corresponds to the number of ids.
#' 
#' @param corpus a CWB corpus
#' @param p_attribute a positional attribute
#' @param registry registry directory
#' @return an integer vector
#' @rdname get_count_vector
#' @export get_count_vector
#' @examples 
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' y <- get_count_vector(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = registry
#'   )
#' df <- data.frame(token_id = 0:(length(y) - 1), count = y)
#' df[["token"]] <- cl_id2str(
#'   "REUTERS", p_attribute = "word",
#'   id = df[["token_id"]], registry = registry
#'   )
#' df <- df[,c("token", "token_id", "count")] # reorder columns
#' df <- df[order(df[["count"]], decreasing = TRUE),]
#' head(df)
get_count_vector <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .check_registry(registry)
  .check_corpus(corpus, registry)
  .check_p_attribute(p_attribute)
  .get_count_vector(corpus = corpus, p_attribute = p_attribute, registry = registry)
}

#' Get Matrix with Regions for Strucs.
#' 
#' The return value is an integer matrix with the left and right corpus positions
#' of the strucs in columns one and two, respectively.
#' 
#' @param corpus a CWB corpus
#' @param s_attribute a structural attribute
#' @param strucs strucs
#' @param registry the registry directory
#' @rdname get_region_matrix
#' @export get_region_matrix
#' @return A matrix with integer values indicating left and right corpus positions
#' (columns 1 and 2, respectively).
#' @examples 
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' y <- get_region_matrix(
#'   corpus = "REUTERS", s_attribute = "id",
#'   strucs = 0L:5L, registry = registry
#'   )
get_region_matrix <- function(corpus, s_attribute, strucs, registry = Sys.getenv("CORPUS_REGISTRY")){
  .check_registry(registry)
  .check_corpus(corpus, registry)
  .check_strucs(corpus = corpus, s_attribute = s_attribute, strucs = strucs, registry = registry)
  .get_region_matrix(corpus = corpus, s_attribute = s_attribute, strucs = strucs, registry = registry)
}

#' Get CBOW Matrix.
#' 
#' Get matrix with moving windows. Negative integer values indicate absence of a
#' token at the respective position.
#' 
#' @param corpus a CWB corpus
#' @param p_attribute a positional attribute
#' @param registry the registry directory
#' @param matrix a matrix
#' @param window window size
#' @rdname get_cbow_matrix
#' @export get_cbow_matrix
#' @examples 
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' m <- get_region_matrix(
#'   corpus = "REUTERS", s_attribute = "places",
#'   strucs = 0L:5L, registry = registry
#'   )
#' windowsize <- 3L
#' m2 <- get_cbow_matrix(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = registry, matrix = m, window = windowsize
#'   )
#' colnames(m2) <- c(-windowsize:-1, "node", 1:windowsize)
get_cbow_matrix <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), matrix, window){
  .check_registry(registry)
  .check_corpus(corpus, registry)
  .check_p_attribute(p_attribute)
  .check_region_matrix(matrix)
  stopifnot(window >= 1L)
  .get_cbow_matrix(corpus = corpus, p_attribute = p_attribute, registry = registry, matrix = matrix, window = window)
}


#' Get token IDs for Region Matrix.
#' 
#' @param corpus a CWB corpus
#' @param p_attribute a positional attribute
#' @param registry registry directory
#' @param matrix a regions matrix
#' @rdname region_matrix_to_ids
#' @export region_matrix_to_ids
#' @examples
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' m <- get_region_matrix(
#'   corpus = "REUTERS", s_attribute = "places",
#'   strucs = 4L:5L, registry = registry
#'   )
#' ids <- region_matrix_to_ids(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = registry, matrix = m
#'   )
#' tokenstream <- cl_id2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = registry, id = ids
#'   )
#' txt <- paste(tokenstream, collapse = " ")
#' txt
region_matrix_to_ids <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), matrix){
  .check_registry(registry)
  .check_corpus(corpus, registry)
  .check_p_attribute(p_attribute)
  .check_region_matrix(matrix)
  .region_matrix_to_ids(corpus = corpus, p_attribute = p_attribute, registry = registry, matrix = matrix)
}

#' Perform Count for Vector of IDs.
#' 
#' The return value is a two-column integer matrix. Column one represents the
#' unique ids of the input vector, column two the respective number of
#' occurrences / counts.
#' 
#' @param ids a vector of ids (integer values)
#' @rdname ids_to_count_matrix
#' @examples 
#' ids <- c(1L, 5L, 5L, 7L, 7L, 7L, 7L)
#' ids_to_count_matrix(ids)
#' table(ids) # alternative to get a similar result
ids_to_count_matrix <- function(ids){
  stopifnot(is.integer(ids))
  .ids_to_count_matrix(ids = ids)
}

#' Count for Region Matrix.
#' 
#' Count tokens in a two-column region matrix with left and right corpus positions.
#' The function returns a two-column matrix with the token ids (column 1) and the number
#' of occurrencs / counts (column 2).
#' 
#' @param corpus a CWB corpus
#' @param p_attribute a positional attribute
#' @param registry registry directory
#' @param matrix a region matrix
#' @rdname region_matrix_to_count_matrix
#' @examples 
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' m <- get_region_matrix(
#'   corpus = "REUTERS", s_attribute = "places",
#'   strucs = 5L:5L, registry = registry
#'   )
#' y <- region_matrix_to_count_matrix(
#'   corpus = "REUTERS", p_attribute = "word",
#'   registry = registry, matrix = m
#'   )
#' df <- as.data.frame(y)
#' colnames(df) <- c("token_id", "count")
#' df[["token"]] <- cl_id2str(
#'   "REUTERS", p_attribute = "word",
#'   registry = registry, id = df[["token_id"]]
#'   )
#' df[order(df[["count"]], decreasing = TRUE),]
#' head(df)
region_matrix_to_count_matrix <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), matrix){
  .check_registry(registry)
  .check_corpus(corpus, registry)
  .check_p_attribute(p_attribute)
  stopifnot(is.matrix(matrix))
  .region_matrix_to_count_matrix(corpus = corpus, p_attribute = p_attribute, registry = registry, matrix = matrix)
}

