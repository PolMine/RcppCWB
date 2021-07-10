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
  "src/cwb/cl/attributes.c" = list(
    "^#include\\s<ctype\\.h>",
    c("void Rprintf(const char *, ...);", "")
  )
)

for (i in 1L:length(insert_before)){
  fname <- path(repodir, names(insert_before)[[i]])
  code <- readLines(fname)
  position <- grep(pattern = insert_before[[i]][[1]], code)[1]
  if (!is.na(position)){
    code <- c(
      code[1L:(position - 1L)],
      insert_before[[i]][[2]],
      code[position:length(code)]
    )
    writeLines(text = code, con = fname)
  }
}

replace <- list(
  "src/cwb/cl/attributes.c" = c("(\\s+)int\\sppos,\\sbpos,\\sdollar,\\srpos;", "\\1int ppos, bpos, rpos;"),
  "src/cwb/cl/attributes.c" = c("^(\\s+)dollar = 0;", "\\1/* dollar = 0; */"),
  "src/cwb/cl/attributes.c" = c("^(\\s+)dollar = ppos;", "\\1/* dollar = ppos; */ "),
  
  "src/cwb/cl/attributes.c" = c('^(\\s+)if\\s\\(STREQ\\(rname,\\s"HOME"\\)\\)', '\\1if (strcmp(rname, "HOME") == 0)'),
  "src/cwb/cl/attributes.c" = c('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"APATH"\\)\\)', '\\1else if (strcmp(rname, "APATH") == 0)'),
  "src/cwb/cl/attributes.c" = c('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"ANAME"\\)\\)', '\\1else if (strcmp(rname, "ANAME") == 0)')
)

for (i in 1L:length(replace)){
  fname <- path(repodir, names(replace)[[i]])
  code <- readLines(fname)
  position <- grep(pattern = replace[[i]][[1]], code)[1]
  if (!is.na(position)){
    code[position] <- gsub(replace[[i]][1], replace[[i]][2], code[position])
    writeLines(text = code, con = fname)
  }
}