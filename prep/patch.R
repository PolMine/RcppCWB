#!/usr/bin/env Rscript 

cwb_svn_dir <- "/Users/andreasblaette/Lab/tmp/cwb/trunk"
cwb_pkg_dir <- "~/Lab/github/RcppCWB/src/cwb"

file.remove(list.files(cwb_pkg_dir, full.names = TRUE))
file.copy(from = cwb_svn_dir, to = pkg_dir, recursive = TRUE)
  
global_replacements <- list(
  c("(f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd-stream|redir->stream),", "Rprintf"),
  c("YY(F|D)PRINTF\\s*\\(\s*(stderr|yyoutput)," , "YY\\1PRINTF ("),
  c("fprintf.", "Rprintf"),
  c("#  define YYFPRINTF fprintf", "# define YYFPRINTF Rprintf")
)

for (dir in c("cl", "cqp", "CQi")){
  files <- list.files(file.path(pkg_dir, dir), full.names = TRUE)
  for (f in files){
    code <- readLines(f)
    for (i in length(global_replacements)){
      code <- gsub(global_replacements[[i]][1], global_replacements[[i]][2], code)
    }
    writeLines(text = code, con = f)
  }
}

