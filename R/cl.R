#' Get Attribute Size (of Positional/Structural Attribute).
#' 
#' Use \code{cl_attribute_size} to get the total number of values of a
#' positional attribute (param \code{attribute_type} = "p"), or structural
#' attribute (param \code{attribute_type} = "s"). Note that indices are
#' zero-based, i.e. the maximum position of a positional / structural
#' attribute is attribute size minus 1 (see examples).
#' @rdname cl_attribute_size
#' @param corpus name of a CWB corpus (upper case)
#' @param attribute name of a p- or s-attribute
#' @param attribute_type either "p" or "s", for structural/positional attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @examples 
#' token_no <- cl_attribute_size(
#'   "REUTERS",
#'   attribute = "word",
#'   attribute_type = "p",
#'   registry = get_tmp_registry()
#' )
#' corpus_positions <- seq.int(from = 0, to = token_no - 1)
#' cl_cpos2id(
#'   "REUTERS",
#'   "word",
#'   cpos = corpus_positions,
#'   registry = get_tmp_registry()
#' )
#' 
#' places_no <- cl_attribute_size(
#'   "REUTERS",
#'   attribute = "places",
#'   attribute_type = "s",
#'   registry = get_tmp_registry()
#' )
#' strucs <- seq.int(from = 0, to = places_no - 1)
#' cl_struc2str(
#'   "REUTERS",
#'   "places",
#'   struc = strucs,
#'   registry = get_tmp_registry()
#' )
cl_attribute_size <- function(corpus, attribute, attribute_type, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cl_attribute_size(corpus = corpus, attribute = attribute, attribute_type = attribute_type, registry = registry)
}

#' Get Lexicon Size.
#' 
#' Get the total number of unique tokens/ids of a positional attribute. Note
#' that token ids are zero-based, i.e. when iterating through tokens, start at
#' 0, the maximum will be \code{cl_lexicon_size()} minus 1.
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param p_attribute name of positional attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @rdname cl_lexicon_size
#' @examples 
#' lexicon_size <- cl_lexicon_size(
#'   "REUTERS",
#'   p_attribute = "word",
#'   registry = get_tmp_registry()
#' )
#' 
#' token_ids <- seq.int(from = 0, to = lexicon_size - 1)
#' cl_id2str(
#'   "REUTERS",
#'   p_attribute = "word",
#'   id = token_ids,
#'   registry = get_tmp_registry()
#' )
cl_lexicon_size <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cl_lexicon_size(corpus = corpus, p_attribute = p_attribute, registry = registry)
}


#' @title Using Structural Attributes.
#' 
#' @description Structural attributes store the metadata of texts in a CWB
#'   corpus and/or any kind of annotation of a region of text. The fundamental
#'   unit are so-called strucs, i.e. indices of regions identified by a left and
#'   a right corpus position. The corpus library (CL) offers a set of functions
#'   to make the translations between corpus positions (cpos) and strucs
#'   (struc).
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param s_attribute name of structural attribute (character vector)
#' @param cpos An \code{integer} vector with corpus positions.
#' @param struc a struc identifying a region
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @rdname s_attributes
#' @name CL: s_attributes
#' @examples
#' # get metadata for matches of token
#' # scenario: id of the texts with occurrence of 'oil'
#' token_to_get <- "oil"
#' token_id <- cl_str2id("REUTERS", p_attribute = "word", str = "oil", get_tmp_registry())
#' token_cpos <- cl_id2cpos("REUTERS", p_attribute = "word", id = token_id, get_tmp_registry())
#' strucs <- cl_cpos2struc("REUTERS", s_attribute = "id", cpos = token_cpos, get_tmp_registry())
#' strucs_unique <- unique(strucs)
#' text_ids <- cl_struc2str("REUTERS", s_attribute = "id", struc = strucs_unique, get_tmp_registry())
#' 
#' # get the full text of the first text with match for 'oil'
#' left_cpos <- cl_cpos2lbound(
#'   "REUTERS", s_attribute = "id",
#'   cpos = min(token_cpos),
#'   registry = get_tmp_registry()
#' )
#' right_cpos <- cl_cpos2rbound(
#'   "REUTERS",
#'   s_attribute = "id",
#'   cpos = min(token_cpos),
#'   registry = get_tmp_registry()
#' )
#' txt <- cl_cpos2str(
#'   "REUTERS", p_attribute = "word",
#'   cpos = left_cpos:right_cpos,
#'   registry = get_tmp_registry()
#' )
#' fulltext <- paste(txt, collapse = " ")
#' 
#' # alternativ approach to achieve same result
#' first_struc_match_oil <- cl_cpos2struc(
#'   "REUTERS", s_attribute = "id",
#'   cpos = min(token_cpos),
#'   registry = get_tmp_registry()
#'   )
#' cpos_struc <- cl_struc2cpos(
#'   "REUTERS", s_attribute = "id",
#'   struc = first_struc_match_oil,
#'   registry = get_tmp_registry()
#' )
#' txt <- cl_cpos2str(
#'   "REUTERS",
#'   p_attribute = "word",
#'   cpos = cpos_struc[1]:cpos_struc[2],
#'   registry = get_tmp_registry()
#' )
#' fulltext <- paste(txt, collapse = " ")
cl_cpos2struc <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  
  if (length(cpos) == 0L) return(integer())
  check_cpos(corpus = corpus, p_attribute = "word", cpos = cpos, registry = registry)
  
  .cl_cpos2struc(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}

