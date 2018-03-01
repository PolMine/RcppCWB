#' Initialize Corpus Query Processor (CQP).
#' 
#' CQP will need to know where to look for CWB indexed corpora. Upon
#' initialization of CQP, the registry directory is set, by calling the function
#' \code{cqp_initialize} during the startup procedure. To reset the registry,
#' use the function \code{cqp_set_registry}. To get the registry used by CQP,
#' use \code{cqp_get_registry}.
#' 
#' 
#' @param registry the registry directory
#' @export cqp_initialize
#' @rdname cqp_initialize
#' @author Andreas Blaette, Bernard Desgraupes, Sylvain Loiseau
#' @examples
#' cqp_get_registry()
cqp_initialize <- function() .init_cqp()


#' @export cqp_get_registry
#' @rdname cqp_initialize
cqp_get_registry <- function() .cqp_get_registry()

#' @export cqp_set_registry
#' @rdname cqp_initialize
cqp_set_registry <- function(registry = Sys.getenv("CORPUS_REGISTRY")){
  .cqp_set_registry(registry_dir = registry)
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
#' @param child a subcorpus
#' @export cqp_query
#' @rdname cqp_query
#' @references 
#' Evert, S. 2005. The CQP Query Language Tutorial. Available online at
#' \url{http://cwb.sourceforge.net/files/CWB_Encoding_Tutorial.pdf}
#' @examples 
#' cqp_query(corpus = "REUTERS", query = '"oil";')
#' cqp_subcorpus_size("REUTERS")
#' cqp_dump_subcorpus("REUTERS")
#' 
#' cqp_query(corpus = "REUTERS", query = '"crude" "oil";')
#' cqp_subcorpus_size("REUTERS", child = "QUERY")
#' cqp_dump_subcorpus("REUTERS")
#' @author Andreas Blaette, Bernard Desgraupes, Sylvain Loiseau
cqp_query <- function(corpus, query){
  stopifnot(corpus %in% cqp_list_corpora())
  query <- .check_cqp_query(query)
  .cqp_query(corpus = corpus, subcorpus = "QUERY", query = query)
}

#' @export cqp_dump_subcorpus
#' @rdname cqp_query
cqp_dump_subcorpus <- function(corpus, child = "QUERY"){
  stopifnot(corpus %in% cqp_list_corpora())
  .cqp_dump_subcorpus(paste(corpus, child, sep = ":"))
}

#' @export cqp_subcorpus_size
#' @rdname cqp_query
cqp_subcorpus_size <- function(corpus, child = "QUERY"){
  stopifnot(corpus %in% cqp_list_corpora())
  .cqp_subcorpus_size(scorpus = paste(corpus, child, sep = ":"))
}

#' @export cqp_list_subcorpora
#' @rdname cqp_query
cqp_list_subcorpora <- function(corpus){
  stopifnot(corpus %in% cqp_list_corpora())
  .cqp_list_subcorpora(inCorpus = corpus)
}

