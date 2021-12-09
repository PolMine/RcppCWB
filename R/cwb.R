#' CWB Tools for Creating Corpora
#' 
#' Wrappers for the CWB tools (\code{cwb-makeall}, \code{cwb-huffcode},
#' \code{cwb-compress-rdx}). Unlike the 'original' command line tools, these
#' wrappers will always perform a specific indexing/compression step on one
#' positional attribute, and produce all components.
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param p_attribute name p-attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @examples
#' # The package includes and 'unfinished' corpus of debates in the UN General 
#' # Assembly ("UNGA"), i.e. it does not yet include the reverse index, and it is
#' # not compressed.
#' #
#' # The first step in the following example is to copy the raw
#' # corpus to a temporary place.
#' 
#' registry <- if (!check_pkg_registry_files()) use_tmp_registry() else get_pkg_registry()
#' home_dir <- system.file(package = "RcppCWB", "extdata", "cwb", "indexed_corpora", "unga")
#' 
#' tmpdir <- normalizePath(tempdir(), winslash = "/")
#' tmp_regdir <- file.path(tmpdir, "registry_tmp", fsep = "/")
#' tmp_data_dir <- file.path(tmpdir, "indexed_corpora", fsep = "/")
#' tmp_unga_dir <- file.path(tmp_data_dir, "unga", fsep = "/")
#' if (!file.exists(tmp_regdir)) dir.create(tmp_regdir)
#' if (!file.exists(tmp_data_dir)) dir.create(tmp_data_dir)
#' if (!file.exists(tmp_unga_dir)){
#'   dir.create(tmp_unga_dir)
#' } else {
#'   file.remove(list.files(tmp_unga_dir, full.names = TRUE))
#' }
#' regfile <- readLines(file.path(registry, "unga"))
#' regfile[grep("^HOME", regfile)] <- sprintf('HOME "%s"', tmp_unga_dir)
#' writeLines(text = regfile, con = file.path(tmp_regdir, "unga"))
#' for (x in list.files(home_dir, full.names = TRUE)){
#'   file.copy(from = x, to = tmp_unga_dir)
#' }
#' 
#' # perform cwb_makeall (equivalent to cwb-makeall command line utility)
#' cwb_makeall(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
#' 
#' # see whether it works
#' ids_sentence_1 <- cl_cpos2id(
#'   corpus = "UNGA", p_attribute = "word", registry = tmp_regdir,
#'   cpos = 0:83
#'   )
#' tokens_sentence_1 <- cl_id2str(
#'   corpus = "UNGA", p_attribute = "word",
#'   registry = tmp_regdir, id = ids_sentence_1
#'   )
#' sentence <- gsub("\\s+([\\.,])", "\\1", paste(tokens_sentence_1, collapse = " "))
#' 
#' # perform cwb_huffcode (equivalent to cwb-makeall command line utility)
#' if (.Platform$OS.type != "windows"){
#'   cwb_huffcode(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
#' }
#' @rdname cwb_utils
#' @export cwb_makeall
cwb_makeall <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  regfile <- file.path(normalizePath(registry, winslash = "/"), tolower(corpus), fsep = "/")
  if (!file.exists(regfile)){
    stop(sprintf("No registry file for corpus '%s' in registry directory '%s'.", corpus, registry))
  }
  
  # The registry directory provided is ignored if the corpus has already been loaded, resulting 
  # in unexpected behavior. Therefore, we unload the corpus and force reloading corpora.
  if (toupper(corpus) %in% cqp_list_corpora()){
    cl_delete_corpus(corpus, registry = registry)
    cqp_reset_registry(registry = registry)
  }
  
  .cwb_makeall(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}


#' @rdname cwb_utils
#' @export cwb_huffcode
cwb_huffcode <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_huffcode(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}

#' @rdname cwb_utils
#' @export cwb_compress_rdx
#' @examples 
#' if (.Platform$OS.type != "windows"){
#'   cwb_compress_rdx(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
#' }
cwb_compress_rdx <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_compress_rdx(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}

#' @param p_attributes Positional attributes (p-attributes) to be declared.
#' @param data_dir The data directory where `cwb_encode` will put the binary
#'   files of the indexed corpus.
#' @param vrt_dir Directory with input corpus files (verticalised format / file
#'   ending *.vrt).
#' @param s_attributes A `list` of named `character` vectors to declare
#'   structural attributes that shall be encoded. The names of the list are the
#'   XML elements present in the corpus. Character vectors making up the list
#'   declare the attributes that include the metadata of regions. To declare a
#'   structural attribute without annotations, provide a zero-length character
#'   vector using `character()` - see examples.
#' @rdname cwb_utils
#' @export cwb_encode
#' @examples
#' if (.Platform$OS.type != "windows"){
#' data_dir <- file.path(tempdir(), "tmp_data_dir")
#' dir.create(data_dir)
#' 
#' cwb_encode(
#'   corpus = "BTMIN",
#'   registry = Sys.getenv("CORPUS_REGISTRY"),
#'   vrt_dir = system.file(package = "RcppCWB", "extdata", "vrt"),
#'   data_dir = data_dir,
#'   p_attributes = c("word", "pos", "lemma"),
#'   s_attributes = list(
#'     plenary_protocol = c(
#'       "lp", "protocol_no", "date", "year", "birthday", "version",
#'       "url", "filetype"
#'     ),
#'     speaker = c(
#'       "id", "type", "lp", "protocol_no", "date", "year", "ai_no", "ai_id",
#'       "ai_type", "who", "name", "parliamentary_group", "party", "role"
#'      ),
#'     p = character()
#'   )
#' )
#' 
#' unlink(data_dir)
#' unlink(file.path(Sys.getenv("CORPUS_REGISTRY"), "btmin"))
#' }
cwb_encode <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY"), data_dir, vrt_dir, p_attributes = c("word", "pos", "lemma"), s_attributes){
  
  s_attributes_noanno <- unlist(lapply(
    names(s_attributes),
    function(s_attr) if (length(s_attributes[[s_attr]]) == 0L) s_attr else character()
  ))
  
  for (s_attr in s_attributes_noanno) s_attributes[[s_attr]] <- NULL
  
  s_attributes_anno <- unname(
    sapply(
      names(s_attributes),
      function(s_attr) paste(s_attr, ":", 0L, "+", paste(s_attributes[[s_attr]], collapse = "+"), sep = "")
    )
  )
  
  .cwb_encode(
    regfile = file.path(registry, tolower(corpus)),
    data_dir = data_dir,
    vrt_dir = vrt_dir,
    p_attributes = p_attributes,
    s_attributes_anno = s_attributes_anno,
    s_attributes_noanno = s_attributes_noanno
  )
}
