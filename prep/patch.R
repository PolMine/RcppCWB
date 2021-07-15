#!/usr/bin/env Rscript 

library(RcppCWB)

repodir <- "~/Lab/github/RcppCWB"
cwb_pkg_dir <- "~/Lab/github/RcppCWB/src/cwb"

setwd(path(cwb_pkg_dir, "cl"))
# system("bison -d -t -p creg registry.y")
system("bison -d -t registry.y")
system("flex -8 -Pcreg registry.l")

setwd(path(cwb_pkg_dir, "cqp"))
# system("bison -d -t -p creg parser.y")
system("bison -d -t parser.y")
system("flex -8 parser.l")

global_replacements <- list(
  c("(vf|f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd->stream|redir->stream),\\s*", "Rprintf("),
  c("YY(F|D)PRINTF\\s*(\\({1,2})\\s*(stderr|yyoutput),\\s*" , "YY\\1PRINTF \\2"),
  c("fprintf\\s*\\(", "Rprintf("),
  c("(\\s+)printf\\(", "\\1Rprintf("),
  c("# define YYFPRINTF fprintf", "# define YYFPRINTF Rprintf"),
  
  c('^\\s*#include\\s+"endian\\.h"\\s*$', '#include "endian2.h"') # only files in cl, maybe limit this
)

for (subdir in c("cl", "cqp", "CQi")){
  files <- list.files(file.path(cwb_pkg_dir, subdir), full.names = TRUE)
  for (f in files){
    code <- readLines(f)
    for (i in 1:length(global_replacements)){
      code <- gsub(global_replacements[[i]][1], global_replacements[[i]][2], code)
    }
    writeLines(text = code, con = f)
  }
}

delete_line_before <- list(
  "src/cwb/cqp/groups.c" = list("^Group\\s\\*", 3L),
  "src/cwb/cqp/groups.c" = list("^Group\\s\\*compute_grouping\\(CorpusList\\s\\*cl,", 1L),
  "src/cwb/cqp/groups.c" = list("^Group\\s\\*compute_grouping\\(CorpusList\\s\\*cl,", 1L),
  "src/cwb/cqp/groups.c" = list("^\\s*if\\s\\(\\(fd\\s=\\sopen_temporary_file\\(temporary_name\\)\\)\\s==\\sNULL\\)\\s\\{", 1L),
  "src/cwb/cqp/groups.c" = list("^\\s*if\\s\\(\\(fd\\s=\\sopen_temporary_file\\(temporary_name\\)\\)\\s==\\sNULL\\)\\s\\{", 1L),
  "src/cwb/cqp/groups.c" = list("^(\\s*)sprintf\\(sort_call,\\sExternalGroupingCommand,\\stemporary_name\\);", 1L),
  "src/cwb/cqp/groups.c" = list("^\\s*return\\sComputeGroupExternally\\(group\\);", 1L),
  "src/cwb/cqp/output.c" = list('^\\s*sprintf\\(prefix,\\s"cqpt\\.%d",\\s\\(unsigned\\sint\\)getpid\\(\\)\\);', 1L, 8L),
  "src/cwb/cqp/parser.y" = list("^\\s*if\\s\\(\\$2\\s&&\\sgenerate_code\\)\\s\\{", 3L),
  "src/cwb/cqp/ranges.c" = list("^\\s*for\\s\\(i\\s=\\s0;\\si\\s<\\sp;\\si\\+\\+\\)", 2L, 5L),
  "src/cwb/cqp/ranges.c" = list("^\\s*for\\s\\(i\\s=\\s0;\\si\\s<\\sp;\\si\\+\\+\\)", 1L, 5L),
  "src/cwb/cqp/ranges.c" = list("^\\s*value\\s=\\scl_string_canonical\\(value,\\ssrt_cl->corpus->charset,\\ssrt_flags,\\sCL_STRING_CANONICAL_STRDUP\\);", 1L, 7L),
  "src/cwb/cl/lex.creg.c" = list("^\\s*if\\s\\(\\s\\(yy_c_buf_p\\)", 4L, 4L),
  "src/cwb/cl/lex.creg.c" = list("^\\s*cregrestart\\(cregin\\s*\\);", 2L, 11L)
)



