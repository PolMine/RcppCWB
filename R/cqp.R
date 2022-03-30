#' Initialize Corpus Query Processor (CQP).
#' 
#' CQP needs to know where to look for CWB indexed corpora. To initialize CQP,
#' call \code{cqp_initialize}. To reset the registry, use the function
#' \code{cqp_reset_registry}. To get the registry used by CQP, use
#' \code{cqp_get_registry}. To get the initialization status, use
#' \code{cqp_is_initialized}
#' 
#' @param registry the registry directory
#' @export cqp_initialize
#' @rdname cqp_initialize
#' @author Andreas Blaette, Bernard Desgraupes, Sylvain Loiseau
#' @examples
#' cqp_is_initialized() # check initialization status
#' if (!cqp_is_initialized()) cqp_initialize()
#' cqp_is_initialized() # check initialization status (TRUE now?)
#' cqp_get_registry() # get registry dir used by CQP
#' cqp_list_corpora() # get list of corpora
cqp_initialize <- function(registry = Sys.getenv("CORPUS_REGISTRY")){
  registry_new <- registry
  # registry # necessary to capture Sys.getenv() assignment
  if (cqp_is_initialized()){
    warning("CQP has already been initialized. Re-initialization is not possible. ",
            "Only resetting registry.")
  } else {
    # workaround to ensure that global registry variable in dynamic
    # library will have 255 characters. Without starting with a very long (fake)
    # initial registry, there is a bug when resetting the registry dir to a dir
    # that is longer than the initial dir
    dummy_superdir <- tempdir()
    dir.create(dummy_superdir, showWarnings = FALSE)
    if (.Platform$OS.type == "windows"){
      dummy_superdir <- normalizePath(dummy_superdir, winslash = "/")
    }
    # the times argument is 247 for Windows compatibility 
    dummy_regdir <- file.path(
      dummy_superdir, 
      paste0(
        rep("x", times = 246 - nchar(dummy_superdir)),
        collapse = ""
      )
    )
    dir.create(dummy_regdir, showWarnings = FALSE)
    Sys.setenv(CORPUS_REGISTRY = dummy_regdir)
    .init_cqp()
  }
  check_registry(registry_new)
  Sys.setenv(CORPUS_REGISTRY = registry_new)
  cqp_reset_registry()
  return( cqp_is_initialized() )
}


#' @export cqp_is_initialized
#' @rdname cqp_initialize
cqp_is_initialized <- function(){
  if (.cqp_get_status() == 0) return(FALSE) else return(TRUE)
}

#' @param silent A single `logical` value, whether to be silent and suppress CQP
#'   messages (`TRUE`), or not (`FALSE`).
#' @param verbose A single `logical` value, whether to show verbose parser
#'   output (`TRUE`) or not (`FALSE`).
#' @export cqp_verbosity
#' @rdname cqp_initialize
cqp_verbosity <- function(silent, verbose){
  .cqp_verbosity(quietly = silent, verbose = verbose)
}

#' @export cqp_get_registry
#' @rdname cqp_initialize
cqp_get_registry <- function() fs::path(.cqp_get_registry())

#' @export cqp_reset_registry
#' @rdname cqp_initialize
cqp_reset_registry <- function(registry = Sys.getenv("CORPUS_REGISTRY")){
  registry_dir <- registry
  if (!cqp_is_initialized()){
    warning("cannot reset registry, cqp has not yet been initialized!")
    return( FALSE )
  } else {
    check_registry(registry_dir)
    Sys.setenv(CORPUS_REGISTRY = registry_dir)
    if (nchar(registry_dir) > 255L){
      stop("cannot assign new registry: maximum nchar(registry) is 255")
    } else {
      .cqp_set_registry(registry_dir = registry_dir)
      return( TRUE )
    }
  }
}


#' List Available CWB Corpora.
#' 
#' List the corpora described by the registry files in the registry directory
#' that is currently set.
#' 
#' @export cqp_list_corpora
#' @examples
#' cqp_list_corpora()
#' @author Andreas Blaette, Bernard Desgraupes, Sylvain Loiseau
cqp_list_corpora <- function() .cqp_list_corpora()