#' @rdname s_attributes
cl_struc2cpos <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), struc){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  check_strucs(corpus = corpus, s_attribute = s_attribute, strucs = struc, registry = registry)
  .cl_struc2cpos(corpus = corpus, s_attribute = s_attribute, registry = registry, struc = struc)
}

#' @rdname s_attributes
cl_struc2str <- function(corpus, s_attribute, struc, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  check_strucs(corpus = corpus, s_attribute = s_attribute, strucs = struc, registry = registry)
  .cl_struc2str(corpus = corpus, s_attribute = s_attribute, struc = struc, registry = registry)
}

#' @rdname s_attributes
cl_cpos2lbound <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  
  if (length(cpos) == 0L) return(integer())
  check_cpos(corpus = corpus, p_attribute = "word", cpos = cpos, registry = registry)
  
  .cl_cpos2lbound(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}

#' @rdname s_attributes
cl_cpos2rbound <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  
  check_cpos(corpus = corpus, p_attribute = "word", cpos = cpos, registry = registry)
  if (length(cpos) == 0L) return(integer())
  
  .cl_cpos2rbound(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}






#' @title Using Positional Attributes.
#' 
#' @description CWB indexed corpora store the text of a corpus as numbers: Every token
#' in the token stream of the corpus is identified by a unique corpus
#' position. The string value of every token is identified by a unique integer
#' id. The corpus library (CL) offers a set of functions to make the transitions
#' between corpus positions, token ids, and the character string of tokens.
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @param p_attribute a p-attribute (positional attribute)
#' @param cpos corpus positions (integer vector)
#' @param id id of a token
#' @param regex a regular expression
#' @param str a character string
#' @rdname p_attributes
#' @name CL: p_attributes
#' @examples 
#' # registry directory and cpos_total will be needed in examples
#' cpos_total <- cl_attribute_size(
#'   corpus = "REUTERS", attribute = "word",
#'   attribute_type = "p", registry = get_tmp_registry()
#'   )
#' 
#' # decode the token stream of the corpus (the quick way)
#' token_stream_str <- cl_cpos2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   cpos = seq.int(from = 0, to = cpos_total - 1),
#'   registry = get_tmp_registry()
#'   )
#'   
#' # decode the token stream (cpos2id first, then id2str)
#' token_stream_ids <- cl_cpos2id(
#'   corpus = "REUTERS", p_attribute = "word",
#'   cpos = seq.int(from = 0, to = cpos_total - 1),
#'   registry = get_tmp_registry()
#'   )
#' token_stream_str <- cl_id2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   id = token_stream_ids, registry = get_tmp_registry()
#' )
#' 
#' # get corpus positions of a token
#' token_to_get <- "oil"
#' id_oil <- cl_str2id(
#'   corpus = "REUTERS", p_attribute = "word",
#'   str = token_to_get, registry = get_tmp_registry()
#'   )
#' cpos_oil <- cl_id2cpos <- cl_id2cpos(
#'   corpus = "REUTERS", p_attribute = "word",
#'   id = id_oil, registry = get_tmp_registry()
#' )
#' 
#' # get frequency of token
#' oil_freq <- cl_id2freq(
#'   corpus = "REUTERS", p_attribute = "word", id = id_oil, registry = get_tmp_registry()
#' )
#' length(cpos_oil) # needs to be the same as oil_freq
#' 
#' # use regular expressions 
#' ids <- cl_regex2id(
#'   corpus = "REUTERS", p_attribute = "word",
#'   regex = "M.*", registry = get_tmp_registry()
#' )
#' m_words <- cl_id2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   id = ids, registry = get_tmp_registry()
#' )
#' 
cl_cpos2str <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), cpos){
  check_registry(registry)
  check_corpus(corpus, registry)
  if (length(cpos) == 0L) return(integer())
  .cl_cpos2str(corpus = corpus, p_attribute = p_attribute, registry = registry, cpos = cpos)
}

