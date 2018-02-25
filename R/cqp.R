#' Initialize CQP.
#' 
#' @export initialize_cwb
#' @rdname init_cqp
#' @examples
#' initialize_cwb()
#' get_registry()
#' get_progname()
initialize_cwb <- function(){
  .initialize_cwb()
}


#' @export get_registry
#' @rdname init_cqp
get_registry <- function(){
  .get_registry()
}




#' List Corpora Defined in Registry Directory.
#' @export cqp_list_corpora
#' @examples
#' registry <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' Sys.setenv("CORPUS_REGISTRY" = registry)
#' initialize_cwb()
#' cqp_list_corpora()
cwb_list_corpora <- function(){
  .cwb_list_corpora()
}

#' Perform a CQP Query.
#' @param corpus a CWB corpus
#' @param query 
#' @export cqp_query
#' @rdname cqp_subcorpora
cqp_query <- function(corpus, query){
  .cqp_query(inMother = corpus, inChild = "FOO", inQuery = query)
}

#' @export cqp_dump_subcorpus
#' @rdname cqp_subcorpora
cqp_dump_subcorpus <- function(corpus, child = "FOO"){
  .cqp_dump_subcorpus(paste(corpus, child, sep = "."))
}

#' @export cqp_subcorpus_size
#' @rdname cqp_subcorpora
cqp_subcorpus_size <- function(corpus, child = "FOO"){
  .cqp_subcorpus_size(paste(corpus, child, sep = "."))
}

#' @export cqp_list_subcorpora
#' @rdname cqp_subcorpora
cqp_list_subcorpora <- function(corpus){
  .cqp_list_subcorpora(inMother = corpus)
}