#' Execute CQP Query and Retrieve Results.
#' 
#' Using CQP queries requires a two-step procedure: At first, you execute a
#' query using \code{cqp_query}. Then, \code{cqp_dump_subcorpus} will return a
#' matrix with the regions of the matches for the query.
#' 
#' The \code{cqp_query} function executes a CQP query. The
#' \code{cqp_subcorpus_size} function returns the number of matches for the CQP
#' query. The \code{cqp_dump_subcorpus} function will return a two-column matrix
#' with the left and right corpus positions of the matches for the CQP query.
#' 
#' @param corpus a CWB corpus
#' @param query a CQP query
#' @param subcorpus subcorpus name
#' @export cqp_query
#' @rdname cqp_query
#' @references 
#' Evert, S. 2005. The CQP Query Language Tutorial. Available online at
#' \url{https://cwb.sourceforge.io/files/CWB_Encoding_Tutorial.pdf}
#' @examples 
#' cqp_query(corpus = "REUTERS", query = '"oil";')
#' cqp_subcorpus_size("REUTERS")
#' cqp_dump_subcorpus("REUTERS")
#' 
#' cqp_query(corpus = "REUTERS", query = '"crude" "oil";')
#' cqp_subcorpus_size("REUTERS", subcorpus = "QUERY")
#' cqp_dump_subcorpus("REUTERS")
#' @author Andreas Blaette, Bernard Desgraupes, Sylvain Loiseau
cqp_query <- function(corpus, query, subcorpus = "QUERY"){
  # stopifnot(corpus %in% cqp_list_corpora())
  query <- check_query(query)
  .cqp_query(corpus = corpus, subcorpus = subcorpus, query = query)
}

#' @export cqp_dump_subcorpus
#' @rdname cqp_query
cqp_dump_subcorpus <- function(corpus, subcorpus = "QUERY"){
  stopifnot(corpus %in% cqp_list_corpora())
  .cqp_dump_subcorpus(paste(corpus, subcorpus, sep = ":"))
}

#' @export cqp_subcorpus_size
#' @rdname cqp_query
cqp_subcorpus_size <- function(corpus, subcorpus = "QUERY"){
  stopifnot(corpus %in% cqp_list_corpora())
  .cqp_subcorpus_size(scorpus = paste(corpus, subcorpus, sep = ":"))
}

#' @export cqp_list_subcorpora
#' @rdname cqp_query
cqp_list_subcorpora <- function(corpus){
  stopifnot(corpus %in% cqp_list_corpora())
  sc <- .cqp_list_subcorpora(inCorpus = corpus)
  # For technical reasons not yet well understood, we find "Last" on the 
  # vector that is returned.
  sc[-which(sc == "Last")]
}

#' @export cqp_drop_subcorpus
#' @rdname cqp_query
cqp_drop_subcorpus <- function(corpus){
  # stopifnot(corpus %in% cqp_list_corpora())
  .cqp_drop_subcorpus(inSubcorpus = corpus)
}

#' Get ranges of subcorpus
#' 
#' @param subcorpus_pointer A pointer (class `externalptr`) referencing a CWB
#'   subcorpus.
subcorpus_get_ranges <- function(subcorpus_pointer){
  .cqp_subcorpus_regions(subcorpus_pointer)
}

#' Create CWB subcorpus from matrix with regions.
#' 
#' @param region_matrix A two-colum `matrix` with regions in rows: Start
#'   position of region in first column, end position in second column.
#' @param corpus A `externalptr` referencing a corpus such as generated by
#'   `cl_find_corpus()`.
#' @param subcorpus A length-one `character` vector providing the name for the
#'   subcorpus.
#' @examples
#' \dontrun{
#' # First we generate a subcorpus from a query result
#' oil_context <- cqp_query("REUTERS", subcorpus = "OIL", query = '[]{3}"oil" []{3}')
#' m <- subcorpus_get_ranges(oil_context)
#' reuters <- cl_find_corpus("REUTERS", registry = get_tmp_registry())
#' p <- matrix_to_subcorpus(subcorpus = "OIL2", corpus = reuters, region_matrix = m)
#' cqp_list_subcorpora("REUTERS")
#' 
#' x <- cqp_query("REUTERS:OIL2", query = '"crude";', subcorpus = "CRUDEOIL")
#' subcorpus_get_ranges(x)
#' 
#' # clean up
#' cqp_drop_subcorpus("REUTERS:OIL")
#' cqp_drop_subcorpus("REUTERS:OIL2")
#' cqp_drop_subcorpus("REUTERS:CRUDEOIL")
#' }
matrix_to_subcorpus <- function(region_matrix, corpus, subcorpus){
  .region_matrix_to_subcorpus(region_matrix = region_matrix, corpus = corpus, subcorpus = subcorpus)
}


#' @param corpus ID of a CWB corpus (length-one `character`).
#' @details `cqp_load_corpus` will return a `logical` value - `TRUE` if corpus
#'   has been loaded successfully, `FALSE` if not.
#' @export cqp_load_corpus
#' @rdname cqp_initialize
cqp_load_corpus <- function(corpus, registry){
  as.logical(.cqp_load_corpus(corpus = corpus, registry = registry))
}