for (i in 1L:length(delete_line_before)){
  fname <- path(repodir, names(delete_line_before)[[i]])
  code <- readLines(fname)
  which_position <- if (length(delete_line_before[[i]]) > 1L) delete_line_before[[i]][[2]] else 1L
  position <- grep(pattern = delete_line_before[[i]][[1]], code)[which_position]
  times <- if (length(delete_line_before[[i]]) == 3L) delete_line_before[[i]][[3]] else 1L
  if (!is.na(position)){
    code <- code[-(position - 1L:times)]
    writeLines(text = code, con = fname)
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
  "src/cwb/cl/storage.c" = list("^(\\s*)lseek\\(fd,\\s0,\\sSEEK_SET\\);", '      if (success < 0) Rprintf("Operation not successful");'),
  "src/cwb/cl/windows-mmap.c" = list('^#include\\s"windows-mmap\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cqp/groups.c" = list("^Group\\s\\*", "/*", 3), # begin of commenting out ComputeGroupExternally(Group *group)
  "src/cwb/cqp/groups.c" = list("^Group\\s\\*compute_grouping\\(CorpusList\\s\\*cl,", c("*/", "")), # end of commenting out ComputeGroupExternally(Group *group)
  
  "src/cwb/cqp/hash.c" = list("^\\s*int\\s*$", "/*", 1),
  "src/cwb/cqp/hash.c" = list("^\\s*int\\s*$", "/*", 2),
  "src/cwb/cqp/hash.c" = list("^unsigned\\sint\\s*$", "/*", 1),
  "src/cwb/cqp/macro.c" = list('#include\\s"hash\\.h"', '#include "../cl/lexhash.h" /* newly added by AB */', 1),
  "src/cwb/cqp/output.c" = list("^FILE\\s\\*\\s*", "/*", 1),
  "src/cwb/cqp/ranges.c" = list("^int", "/*", 9),
  "src/cwb/CQi/auth.c" = list("/\\*\\sdata\\sstructures\\s\\(internal\\suse\\sonly\\)\\s\\*/", c("void Rprintf(const char *, ...);", ""), 1),
  "src/cwb/CQi/server.c" = list("^\\/\\*", c("void Rprintf(const char *, ...);", ""), 3L),
  "src/cwb/utils/cwb-makeall.c" = list("/\\*\\*\\sThe\\scorpus\\swe\\sare\\sworking\\son\\s\\*/", c("#include <netinet/in.h>", ""), 1),
  "src/cwb/cl/lex.creg.c" = list("/\\*\\send\\sstandard\\sC\\sheaders\\.\\s\\*/", c("void Rprintf(const char *, ...);", ""), 1),
  "src/cwb/cl/lex.creg.c" = list("#ifndef\\sYY_NO_INPUT", "/*", 1L),
  "src/cwb/cl/lex.creg.c" = list("^\\s*static\\svoid\\syyunput\\s\\(int\\sc,\\sregister\\schar\\s\\*\\syy_bp\\s\\)", "/*", 1L),
  "src/cwb/cl/lex.creg.c" = list("^#ifndef\\sYY_NO_INPUT", "/*", 2L)
)

for (i in 1L:length(insert_before)){
  fname <- path(repodir, names(insert_before)[[i]])
  code <- readLines(fname)
  which_position <- if (length(insert_before[[i]]) > 2L) insert_before[[i]][[3]] else 1
  position <- grep(pattern = insert_before[[i]][[1]], code)[which_position]
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
  "src/cwb/cl/regopt.c" = list('^#include\\s"regopt\\.h"', "void Rprintf(const char *, ...);"),
  "src/cwb/cqp/groups.c" = list("return\\sComputeGroupInternally\\(group\\);", c("   */", "  return group;")),
  "src/cwb/cqp/hash.c" = list("^\\s*\\}\\s*$", "*/", 1),
  "src/cwb/cqp/hash.c" = list("^\\s*\\}\\s*$", "*/", 2),
  "src/cwb/cqp/hash.c" = list("^\\s*\\}\\s*$", "*/", 3),
  "src/cwb/cqp/output.c" = list("^\\}\\s*$", "*/", 3),
  "src/cwb/cqp/ranges.c" = list("^\\}\\s*$", "*/", 11),
  "src/cwb/CQi/cqpserver.c" = list('#include "\\.\\./cqp/groups\\.h"', "void Rprintf(const char *, ...);", 1),
  "src/cwb/CQi/server.c" = list('#include "\\.\\./cqp/hash\\.h"', '#include "../cl/lexhash.h" /* inserted by AB */', 1),
  "src/cwb/cl/lex.creg.c" = list("#endif", "*/", 28L),
  "src/cwb/cl/lex.creg.c" = list("^\\}$", "*/", 5L)
)

for (i in 1L:length(insert_after)){
  fname <- path(repodir, names(insert_after)[[i]])
  code <- readLines(fname)
  which_position <- if (length(insert_after[[i]]) > 2L) insert_after[[i]][[3]] else 1L
  position <- grep(pattern = insert_after[[i]][[1]], code)[which_position]
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
  
  "src/cwb/cl/storage.c" = list("^(\\s+)write\\(fd,\\s&fd,\\ssizeof\\(int\\)\\);", "\\1ssize_t success = write(fd, &fd, sizeof(int));", 1),
  "src/cwb/cqp/eval.c" = list("^(\\s*)if\\s\\(ctptr->pa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->pa_ref.del) {", 2),
  "src/cwb/cqp/eval.c" = list("^(\\s*)if\\s\\(ctptr->pa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->pa_ref.del) {", 1),
  
  "src/cwb/cqp/eval.c" = list("^(\\s*)if\\s\\(ctptr->sa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->sa_ref.del) {", 1),
  "src/cwb/cqp/eval.c" = list("^(\\s*)if\\s\\(ctptr->idlist\\.delete\\)\\s\\{", "\\1if (ctptr->idlist.del) {", 1),
  "src/cwb/cqp/eval.c" = list("^(\\s*)int\\ssmallest_col,\\scorpus_size;", "\\1int smallest_col;", 1),
  "src/cwb/cqp/eval.c" = list("^(\\s*)corpus_size\\s=\\sevalenv->query_corpus->mother_size;", "\\1/* corpus_size = evalenv->query_corpus->mother_size; */", 1),
  "src/cwb/cqp/eval.c" = list("^(\\s*)assert\\(col->type\\s=\\stabular\\);", "\\1assert((col->type = tabular));", 1),
  "src/cwb/cqp/eval.h" = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 3),
  "src/cwb/cqp/eval.h" = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 2),
  "src/cwb/cqp/eval.h" = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 1),
  "src/cwb/cqp/groups.c" = list("^(.*?)\\s*/\\*\\s\\(source\\sID,\\starget\\sID)\\s*\\*/", "\\1", 1),
  "src/cwb/cqp/groups.c" = list("^(.*?)\\s*/\\*\\smodifies\\sGroup\\sobject\\sin\\splace\\sand\\sreturns\\spointer\\s\\*/", "\\1", 1),
  "src/cwb/cqp/groups.c" = list("^(\\s*)(if\\s\\(UseExternalGrouping\\s&&\\s\\!insecure\\s&&\\s\\!\\(source_is_struc\\s\\|\\|\\starget_is_struc\\s\\|\\|\\sis_grouped\\)\\))", "\\1/* \\2", 1),
  "src/cwb/cqp/hash.c" = list("^(.*?)\\s*/\\*\\swill\\sexit\\son\\sint\\soverflow\\s\\*/", "\\1", 1),
  "src/cwb/cqp/hash.h" = list("(\\s*int\\sis_prime\\(int\\sn\\);)", "/* \\1 */", 1),
  "src/cwb/cqp/hash.h" = list("^(\\s*int\\sfind_prime\\(int\\sn\\);)", "/* \\1 */", 1),
  "src/cwb/cqp/hash.h" = list("^(\\s*unsigned\\sint\\shash_string\\(char\\s\\*s\\);)", "/* \\1 */", 1),
  "src/cwb/cqp/html-print.c" = list("^(\\s*)char\\s\\*s;", "\\1/* char *s; */", 1),
  "src/cwb/cqp/html-print.c" = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
  "src/cwb/cqp/html-print.c" = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
  "src/cwb/cqp/html-print.c" = list("^(\\s*)AttributeList\\s\\*strucs;", "\\1/* AttributeList *strucs; */", 1),
  "src/cwb/cqp/html-print.c" = list("^(\\s*)strucs\\s=\\scd->printStructureTags;", "\\1/* strucs = cd->printStructureTags; */", 1),
  "src/cwb/cqp/latex-print.c" = list("^(\\s*)char\\s\\*s;", "\\1/* char *s; */", 1),
  "src/cwb/cqp/latex-print.c" = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
  "src/cwb/cqp/latex-print.c" = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
  "src/cwb/cqp/options.h" = list("^(\\s*)enum\\s_which_app\\s\\{\\sundef,\\scqp,\\scqpcl,\\scqpserver}\\swhich_app;", "\\1enum _which_app { undef, cqp, cqpcl, cqpserver} extern which_app;", 1),
  "src/cwb/cqp/output.c" = list('^(.*?)\\s*/\\*\\sholds\\s"cqpt\\.\\$\\$",\\sso\\s64\\schars\\sis\\splenty\\sof\\sheadroom\\s\\*/', "\\1", 1),
  "src/cwb/cqp/output.c" = list('^(.*?)(\\s*)/\\*\\s"cqpt\\.\\$\\$"\\s\\*/', "\\1", 1),
  "src/cwb/cqp/output.c" = list("^(.*?)\\s/\\*\\sstring\\sis\\sallocated\\sby\\stempnam\\(\\),\\sneeds\\sto\\sbe\\sfree'd\\sbelow\\s\\*/", "\\1", 1),
  "src/cwb/cqp/output.h" = list("^(\\s*)FILE\\s\\*open_temporary_file\\(char\\s\\*tmp_name_buffer);", "\\1/* FILE *open_temporary_file(char *tmp_name_buffer); */", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)c->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1c->idlist.del = left->pa_ref.del;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)left->pa_ref\\.delete\\s=\\s0;", "\\1left->pa_ref.del = 0;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)node->idlist\\.delete\\s=\\s0;", "\\1node->idlist.del = 0;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)res->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1res->idlist.del = left->pa_ref.del;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)res->pa_ref\\.delete\\s=\\sauto_delete;", "\\1res->pa_ref.del = auto_delete;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)res->sa_ref\\.delete\\s=\\sauto_delete;", "\\1res->sa_ref.del = auto_delete;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)res->pa_ref\\.delete\\s=\\s0;", "\\1res->pa_ref.del = 0;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)res->pa_ref\\.delete\\s=\\sauto_delete;", "\\1res->pa_ref.del = auto_delete;", 1),
  "src/cwb/cqp/parse_actions.c" = list("^(\\s*)res->sa_ref\\.delete\\s=\\s0;", "\\1res->sa_ref.del = 0;", 1),
  "src/cwb/cqp/parser.y" = list("^(\\s*)int\\sok;", "\\1int ok __attribute__((unused));", 3),
  "src/cwb/cqp/parser.y" = list("^(\\s*)int\\sok;", "\\1int ok __attribute__((unused));", 2),
  "src/cwb/cqp/parser.y" = list("^(\\s*)ok\\s=\\sSortSubcorpus\\(\\$2,\\s\\$3,\\s\\(\\$4\\s>=\\s1\\)\\s\\?\\s\\$4\\s:\\s1,\\s&\\(\\$5\\)\\);", "\\1SortSubcorpus($2, $3, ($4 >= 1) ? $4 : 1, &($5));", 1),
  "src/cwb/cqp/ranges.c" = list("^(\\s*)line\\s=\\s-1;(\\s*)\\s/\\*\\swill\\sindicate\\ssort\\sfailure\\sbelow\\sif\\stext_size\\s==\\s0\\s\\*/", "\\1line = -1;\\2", 1),
  "src/cwb/cqp/ranges.c" = list("^(\\s*)\\}(\\s)/\\*\\send\\sfor\\seach\\stoken\\s\\*/", "\\1}\\2", 1),
  "src/cwb/cqp/ranges.c" = list("^(\\s*)line\\s=\\s-1;(\\s*)/\\*\\swill\\sindicate\\sfailure\\sof\\sexternal\\ssort\\scommand\\s*\\*/", "\\1line = -1;\\2", 1),
  "src/cwb/cqp/ranges.c" = list("^(\\s*)break;\\s*/\\*\\sabort\\s\\*/", "\\1break;", 1),
  "src/cwb/cqp/ranges.c" = list("^(\\s*)ok\\s=\\sSortExternally\\(\\);", "\\1/* ok = SortExternally(); */", 1),
  # "src/cwb/cqp/regex2dfa.c" = list("^(\\s*)va_start\\(AP,\\sFormat\\);\\vRprintf(Format, AP); va_end(AP);", "\\1va_start(AP, Format); Rprintf(Format, AP); va_end(AP);", 1),
  "src/cwb/cqp/regex2dfa.c" = list("^(\\s*)int\\signore_value;", "\\1int ignore_value __attribute__((unused));", 1),
  "src/cwb/cqp/sgml-print.c" = list("^(\\s*)AttributeList\\s\\*strucs;", "\\1/* AttributeList *strucs; */", 1),
  "src/cwb/cqp/sgml-print.c" = list("^(\\s*)strucs\\s=\\scd->printStructureTags;", "\\1/* strucs = cd->printStructureTags; */", 1),
  "src/cwb/cl/lex.creg.c" = list("^(\\s*)(static\\svoid\\syyunput\\s\\(int\\sc,char\\s\\*buf_ptr\\s+\\);).*?$", "\\1/* \\2 */", 1),
  "src/cwb/cl/lex.creg.c" = list("^\\s*static\\svoid\\syyunput\\s\\(int\\sc,\\sregister\\schar\\s\\*\\syy_bp\\s\\)", "static void yyunput (int c, register char * yy_bp )", 1L),
  "src/cwb/cl/lex.creg.c" = list("^(\\s)\\{(\\s*)/\\*\\sneed\\smore\\sinput\\s\\*/", "\\1{\\2", 1L),
  "src/cwb/cl/lex.creg.c" = list("^(\\s*)\\{(\\s*)/\\*\\sneed\\smore\\sinput\\s\\*/", "\\1{\\2", 1L),
  "src/cwb/cl/lex.creg.c" = list("/\\*\\scast\\sfor\\s8-bit\\schar's\\s\\*/", "", 1L),
  "src/cwb/cl/lex.creg.c" = list("\\s*/\\*\\spreserve\\scregtext\\s\\*/", "", 1L),
  "src/cwb/cl/lex.creg.c" = list("/\\*\\sifndef YY_NO_INPUT\\s\\*/", "", 1L)
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
  "src/cwb/cl/corpus.c" = list("(\\s+)stderr,", 1),
  "src/cwb/cqp/ranges.c" = list("^\\s*/\\*\\suses\\ssettings\\sfrom\\sstatic\\ssrt_\\*\\svariables\\s\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("^\\s*/\\*\\sdetermine\\sstart\\sand\\send\\sposition\\sof\\ssort\\sinterval\\sfor\\sthis\\smatch\\s\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\sadjust\\ssort\\sboundaries.*?\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/*\\sswap\\sstart\\sand\\send\\sof\\sinterval.*?\\*/", 1),
  
  "src/cwb/cqp/ranges.c" = list("/\\*\\sdetermine\\ssort\\sdirection\\s\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\show\\smany\\stokens\\sto\\sprint\\s\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\swhen\\susing\\sflags,\\sprint\\snormalised\\stoken\\ssequence.*?\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\sprint\\ssequence\\sof\\stokens\\sin\\ssort\\sinterval\\s\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\snow,\\sexecute\\sthe\\sexternal\\ssort\\scommand.*?\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\srun\\ssort\\scmd\\sand\\sread\\sfrom\\spipe\\s\\*/", 1),
  "src/cwb/cqp/ranges.c" = list("/\\*\\snow\\swe\\sshould\\shave\\sread\\sexactly.*?\\*/", 1),
  "src/cwb/cl/lex.creg.c" = list("^\\s/\\*\\sundo\\seffects\\sof\\ssetting\\sup\\scregtext\\s\\*/", 1L),
  "src/cwb/cl/lex.creg.c" = list("/\\*\\sneed\\sto\\sshift\\sthings\\sup\\sto\\smake\\sroom\\s\\*/", 1L),
  "src/cwb/cl/lex.creg.c" = list("/\\*\\s\\+2\\sfor\\sEOB\\schars\\.\\s\\*/", 1L),
  "src/cwb/cl/lex.creg.c" = list("^\\s*/\\*\\sThis\\swas\\sreally\\sa\\sNUL\\.\\s\\*/", 1L),
  "src/cwb/cl/lex.creg.c" = list("^\\s*/\\*FALLTHROUGH\\*/\\s*$", 1L)
  
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

extern <- list(
  
  "src/cwb/cqp/corpmanag.c" = c(
    "CorpusList *corpuslist;"
  ),
  
  "src/cwb/cqp/corpmanag.h" = c(
    "CorpusList *current_corpus;",
    "CorpusList *corpuslist;"
  ),
  
  "src/cwb/cqp/cqp.h" = c(
    "CYCtype LastExpression;",
    "int exit_cqp;",
    "char *cqp_input_string;",
    "int cqp_input_string_position;",
    "int initialize_cqp(int argc, char **argv);",
    "int cqp_parse_file(FILE *fd, int exit_on_parse_errors);",
    "int cqp_parse_string(char *s);",
    "int EvaluationIsRunning;",
    "int setInterruptCallback(InterruptCheckProc f);",
    "void CheckForInterrupts(void);",
    "int signal_handler_is_installed;",
    "void install_signal_handler(void);"
  ),
  
  "src/cwb/cqp/eval.h" = c(
    "int eep;",
    "EvalEnvironment Environment[MAXENVIRONMENT];",
    "EEP CurEnv, evalenv;"
  ),
  
  "src/cwb/cqp/options.h" = c(
    "int insecure;",
    "int inhibit_activation;",
    "int parseonly;",
    "int verbose_parser;",
    "int show_symtab;",
    "int show_gconstraints;",
    "int show_evaltree;",
    "int show_patlist;",
    "int show_compdfa;",
    "int show_dfa;",
    "int symtab_debug;",
    "int parser_debug;",
    "int tree_debug;",
    "int eval_debug;",
    "int search_debug;",
    "int initial_matchlist_debug;",
    "int debug_simulation;",
    "int activate_cl_debug;",
    "int server_log;",
    "int server_debug;",
    "int snoop;",
    "int private_server;",
    "int server_port;",
    "int localhost;",
    "int server_quit;",
    "int query_lock;",
    "int query_lock_violation;",
    "int enable_macros;",
    "int macro_debug;",
    "int hard_boundary;",
    "int hard_cut;",
    "int subquery;",
    "char *def_unbr_attr;",
    "int query_optimize;",
    "enum _matching_strategy { traditional, shortest_match, standard_match, longest_match } matching_strategy;",
    "char *matching_strategy_name;",
    "int strict_regions;",
    "int use_readline;",
    "int highlighting;",
    "int paging;",
    "char *pager;",
    "char *tested_pager;",
    "char *less_charset_variable;",
    "int use_colour;",
    "int progress_bar;",
    "int pretty_print;",
    "int autoshow;",
    "int timing;",
    "int show_tag_attributes;",
    "int show_targets;",
    "char *printModeString;",
    "char *printModeOptions;",
    "int printNrMatches;",
    "char *printStructure;",
    "char *left_delimiter;",
    "char *right_delimiter;",
    "char *registry;",
    "char *LOCAL_CORP_PATH;",
    "int auto_save;",
    "int save_on_exit;",
    "char *cqp_init_file;",
    "char *macro_init_file;",
    "char *cqp_history_file;",
    "int write_history_file;",
    "int batchmode;",
    "int silent;",
    "char *default_corpus;",
    "char *query_string;",
    "int UseExternalSorting;",
    "char *ExternalSortingCommand;",
    "int UseExternalGrouping;",
    "char *ExternalGroupingCommand;",
    "int user_level;",
    "int rangeoutput;",
    "int child_process;",
    "ContextDescriptor CD;",
    "int handle_sigpipe;",
    "char *progname;",
    "char *licensee;",
    "FILE *batchfd;"
  )
)

lapply(
  names(extern),
  function(f){
    fname <- path(repodir, f)
    code <- readLines(fname)
    for (ext in extern[[f]]){
      for (position in which(startsWith(code, ext))){
        code[position] <- paste("extern", code[position], sep = " ")
      }
    }
    writeLines(text = code, con = fname)
  }
)


