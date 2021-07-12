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
  "src/cwb/cl/bitio.c" = list("^#include\\s<sys/types\\.h>", "void Rprintf(const char *, ...);"),
  "src/cwb/cl/class-mapping.c" = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/corpus.c" = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);"),
  
  "src/cwb/cl/fileutils.c" = list('^#include\\s<sys/stat\\.h>', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/fileutils.c" = list('^#include\\s"globals\\.h"', c("#include <signal.h> /* added by Andreas Blaette  */", "#include <sys/socket.h> /* added by Andreas Blaette */", "")),
  "src/cwb/cl/globals.h" = list('^#ifndef\\s_globals_h_', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/lexhash.c" = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/macros.c" = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/makecomps.c" = list('#include\\s<ctype\\.h>', c("void Rprintf(const char *, ...);", "")),
  "src/cwb/cl/registry.y" = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/special-chars.c" = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/storage.c" = list('^#include\\s<sys/types\\.h>', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/storage.c" = list("^(\\s*)lseek\\(fd,\\s0,\\sSEEK_SET\\);", 'if (success < 0) Rprintf("Operation not successful");')
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
  "src/cwb/cl/cdaccess.c" = list('^#include\\s"cdaccess\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/globals.c" = list('^#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cl/lexhash.c" = list('^#include\\s"lexhash\\.h"', "#include <unistd.h>"),
  "src/cwb/cl/ngram-hash.c" = list("^#include\\s<math\\.h>", "void Rprintf(const char *, ...);"),
  "src/cwb/cl/regopt.c" = list('^#include\\s"regopt\\.h"', "void Rprintf(const char *, ...);")
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
  "src/cwb/cl/attributes.c" = list("(\\s+)int\\sppos,\\sbpos,\\sdollar,\\srpos;", "\\1int ppos, bpos, rpos;", 1),
  "src/cwb/cl/attributes.c" = list("^(\\s+)dollar = 0;", "\\1/* dollar = 0; */", 1),
  "src/cwb/cl/attributes.c" = list("^(\\s+)dollar = ppos;\\s", "\\1/* dollar = ppos; */", 1),
  
  "src/cwb/cl/attributes.c" = list('^(\\s+)if\\s\\(STREQ\\(rname,\\s"HOME"\\)\\)', '\\1if (strcmp(rname, "HOME") == 0)', 1),
  "src/cwb/cl/attributes.c" = list('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"APATH"\\)\\)', '\\1else if (strcmp(rname, "APATH") == 0)', 1),
  "src/cwb/cl/attributes.c" = list('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"ANAME"\\)\\)', '\\1else if (strcmp(rname, "ANAME") == 0)', 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)int\\sregex_result,\\sidx,\\si,\\slen,\\slexsize;", "\\1int idx, i, lexsize;", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)int\\soptimised,\\sgrain_match;", "\\1int optimised;", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)char\\s\\*word,\\s\\*preprocessed_string;", "\\1char *word;", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)int\\soff_start,\\soff_end;", "\\1int off_start;", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)char\\s\\*p;", "\\1/* char *p; */", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)int\\si;", "\\1/* int i; */", 3),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)DynCallResult\\sarg;", "\\1/* DynCallResult arg; */", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)arg\\s=\\sargs\\[argnum\\];", "\\1/* arg = args[argnum]; */", 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)fgets\\(call,\\sCL_MAX_LINE_LENGTH,\\spipe\\);", '\\1if (fgets(call, CL_MAX_LINE_LENGTH, pipe) == NULL) Rprintf("fgets failure");', 1),
  "src/cwb/cl/cdaccess.c" = list("^(\\s*)off_end\\s=\\sntohl\\(lexidx_data\\[idx\\s\\+\\s1\\]\\)\\s-\\s1;", "\\1/* off_end = ntohl(lexidx_data[idx + 1]) - 1; */", 1),
  
  "src/cwb/cl/class-mapping.c" = list("(\\s*)\\*\\s@param\\sclass\\s+The\\sclass\\s+to\\scheck\\.", "\\1* @param obj  The class to check.", 2),
  "src/cwb/cl/class-mapping.c" = list("(\\s*)\\*\\s@param\\sclass\\s+The\\sclass\\s+to\\scheck\\.", "\\1* @param obj  The class to check.", 1),
  "src/cwb/cl/class-mapping.c" = list("^(\\s+)return\\smember_of_class_i\\(map,\\sclass,\\sid\\);", "\\1return member_of_class_i(map, obj, id);", 1),
  "src/cwb/cl/class-mapping.c" = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 2),
  "src/cwb/cl/class-mapping.c" = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 1),
  "src/cwb/cl/class-mapping.c" = list("^(\\s+)class->tokens,", "\\1obj->tokens,", 1),
  "src/cwb/cl/class-mapping.c" = list("^(\\s+)class->nr_tokens,", "\\1obj->nr_tokens,", 1),
  "src/cwb/cl/class-mapping.h" = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 2),
  "src/cwb/cl/class-mapping.h" = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 1),
  "src/cwb/cl/makecomps.c" = list("^(\\s*)char\\serrmsg\\[CL_MAX_LINE_LENGTH\\];", "/* char errmsg[CL_MAX_LINE_LENGTH]; */", 1),
  "src/cwb/cl/ngram-hash.c" = list("^(\\s*)cl_ngram_hash_entry\\sentry,\\stemp;", "\\1cl_ngram_hash_entry entry;", 1),
  "src/cwb/cl/ngram-hash.c" = list("^(\\s*)temp\\s=\\sentry;", "\\1/* temp = entry; */", 1),
  "src/cwb/cl/regopt.c" = list("^(\\s*)if\\s\\(ch\\s>=\\s32\\s&\\sch\\s<\\s127\\)", "\\1if ((ch >= 32) & (ch < 127))", 1),
  
  "src/cwb/cl/storage.c" = list("^(\\s+)write\\(fd,\\s&fd,\\ssizeof\\(int\\)\\);", "\\1ssize_t success = write(fd, &fd, sizeof(int));", 1)
)

for (i in 1L:length(replace)){
  fname <- path(repodir, names(replace)[[i]])
  code <- readLines(fname)
  position <- grep(pattern = replace[[i]][[1]], code)[ replace[[i]][[3]] ]
  if (!is.na(position)){
    code[position] <- gsub(replace[[i]][1], replace[[i]][2], code[position])
    writeLines(text = code, con = fname)
  }
}

remove_lines <- list(
  "src/cwb/cl/corpus.c" = list("(\\s+)stderr,", 3),
  "src/cwb/cl/corpus.c" = list("(\\s+)stderr,", 2),
  "src/cwb/cl/corpus.c" = list("(\\s+)stderr,", 1)
)

for (i in 1L:length(remove_lines)){
  fname <- path(repodir, names(remove_lines)[[i]])
  code <- readLines(fname)
  position <- grep(pattern = remove_lines[[i]][[1]], code)[ remove_lines[[i]][[2]] ]
  if (!is.na(position)){
    code <- code[-position]
    writeLines(text = code, con = fname)
  }
}