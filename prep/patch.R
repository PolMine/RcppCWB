#!/usr/bin/env Rscript 

library(RcppCWB)

repodir <- "~/Lab/github/RcppCWB"
cwb_pkg_dir <- "~/Lab/github/RcppCWB/src/cwb"

global_replacements <- list(
  c("(f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd-stream|redir->stream),", "Rprintf"),
  c("YY(F|D)PRINTF\\s*\\(\\s*(stderr|yyoutput)," , "YY\\1PRINTF ("),
  c("fprintf.", "Rprintf"),
  c("#  define YYFPRINTF fprintf", "# define YYFPRINTF Rprintf")
)

for (subdir in c("cl", "cqp", "CQi")){
  files <- list.files(file.path(cwb_pkg_dir, subdir), full.names = TRUE)
  message("#### Directory: ", subdir)
  for (f in files){
    message("- ", basename(f))
    code <- readLines(f)
    for (i in length(global_replacements)){
      code <- gsub(global_replacements[[i]][1], global_replacements[[i]][2], code)
    }
    writeLines(text = code, con = f)
  }
}

if (FALSE){
  status(repo = repodir)
}