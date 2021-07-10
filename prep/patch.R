#!/usr/bin/env Rscript 

library(RcppCWB)

repodir <- "~/Lab/github/RcppCWB"
cwb_pkg_dir <- "~/Lab/github/RcppCWB/src/cwb"

global_replacements <- list(
  c("(f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd-stream|redir->stream),\\s*", "Rprintf("),
  c("YY(F|D)PRINTF\\s*\\(\\s*(stderr|yyoutput)," , "YY\\1PRINTF ("),
  c("fprintf\\(", "Rprintf("),
  c("(\\s+)printf\\(", "\\1Rprintf("),
  c("#  define YYFPRINTF fprintf", "# define YYFPRINTF Rprintf"),
  
  c('^\\s*#include\\s+"endian\\.h"\\s*$', '#include "endian2.h"') # only files in cl, maybe limit this
)

for (subdir in c("cl", "cqp", "CQi")){
  files <- list.files(file.path(cwb_pkg_dir, subdir), full.names = TRUE)
  message("#### Directory: ", subdir)
  for (f in files){
    message("- ", basename(f))
    code <- readLines(f)
    for (i in 1:length(global_replacements)){
      code <- gsub(global_replacements[[i]][1], global_replacements[[i]][2], code)
    }
    writeLines(text = code, con = f)
  }
}

insert_before <- list(
  # "src/cl/asdf.c" = list("", "")
)

for (i in 1L:length(insert_before)){
  f <- path(repodir, names(insert_before)[[i]])
  code <- readLines(f)
  position <- grep(pattern = insert_before[[i]][[1]], code)[1]
  code <- c(code[1L:(position - 1L)], insert_before[[i]][[2]], code[position:length(code)])
  writeLines(text = code, con = f)
}