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
  "src/cwb/cl/attributes.c" = list("^#include\\s<ctype\\.h>", c("void Rprintf(const char *, ...);", "")),
  "src/cwb/cl/bitio.c" = list("^#include\\s<sys/types\\.h>", "void Rprintf(const char *, ...);")
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

insert_after <- list(
  "src/cwb/cl/bitio.c" = list("^static\\sint\\sBaseTypeBits", "void Rprintf(const char *, ...);"),
  "src/cwb/cl/bitfields.c" = list("^static\\sint\\sBaseTypeBits", "void Rprintf(const char *, ...);"),
  "src/cwb/cl/cdaccess.c" = list('^#include\\s"cdaccess\\.h"', "void Rprintf(const char *, ...);")
)

for (i in 1L:length(insert_after)){
  fname <- path(repodir, names(insert_after)[[i]])
  code <- readLines(fname)
  position <- grep(pattern = insert_after[[i]][[1]], code)[1]
  if (!is.na(position)){
    code <- c(
      code[1L:position],
      insert_after[[i]][[2]],
      code[(position + 1L):length(code)]
    )
    writeLines(text = code, con = fname)
  }
}


replace <- list(
  "src/cwb/cl/attributes.c" = c("(\\s+)int\\sppos,\\sbpos,\\sdollar,\\srpos;", "\\1int ppos, bpos, rpos;"),
  "src/cwb/cl/attributes.c" = c("^(\\s+)dollar = 0;", "\\1/* dollar = 0; */"),
  "src/cwb/cl/attributes.c" = c("^(\\s+)dollar = ppos;\\s", "\\1/* dollar = ppos; */"),
  
  "src/cwb/cl/attributes.c" = c('^(\\s+)if\\s\\(STREQ\\(rname,\\s"HOME"\\)\\)', '\\1if (strcmp(rname, "HOME") == 0)'),
  "src/cwb/cl/attributes.c" = c('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"APATH"\\)\\)', '\\1else if (strcmp(rname, "APATH") == 0)'),
  "src/cwb/cl/attributes.c" = c('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"ANAME"\\)\\)', '\\1else if (strcmp(rname, "ANAME") == 0)'),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)int\\sregex_result,\\sidx,\\si,\\slen,\\slexsize;", "\\1int idx, i, lexsize;"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)int\\soptimised,\\sgrain_match;", "\\1int optimised;"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)char\\s\\*word,\\s\\*preprocessed_string;", "\\1char *word;"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)int\\soff_start,\\soff_end;", "\\1int off_start;"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)char *p;", "\\1/* char *p; */"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)int\\si;", "\\1/* int i; */"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)DynCallResult\\sarg;", "\\1/* DynCallResult arg; */"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)arg\\s=\\sargs\\[argnum\\];", "\\1/* arg = args[argnum]; */"),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)fgets\\(call,\\sCL_MAX_LINE_LENGTH,\\spipe\\);", '\\1if (fgets(call, CL_MAX_LINE_LENGTH, pipe) == NULL) Rprintf("fgets failure");'),
  "src/cwb/cl/cdaccess.c" = c("^(\\s*)off_end\\s=\\sntohl\\(lexidx_data\\[idx\\s+\\s1\\]\\)\\s-\\s1;", "\\1/* off_end = ntohl(lexidx_data[idx + 1]) - 1; */")
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