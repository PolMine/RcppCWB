#' CWB Tools for Creating Corpora
#' 
#' Wrappers for the CWB tools (`cwb-makeall`, `cwb-huffcode`,
#' `cwb-compress-rdx`). Unlike the 'original' command line tools, these wrappers
#' will always perform a specific indexing/compression step on one positional
#' attribute, and produce all components.
#' 
#' @param corpus Name of a CWB corpus (upper case).
#' @param p_attribute Name of p-attribute.
#' @param registry Path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY.
#' @param quietly A `logical` value, whether to turn off messages (including
#'   warnings).
#' @param verbose A `logical` value, whether to show progress information
#'   (counter of tokens processed).
#' @examples
#' # The package includes and 'unfinished' corpus of debates in the UN General 
#' # Assembly ("UNGA"), i.e. it does not yet include the reverse index, and it is
#' # not compressed.
#' #
#' # The first step in the following example is to copy the raw
#' # corpus to a temporary place.
#' 
#' home_dir <- system.file(package = "RcppCWB", "extdata", "cwb", "indexed_corpora", "unga")
#' 
#' tmp_data_dir <- file.path(tempdir(), "indexed_corpora")
#' tmp_unga_dir <- file.path(tmp_data_dir, "unga2")
#' if (!file.exists(tmp_data_dir)) dir.create(tmp_data_dir)
#' if (!file.exists(tmp_unga_dir)){
#'   dir.create(tmp_unga_dir)
#' } else {
#'   file.remove(list.files(tmp_unga_dir, full.names = TRUE))
#' }
#' 
#' regfile <- readLines(
#'   system.file(package = "RcppCWB", "extdata", "cwb", "registry", "unga")
#' )
#' regfile[grep("^HOME", regfile)] <- sprintf('HOME "%s"', tmp_unga_dir)
#' regfile[grep("^ID", regfile)] <- "ID unga2"
#' writeLines(text = regfile, con = file.path(get_tmp_registry(), "unga2"))
#' for (x in list.files(home_dir, full.names = TRUE)){
#'   file.copy(from = x, to = tmp_unga_dir)
#' }
#' 
#' # perform cwb_makeall (equivalent to cwb-makeall command line utility)
#' cwb_makeall(corpus = "UNGA2", p_attribute = "word", registry = get_tmp_registry())
#' cl_load_corpus("UNGA2", registry = get_tmp_registry())
#' cqp_load_corpus("UNGA2", registry = get_tmp_registry())
#' 
#' # see whether it works
#' ids_sentence_1 <- cl_cpos2id(
#'   corpus = "UNGA2", p_attribute = "word", registry = get_tmp_registry(),
#'   cpos = 0:83
#'   )
#' tokens_sentence_1 <- cl_id2str(
#'   corpus = "UNGA2", p_attribute = "word",
#'   registry = get_tmp_registry(), id = ids_sentence_1
#'   )
#' sentence <- gsub("\\s+([\\.,])", "\\1", paste(tokens_sentence_1, collapse = " "))
#' 
#' # perform cwb_huffcode (equivalent to cwb-makeall command line utility)
#' cwb_huffcode(
#'   corpus = "UNGA2",
#'   p_attribute = "word",
#'   registry = get_tmp_registry()
#' )
#' @rdname cwb_utils
#' @export cwb_makeall
#' @importFrom utils capture.output
#' @importFrom fs path path_expand
cwb_makeall <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), quietly = FALSE){
  
  registry <- path_expand(path(registry))
  check_registry(registry)
  regfile <- path(registry, tolower(corpus))
  if (!file.exists(regfile)){
    stop(
      sprintf(
        "No registry file for corpus '%s' in registry directory '%s'.",
        corpus, registry
        )
      )
  }
  
  # The registry directory provided is ignored if the corpus has already been
  # loaded, resulting in unexpected behavior. Therefore, we unload the corpus
  # and force reloading corpora.
  if (toupper(corpus) %in% cqp_list_corpora()){
    cl_delete_corpus(corpus, registry = registry)
    cqp_reset_registry(registry = registry)
  }
  
  makeall <- function()
    .cwb_makeall(x = corpus, p_attribute = p_attribute, registry_dir = registry)
  
  if (quietly){
    capture.output({success <- makeall()}, type = "output")
  } else {
    success <- makeall()
  }
  success
}


