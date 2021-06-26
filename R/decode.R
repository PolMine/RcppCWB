#' Decode Structural Attribute.
#' 
#' Get \code{data.frame} with left and right corpus positions (cpos) for structural
#' attributes and values.
#' 
#' Two approaches are implemented: A pure R solution will decode the files directly in 
#' the directory specified by \code{data_dir}. An implementation using Rcpp will use the
#' registry file for \code{corpus} to find the data directory.
#' 
#' @param corpus A CWB corpus (ID in upper case).
#' @param s_attribute A structural attribute (length 1 `character` vector).
#' @param data_dir The data directory where the binary files of the corpus are
#'   stored.
#' @param encoding Encoding of the values ("latin-1" or "utf-8")
#' @param registry The CWB registry directory.
#' @param method A length-one `character` vector, whether to use "R" or "Rcpp"
#'   implementation for decoding structural attribute.
#' @return A \code{data.frame} with three columns. Column \code{cpos_left} are
#'   the start corpus positions of a structural annotation, \code{cpos_right}
#'   the end corpus positions. Column \code{value} is the value of the
#'   annotation.
#' @export s_attribute_decode
#' @rdname s_attribute_decode
#' @examples 
#' registry <- if (!check_pkg_registry_files()) use_tmp_registry() else get_pkg_registry()
#' Sys.setenv(CORPUS_REGISTRY = registry)
#' 
#' # pure R implementation (Rcpp implementation fails on Windows in vanilla mode)
#' b <- s_attribute_decode(
#'   data_dir = system.file(package = "RcppCWB", "extdata", "cwb", "indexed_corpora", "reuters"),
#'   s_attribute = "places", method = "R"
#' )
#' 
#' # Using Rcpp wrappers for CWB C code
#' b <- s_attribute_decode(
#'   corpus = "REUTERS",
#'   data_dir = system.file(package = "RcppCWB", "extdata", "cwb", "indexed_corpora", "reuters"),
#'   s_attribute = "places",
#'   method = "Rcpp"
#' )
s_attribute_decode <- function(corpus, data_dir, s_attribute, encoding = NULL, registry = Sys.getenv("CORPUS_REGISTRY"), method = c("R", "Rcpp")){
  
  if (!is.character(method)) stop("Argument 'method' needs to be a character vector.")
  if (length(method) != 1L) stop("Argument 'method' needs to be a length 1 vector.")
  if (!method %in% c("Rcpp", "R")) stop("Argument 'method' needs to be either 'R' or 'Rcpp'.")
  
  if (method == "R"){
    
    if (missing(data_dir)) stop("data_dir needs to be specified to use R method")
    avs_file <- file.path(data_dir, paste(s_attribute, "avs", sep = ".")) # attribute values
    avx_file <- file.path(data_dir, paste(s_attribute, "avx", sep = ".")) # attribute value index
    rng_file <- file.path(data_dir, paste(s_attribute, "rng", sep = ".")) # ranges
    
    avs_file_size <- file.info(avs_file)[["size"]]
    avx_file_size <- file.info(avx_file)[["size"]]
    rng_file_size <- file.info(rng_file)[["size"]]
    
    avs <- readBin(con = avs_file, what = character(), n = avs_file_size)
    rng <- readBin(rng_file, what = integer(), size = 4L, n = rng_file_size / 4, endian = "big")
    avx <- readBin(avx_file, what = integer(), size = 4L, n = avx_file_size / 4, endian = "big")
    
    if (!is.null(encoding)) Encoding(avs) <- encoding
    region_matrix <- matrix(rng, ncol = 2, byrow = TRUE)
    avx_matrix <- matrix(avx, ncol = 2, byrow = TRUE)
    
    colnames(region_matrix) <- c("cpos_left", "cpos_right")
    
    values <- avs[match(avx_matrix[,2], unique(avx_matrix[,2]))]
    
    df <- data.frame(
      cpos_left = region_matrix[,1],
      cpos_right = region_matrix[,2],
      value = values,
      stringsAsFactors = FALSE
    )
  } else if (method == "Rcpp"){
    
    if (missing(corpus)) stop("corpus needs to be specified to use 'Rcpp' method")
    check_registry(registry = registry)
    check_corpus(corpus = corpus, registry = registry)
    check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
    
    values <- .decode_s_attribute(corpus = corpus, s_attribute = s_attribute, registry = registry)

    s_attr_size <- cl_attribute_size(
      corpus = corpus,
      attribute = s_attribute,
      attribute_type = "s",
      registry = registry
    )
    
    region_matrix <- get_region_matrix(
      corpus = corpus, 
      s_attribute = s_attribute,
      strucs = 0L:(s_attr_size - 1L), 
      registry = registry
    )

    df <- data.frame(
      cpos_left = region_matrix[,1],
      cpos_right = region_matrix[,2],
      value = values,
      stringsAsFactors = FALSE
    )
  }
  
  df
}


#' Get regions defined by a structural attribute
#' 
#' Get all regions defined by a structural attribute. Unlike
#' `get_region_matrix()` that returns a region matrix for a defined subset of
#' strucs, all regions are returned. As it is the fastest option, the function
#' reads the binary *.rng file for the structural attribute directly. The corpus
#' library (CL) is not used in this case.
#' 
#' @param corpus A length-one `character` vector with a corpus ID.
#' @param s_attr A length-one `character` vector stating a structural attribute.
#' @param registry A length-one `character` vector stating the registry
#'   directory (defaults to CORPUS_REGISTRY environment variable).
#' @param data_dir The data directory of the corpus.
#' @return A two-colum `matrix` with the regions defined by the structural
#'   attribute: Column 1 defines left corpus positions and column 2 right corpus
#'   positions of regions.
#' @examples 
#' s_attr_regions("REUTERS", s_attr = "id")
s_attr_regions <- function(corpus, s_attr, registry = Sys.getenv("CORPUS_REGISTRY"), data_dir = corpus_data_dir(corpus = corpus, registry = registry)){
  rng_file <- file.path(data_dir, paste(s_attr, "rng", sep = "."))
  rng_file_size <- file.info(rng_file)[["size"]]
  con <- file(rng_file, open = "rb")
  rng <- readBin(con, what = integer(), size = 4L, n = rng_file_size / 4L, endian = "big")
  close(con)
  matrix(rng, ncol = 2L, byrow = TRUE)
}