#' @rdname p_attributes
cl_cpos2id <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), cpos){
  check_registry(registry)
  check_corpus(corpus, registry)
  if (length(cpos) == 0L) return(integer())
  .cl_cpos2id(corpus = corpus, p_attribute = p_attribute, registry = registry, cpos = cpos)
}

#' @rdname p_attributes
cl_id2str <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), id){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_id(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
  .cl_id2str(corpus = corpus, p_attribute = p_attribute, registry = registry, id = id)
}

#' @rdname p_attributes
cl_regex2id <- function(corpus, p_attribute, regex, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  .cl_regex2id(corpus = corpus, p_attribute = p_attribute, regex = regex, registry = registry)
}

#' @rdname p_attributes
cl_str2id <- function(corpus, p_attribute, str, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  .cl_str2id(corpus = corpus, p_attribute = p_attribute, str = str, registry = registry)
}

#' @rdname p_attributes
cl_id2freq <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_p_attribute(p_attribute = p_attribute, corpus = corpus, registry = registry)
  check_id(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
  .cl_id2freq(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
}


#' @rdname p_attributes
cl_id2cpos <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry)
  check_p_attribute(p_attribute = p_attribute, corpus = corpus, registry = registry)
  check_id(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
  .cl_id2cpos(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
}

#' Load corpus.
#' 
#' @inheritParams cl_delete_corpus
#' @return A `externalptr` referencing the C representation of the corpus.
#' @export
cl_find_corpus <- function(corpus, registry){
  .cl_find_corpus(corpus = tolower(corpus), registry = registry)
}

#' Drop loaded corpus.
#' 
#' Remove a corpus from the list of loaded corpora of the corpus library (CL).
#' 
#' The corpus library (CL) internally maintains a list of corpora including
#' information on positional and structural attributes so that the registry file
#' needs not be parsed again and again. However, when an attribute has been
#' added to the corpus, it will not yet be visible, because it is not part of
#' the data that has been loaded. The \code{cl_delete_corpus} function exposes a
#' CL function named identically, to force reloading the corpus (after it has
#' been deleted), which will include parsing an updated registry file.
#' 
#' @param corpus name of a CWB corpus (upper case) 
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @return An `integer` value 1 is returned invisibly if a previously loaded
#'   corpus has been deleted, or 0 if the corpus has not been loaded and has not
#'   been deleted.
#' @export cl_delete_corpus
#' @examples
#' cl_attribute_size("UNGA", attribute = "word", attribute_type = "p")
#' corpus_is_loaded("UNGA")
#' cl_delete_corpus("UNGA")
#' corpus_is_loaded("UNGA")
cl_delete_corpus <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  as.logical(.cl_delete_corpus(corpus = corpus, registry = registry))
}

#' Get charset of a corpus.
#' 
#' The encoding of a corpus is declared in the registry file (corpus property
#' "charset"). Once a corpus is loaded, this information is available without
#' parsing the registry file again and again. The \code{cl_charset_name} offers
#' a quick access to this information.
#' 
#' @param corpus Name of a CWB corpus (upper case).
#' @param registry Path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @export cl_charset_name
#' @examples
#' cl_charset_name(
#'   corpus = "REUTERS",
#'   registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' )
cl_charset_name <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cl_charset_name(corpus = corpus, registry = registry)
}

#' Check whether structural attribute has values
#' 
#' Structural attributes do not necessarily have values, structural attributes
#' (such as annotations of sentences or paragraphs) may just define regions of
#' corpus positions. Use this function to test whether an attribute has values.
#' 
#' @param corpus Corpus ID, a length-one `character` vector.
#' @param s_attribute Structural attribute to check, a length-one `character` vector.
#' @param registry The registry directory of the corpus.
#' @return `TRUE` if the attribute has values and `FALSE` if not. `NA` if the structural
#'   attribute is not available.
#' @export cl_struc_values
#' @examples
#' cl_struc_values("REUTERS", "places") # TRUE - attribute has values
#' cl_struc_values("REUTERS", "date") # NA - attribute does not exist
cl_struc_values <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry)
  registry <- normalizePath(path.expand(registry))
  i <- .cl_struc_values(corpus = corpus, s_attribute = s_attribute, registry = registry)
  if (i == 1L) TRUE else if (i == 0L) FALSE else if (i < 0L) as.integer(NA)
}

#' Get data directory of a corpus
#' 
#' Extract the data directory from the intenal C representation of the content
#' of the registry file for a corpus.
#' @param corpus A length-one `character` vector with the corpus ID.
#' @param registry A length-one `character` vector with the registry directory.
#' @return A length-one `character` vector stating the data directory.
#' @export corpus_data_dir
#' @examples
#' corpus_data_dir("REUTERS")
corpus_data_dir <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry)
  registry <- normalizePath(path.expand(registry))
  .corpus_data_dir(corpus = corpus, registry = registry)
}

