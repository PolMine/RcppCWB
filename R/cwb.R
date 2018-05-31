#' CWB Utility Functions
#' 
#' Wrappers for CWB utilities (cwb-makeall, cwb-huffcode, cwb-compress-rdx).
#' 
#' @rdname cwb_utils
#' @param corpus name of a CWB corpus (upper case)
#' @param p_attribute name p-attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @export cwb_makeall
cwb_makeall <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_makeall(corpus = corpus, p_attribute = p_attribute, registry_dir = registry)
}