#' @rdname cwb_utils
#' @export cwb_huffcode
#' @param delete A `logical` value, whether to remove redundant files after
#'   compression.
cwb_huffcode <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), quietly = FALSE, delete = TRUE){
  
  registry <- path_expand(path(registry))
  check_registry(registry)
  regfile <- path(registry, tolower(corpus))
  if (!file.exists(regfile)){
    stop(sprintf(
      "No registry file for corpus '%s' in registry directory '%s'.",
      corpus, registry
    ))
  }
  
  huffcode <- function()
    .cwb_huffcode(x = corpus, p_attribute = p_attribute, registry_dir = registry)
  
  if (quietly){
    capture.output({success <- huffcode()}, type = "output")
  } else {
    success <- huffcode()
  }
  
  if (delete){
    data_dir <- corpus_data_dir(corpus = corpus, registry = registry)
    fname <- path(data_dir, sprintf("%s.corpus", p_attribute))
    if (!file.exists(fname)) warning("cwb_huffcode: file to delete missing")
    removed <- file.remove(fname)
    if (removed) if (!quietly) message("redundant file deleted: ", fname)
  }
  
  success
  
}

#' @rdname cwb_utils
#' @export cwb_compress_rdx
#' @examples 
#' cwb_compress_rdx(
#'   corpus = "UNGA2",
#'   p_attribute = "word",
#'   registry = get_tmp_registry()
#' )
cwb_compress_rdx <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), quietly = FALSE, delete = TRUE){
  
  registry <- path_expand(path(registry))
  check_registry(registry)
  regfile <- path(registry, tolower(corpus))
  if (!file.exists(regfile)){
    stop(sprintf("No registry file for corpus '%s' in registry directory '%s'.", corpus, registry))
  }
  
  compress_rdx <-function()
    .cwb_compress_rdx(x = corpus, p_attribute = p_attribute, registry_dir = registry)
  
  if (quietly){
    capture.output({success <- compress_rdx()}, type = "output")
  } else {
    success <- compress_rdx()
  }
  
  if (delete){
    data_dir <- corpus_data_dir(corpus = corpus, registry = registry)
    
    rev_file <- path(data_dir, sprintf("%s.corpus.rev", p_attribute))
    if (!file.exists(rev_file)) warning("cwb_huffcode: file to delete missing")
    removed <- file.remove(rev_file)
    if (removed) if (!quietly) message("redundant file deleted: ", rev_file)
    
    rdx_file <- path(data_dir, sprintf("%s.corpus.rdx", p_attribute))
    if (!file.exists(rdx_file)) warning("cwb_huffcode: file to delete missing")
    removed <- file.remove(rdx_file)
    if (removed) if (!quietly) message("redundant file deleted: ", rdx_file)
  }

  success
}