#' Check whether corpus is loaded
#' 
#' @inheritParams corpus_data_dir
#' @return `TRUE` if corpus is loaded and `FALSE` if not.
#' @export corpus_is_loaded
corpus_is_loaded <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  as.logical(.corpus_is_loaded(corpus = corpus, registry = registry))
}

#' Load corpus
#' 
#' @inheritParams corpus_data_dir
#' @return `TRUE` if corpus could be loaded and `FALSE` if not.
#' @export corpus_is_loaded
#' @examples
#' cl_load_corpus("REUTERS")
cl_load_corpus <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  as.logical(.cl_load_corpus(corpus = corpus, registry = registry))
}

#' Show CL corpora
#' 
#' @return A `character` vector.
#' @export corpus_is_loaded
#' @examples
#' cl_list_corpora()
cl_list_corpora <- function(){
  .cl_list_corpora()
}

############### experimental functionality ##########################

#' Experimental low-level CL access.
#' 
#' Set of functions with same functionality as cl_* functions to improve the
#' ease of writing code.
#' 
#' @name cl_rework
#' @rdname cl_rework
NULL

#' @param corpus ID of a CWB corpus (length-one `character` vector).
#' @param s_attribute A structural attribute (length-one `character` vector).
#' @param registry Registry directory.
#' @rdname cl_rework
#' @export
s_attr <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .s_attr(corpus = corpus, s_attribute = s_attribute, registry = registry)
}

#' @param p_attribute A positional attribute (length-one `character` vector).
#' @rdname cl_rework
#' @export
p_attr <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .p_attr(corpus = corpus, p_attribute = p_attribute, registry = registry)
}

#' @param p_attr A `externalptr` referencing a p-attribute.
#' @rdname cl_rework
#' @export
p_attr_size <- function(p_attr){
  .p_attr_size(p_attr)
}

#' @param s_attr A `externalptr` referencing a p-attribute.
#' @rdname cl_rework
#' @export
s_attr_size <- function(s_attr){
  .s_attr_size(s_attr)
}

#' @rdname cl_rework
#' @export
p_attr_lexicon_size <- function(p_attr){
  .p_attr_lexicon_size(p_attr)
}

#' @param cpos An `integer` vector of corpus positions.
#' @rdname cl_rework
#' @export
cpos_to_struc <- function(cpos, s_attr){
  .cpos_to_struc(s_attr = s_attr, cpos = cpos) # reverse order!
}

#' @rdname cl_rework
#' @export
cpos_to_str <- function(cpos, p_attr){
  .cpos_to_str(p_attr = p_attr, cpos = cpos) # reverse order!
}

#' @rdname cl_rework
#' @export
cpos_to_id <- function(cpos, p_attr){
  .cpos_to_id(p_attr = p_attr, cpos = cpos) # reverse order!
}


#' @param struc A length-one `integer` vector with a struc.
#' @rdname cl_rework
#' @export
struc_to_cpos <- function(struc, s_attr){
  .struc_to_cpos(s_attr = s_attr, struc = struc) # reverse order!
}

#' @rdname cl_rework
#' @export
struc_to_str <- function(struc, s_attr){
  .struc_to_str(s_attr = s_attr, struc = struc) # reverse order!
}

#' @param regex A regular expression.
#' @rdname cl_rework
#' @export
regex_to_id <- function(regex, p_attr){
  .regex_to_id(p_attr = p_attr, regex = regex) # reverse order!
}

#' @param str A `character` vector.
#' @rdname cl_rework
#' @export
str_to_id <- function(str, p_attr){
  .str_to_id(p_attr = p_attr, str = str) # reverse order!
}

#' @param id An `integer` vector with token ids.
#' @rdname cl_rework
#' @export
id_to_freq <- function(id, p_attr){
  .id_to_freq(p_attr = p_attr, id = id) # reverse order!
}

#' @rdname cl_rework
#' @export
id_to_cpos <- function(id, p_attr){
  .id_to_cpos(p_attr = p_attr, id = id) # reverse order!
}

#' @rdname cl_rework
#' @export
cpos_to_lbound <- function(cpos, s_attr){
  .cpos_to_lbound(s_attr = s_attr, cpos = cpos) # reverse order!
}


#' @rdname cl_rework
#' @export
cpos_to_rbound <- function(cpos, s_attr){
  .cpos_to_rbound(s_attr = s_attr, cpos = cpos) # reverse order!
}
