#' Generate and Write Registry File.
#' 
#' Generate a registry file that declares positional and structural attributes.
#'
#' A CWB indexed corpus needs to be described in a registry file. See the CWB
#' Corpus Encoding Tutorial, Appendix A (pp. 21f) for an example
#' (\url{http://cwb.sourceforge.net/files/CWB_Encoding_Tutorial.pdf}).
#' 
#' @param registry_dir the registry directory
#' @param corpus name of the corpus, will be used as ID (using tolower)
#' @param description long descriptive name for the corpus
#' @param data_dir path to binary data files
#' @param info_file optional info file, if NULL, we assume that file ".info.md" is in \code{data_dir}
#' @param encoding corpus encoding, should be "latin1" or "utf-8"
#' @param language language ISO code, such as "de", "en", "fr"
#' @param p_attributes positional attributes to be declared
#' @param s_attributes structural attributes to be declared
#' @return the character vector of the registry file is returned invisibly
#' @rdname registry_write
#' @export registry_write
registry_write <- function(registry_dir, corpus, description = "", data_dir, info_file = NULL, encoding = c("latin1", "utf-8"), language = c("de", "en", "fr"), p_attributes = "word", s_attributes = NULL){
  if (is.null(info_file)) info_file <- file.path(dirname(data_dir), ".info.md")
  regfile <- c(
    "##",                                                                                                   
    sprintf("## registry entry for corpus %s", toupper(corpus)),                                                                                
    "##",                                                                                                                  
    "",
    "# long descriptive name for the corpus",                                                                              
    sprintf("NAME \"%s\"", description),
    "# corpus ID (must be lowercase in registry!)",                                                                        
    sprintf("ID   %s", tolower(corpus)),                                                                                                        
    "# path to binary data files",                                                                                         
    sprintf("HOME %s", data_dir),
    "# optional info file (displayed by \",info;\" command in CQP)",                                                       
    sprintf("INFO %s", info_file),
    "",                                                                                                                 
    "# corpus properties provide additional information about the corpus:",                                                
    sprintf("##:: charset = \"%s\"\t# character encoding of corpus data", encoding),
    sprintf("##:: language = \"\"\t# insert ISO code for language (de, en, fr, ...)", language), 
    "#========================================================================#",                                          
    "",                                                                                                                   
    "",                                                                                                                   
#     "##",                                                                                                                  
#    "## p-attributes (token annotations)",                                                                                 
#    "##",                                                                                                                  
#    "",                                                                                                               
    paste0("ATTRIBUTE", " ", p_attributes),
#    "",
#    "##",
#    "## s-attributes (structural markup)",
#    "##",
#    "",
    if (! is.null(s_attributes)) paste0("STRUCTURE", " ", s_attributes) else "",
#    "",
#    "Yours sincerely, RcppCWB on behalf of the Encode tool",
    ""

  )
  writeLines(
    text = regfile,
    con = file.path(registry_dir, tolower(corpus))
    )
  invisible(regfile)
}