#' @param p_attributes Positional attributes (p-attributes) to be declared.
#' @param data_dir The data directory where `cwb_encode` will save the binary
#'   files of the indexed corpus.  Tilde expansion is performed on `data_dir`
#'   using `path.expand()` to avoid a crash.
#' @param vrt_dir Directory with input corpus files (verticalised format / file
#'   ending *.vrt). Tilde expansion is performed on `vrt_dir` using
#'   `path.expand()` to avoid a crash.
#' @param encoding The encoding of the files to be encoded. Needs to be an
#'   encoding supported by CWB, see `cwb_charsets()`. "UTF-8" is taken as
#'   "utf8". Defaults to "utf8" (recommended charset).
#' @param s_attributes A `list` of named `character` vectors to declare
#'   structural attributes that shall be encoded. The names of the list are the
#'   XML elements present in the corpus. Character vectors making up the list
#'   declare the attributes that include the metadata of regions. To declare a
#'   structural attribute without annotations, provide a zero-length character
#'   vector using `character()` - see examples.
#' @param skip_blank_lines A `logical` value, whether to skip blank lines in the
#'   input.
#' @param strip_whitespace A `logical` value, whether to strip whitespace from
#'   tokens
#' @param xml A `logical` value, whether input is XML.
#' @rdname cwb_utils
#' @export cwb_encode
#' @importFrom fs path
#' @examples
#' data_dir <- file.path(tempdir(), "bt_data_dir")
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
cwb_encode <- function(
  corpus, registry = Sys.getenv("CORPUS_REGISTRY"), data_dir, vrt_dir,
  encoding = "utf8", p_attributes = c("word", "pos", "lemma"), s_attributes,
  skip_blank_lines = TRUE, strip_whitespace = TRUE, xml = TRUE, quietly = FALSE, verbose = FALSE
){
  
  if (encoding == "UTF-8") encoding <- "utf8"
  if (!encoding %in% cwb_charsets()) stop(
    sprintf(
      "encoding '%' is not a valid CWB character set, see cwb_charsets() for options",
      cwb_charsets
    )
  )
  
  stopifnot(
    is.character(corpus), length(corpus) == 1L,
    is.character(registry), length(registry) == 1L, dir.exists(registry),
    is.character(data_dir), length(data_dir) == 1L,
    dir.exists(data_dir), length(list.files(data_dir)) == 0L,
    is.character(vrt_dir), length(vrt_dir) == 1L, dir.exists(vrt_dir),
    length(list.files(vrt_dir)) > 0L,
    is.character(encoding), length(encoding) == 1L,
    is.character(p_attributes),
    is.logical(skip_blank_lines), length(skip_blank_lines) == 1L,
    is.logical(strip_whitespace), length(strip_whitespace) == 1L,
    is.logical(xml), length(xml) == 1L
  )
  
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
  
  # Ensure that paths are standardized
  regfile <- as.character(fs::path(fs::path_expand(registry), tolower(corpus)))
  data_dir <- as.character(fs::path(fs::path_expand(data_dir)))
  vrt_dir <- as.character(fs::path(fs::path_expand(vrt_dir)))
  
  .cwb_encode(
    regfile = regfile,
    data_dir = data_dir,
    vrt_dir = vrt_dir,
    encoding = encoding,
    p_attributes = p_attributes,
    s_attributes_anno = s_attributes_anno,
    s_attributes_noanno = s_attributes_noanno,
    skip_blank_lines = skip_blank_lines,
    xml = xml,
    strip_whitespace = strip_whitespace,
    quiet = quietly,
    verbosity = verbose
  )
}

#' Get CWB version
#' 
#' Get the CWB version used and available when compiling the source code.
#' 
#' @export
#' @return A `numeric_version` object.
#' @examples
#' cwb_version()
cwb_version <- function() as.numeric_version(.cwb_version())




#' Character sets supported by CWB
#' 
#' The function returns a `character` vector with characters sets (charsets)
#' supported by the Corpus Workbench (CWB). The vector is derived from the the
#' `CorpusCharset` object defined in the header file of the corpus library (CL).
#' 
#' Early versions of the CWB were developed for "latin1", "utf8" support has been
#' introduced with CWB v3.2. Note that RcppCWB is tested only for "latin1" and
#' "utf8" and that R uses "UTF-8" rather than utf8" (CWB) by convention.
#' @export
#' @examples
#' cwb_charsets()
cwb_charsets <- function() c(
  "ascii",
  "latin1",
  "latin2",
  "latin3",
  "latin4",
  "cyrillic",
  "arabic",
  "greek",
  "hebrew",
  "latin5",
  "latin6",
  "latin7",
  "latin8",
  "latin9",
  "utf8"
)