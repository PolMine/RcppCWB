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
#' @examples
#' regdir <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' tmpdir <- tempdir()
#' tmp_regdir <- file.path(tmpdir, "registry")
#' tmp_data_dir <- file.path(tmpdir, "indexed_corpora")
#' tmp_unga_dir <- file.path(tmp_data_dir, "unga")
#' if (!file.exists(tmp_regdir)) dir.create(tmp_regdir)
#' if (!file.exists(tmp_data_dir)) dir.create(tmp_data_dir)
#' if (!file.exists(tmp_unga_dir)) dir.create(tmp_unga_dir)
#' file.copy(from = file.path(regdir, "unga"), to = file.path(tmp_regdir, "unga"))
#' home_dir <- registry_file_parse(corpus = "unga", registry_dir = regdir)[["home"]]
#' for (x in list.files(home_dir, full.names = TRUE)){
#'   file.copy(from = x, to = tmp_unga_dir)
#' }
#' cwb_makeall(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
#' ids_sentence_1 <- cl_cpos2id(
#'   corpus = "UNGA", p_attribute = "word", registry = tmp_regdir,
#'   cpos = 0:83
#'   )
#' tokens_sentence_1 <- cl_id2str(
#'   corpus = "UNGA", p_attribute = "word",
#'   registry = tmp_regdir, id = ids_sentence_1
#'   )
#' sentence <- gsub("\\s+([\\.,])", "\\1", paste(token, collapse = " "))
cwb_makeall <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_makeall(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}
