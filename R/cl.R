#' Exposed functions of the corpus library (CL).
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param attribute name of a s- or p-attribute
#' @param attribute_type either "p" or "s"
#' @param registry path to the registry directory
#' @param p_attribute a p-attribute
#' @param s_attribute a s-attribute
#' @param cpos corpus positions (integer vector)
#' @param struc strucs (integer vector)
#' @param id id of a token
#' @param regex a regular expression
#' @param str a character string
#' @rdname cl
#' @export cl_attribute_size cwb_lexicon_size cwb_cpos2struc cwb_cpos2str cwb_cpos2id
#' @export cwb_struc2cpos cwb_id2str cwb_struc2str cwb_regex2id cwb_str2id cwb_id2freq cwb_id2cpos 
#' @export cwb_cpos2lbound cwb_cpos2rbound
#' @name cl_functions
NULL

#' @rdname cl
cl_attribute_size <- function(corpus, attribute, attribute_type, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cl_attribute_size(corpus = corpus, attribute = attribute, attribute_type = attribute_type, registry = registry)
}


#' @rdname cl
cwb_lexicon_size <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_lexicon_size(corpus = corpus, p_attribute = p_attribute, registry = registry)
}


#' @rdname cl
cwb_cpos2struc <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_cpos2struc(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}


#' @rdname cl
cwb_cpos2str <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), cpos){
  .cwb_cpos2str(corpus = corpus, p_attribute = p_attribute, registry = registry, cpos = cpos)
}


#' @rdname cl
cwb_cpos2id <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), cpos){
  .cwb_cpos2id(corpus = corpus, p_attribute = p_attribute, registry = registry, cpos = cpos)
}

#' @rdname cl
cwb_struc2cpos <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), struc){
  .cwb_struc2cpos(corpus = corpus, s_attribute = s_attribute, registry = registry, struc = struc)
}


#' @rdname cl
cwb_id2str <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), id){
  .cwb_id2str(corpus = corpus, p_attribute = p_attribute, registry = registry, id = id)
}


#' @rdname cl
cwb_struc2str <- function(corpus, s_attribute, struc, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_struc2str(corpus = corpus, s_attribute = s_attribute, struc = struc, registry = registry)
}


#' @rdname cl
cwb_regex2id <- function(corpus, p_attribute, regex, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_regex2id(corpus = corpus, p_attribute = p_attribute, regex = regex, registry = registry)
}


#' @rdname cl
cwb_str2id <- function(corpus, p_attribute, str, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_str2id(corpus = corpus, p_attribute = p_attribute, str = str, registry = registry)
}

#' @rdname cl
cwb_id2freq <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_id2freq(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
}


#' @rdname cl
cwb_id2cpos <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_id2cpos(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
}


#' @rdname cl
cwb_cpos2lbound <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_cpos2lbound(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}

#' @rdname cl
cwb_cpos2rbound <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_cpos2rbound(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}
