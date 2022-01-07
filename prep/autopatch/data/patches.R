# Order to maintain:
# - delete_line_before
# - insert_before
# - insert_after
# - replace
# - remove_lines

file_patches <- list(
  
  "src/cwb/cl/lex.creg.c" = list(
    delete_line_before = list("^\\s*if\\s\\(\\s\\(yy_c_buf_p\\)", 4L, 4L),
    delete_line_before = list("^\\s*cregrestart\\(cregin\\s*\\);", 2L, 11L),
    delete_line_before = list("/\\*\\*\\sImmediately\\sswitch\\sto\\sa\\sdifferent\\sinput\\sstream\\.", 1L, 1L),
    insert_before = list("/\\*\\send\\sstandard\\sC\\sheaders\\.\\s\\*/", c("void Rprintf(const char *, ...);", ""), 1),
    insert_before = list("#ifndef\\sYY_NO_INPUT", "/*", 1L),
    insert_before = list("^\\s*static\\svoid\\syyunput\\s\\(int\\sc,\\sregister\\schar\\s\\*\\syy_bp\\s\\)", "/*", 1L),
    insert_before = list("^#ifndef\\sYY_NO_INPUT", "/*", 2L),
    insert_before = list("/\\*\\*\\sImmediately\\sswitch\\sto\\sa\\sdifferent\\sinput\\sstream\\.", c("*/", ""), 1L),
    insert_after = list("#endif", "*/", 28L),
    insert_after = list("^\\}$", "*/", 5L),
    replace = list("^(\\s*)(static\\svoid\\syyunput\\s\\(int\\sc,char\\s\\*buf_ptr\\s+\\);).*?$", "\\1/* \\2 */", 1),
    replace = list("^\\s*static\\svoid\\syyunput\\s\\(int\\sc,\\sregister\\schar\\s\\*\\syy_bp\\s\\)", "static void yyunput (int c, register char * yy_bp )", 1L),
    replace = list("^(\\s*)\\{(\\s*)/\\*\\sneed\\smore\\sinput\\s\\*/", "\\1{\\2", 1L),
    replace = list("/\\*\\scast\\sfor\\s8-bit\\schar's\\s\\*/", "", 1L),
    replace = list("\\s*/\\*\\spreserve\\scregtext\\s\\*/", "", 1L),
    replace = list("/\\*\\sifndef YY_NO_INPUT\\s\\*/", "", 1L),
    remove_lines = list("^\\s/\\*\\sundo\\seffects\\sof\\ssetting\\sup\\scregtext\\s\\*/", 1L),
    remove_lines = list("/\\*\\sneed\\sto\\sshift\\sthings\\sup\\sto\\smake\\sroom\\s\\*/", 1L),
    remove_lines = list("/\\*\\s\\+2\\sfor\\sEOB\\schars\\.\\s\\*/", 1L),
    remove_lines = list("^\\s*/\\*\\sThis\\swas\\sreally\\sa\\sNUL\\.\\s\\*/", 1L),
    remove_lines = list("^\\s*/\\*FALLTHROUGH\\*/\\s*$", 1L)
  ),
  
  "src/cwb/cl/endian2.h" = list(
    delete_line_before = list("/* for consistency:", 1L, 1L),
    insert_before = list("#include\\s<windows\\.h>" , "#include <winsock2.h> /* AB reversed order, in original CWB code windows.h is included first */", 1L)
  ),
  
  "src/cwb/cl/Makefile" = list(
    delete_line_before = list("^libcl.a: \\$\\(OBJS\\)", 1L, 6L),
    delete_line_before = list("##\\scl\\.h\\sheader\\s", 1L, 11L),
    delete_line_before = list("^depend:$", 1L, 22L),
    replace = list("^TOP\\s=\\s\\$\\(shell\\spwd\\)/\\.\\.", "TOP = $(R_PACKAGE_SOURCE)", 1L),
    replace = list("^(\\s+)endian.h", "\\1endian2.h", 1L),
    replace = list("^(\\s+)\\$\\(AR\\)\\s", "\\1$(AR) cq ", 1L),
    remove_lines = list("^\\s+\\$\\(RANLIB\\)", 1L)
  ),
  
  "src/cwb/cl/attributes.c" = list(
    insert_before = list("^#include\\s<ctype\\.h>", c("void Rprintf(const char *, ...);", "")),
    replace = list("(\\s+)int\\sppos,\\sbpos,\\sdollar,\\srpos;", "\\1int ppos, bpos, rpos;", 1),
    replace = list("^(\\s+)dollar = 0;", "\\1/* dollar = 0; */", 1),
    replace = list("^(\\s+)dollar = ppos;\\s", "\\1/* dollar = ppos; */", 1),
    replace = list('^(\\s+)if\\s\\(STREQ\\(rname,\\s"HOME"\\)\\)', '\\1if (strcmp(rname, "HOME") == 0)', 1),
    replace = list('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"APATH"\\)\\)', '\\1else if (strcmp(rname, "APATH") == 0)', 1),
    replace = list('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"ANAME"\\)\\)', '\\1else if (strcmp(rname, "ANAME") == 0)', 1)
  ),
  
  "src/cwb/cl/bitio.c" = list(
    insert_before = list("^#include\\s<sys/types\\.h>", "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/class-mapping.c" = list(
    insert_before = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
    replace = list("(\\s*)\\*\\s@param\\sclass\\s+The\\sclass\\s+to\\scheck\\.", "\\1* @param obj  The class to check.", 2),
    replace = list("(\\s*)\\*\\s@param\\sclass\\s+The\\sclass\\s+to\\scheck\\.", "\\1* @param obj  The class to check.", 1),
    replace = list("^(\\s+)return\\smember_of_class_i\\(map,\\sclass,\\sid\\);", "\\1return member_of_class_i(map, obj, id);", 1),
    replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 2),
    replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 1),
    replace = list("^(\\s+)class->tokens,", "\\1obj->tokens,", 1),
    replace = list("^(\\s+)class->nr_tokens,", "\\1obj->nr_tokens,", 1)
  ),
  
  "src/cwb/cl/corpus.c" = list(
    insert_before = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);"),
    remove_lines = list("(\\s+)stderr,", 3),
    remove_lines = list("(\\s+)stderr,", 2),
    remove_lines = list("(\\s+)stderr,", 1)
  ),
  
  "src/cwb/cl/fileutils.c" = list(
    insert_before = list('^#include\\s<sys/stat\\.h>', "void Rprintf(const char *, ...);"),
    insert_before = list('^#include\\s"globals\\.h"', c("#include <signal.h> /* added by Andreas Blaette  */", "#include <sys/socket.h> /* added by Andreas Blaette */", ""))
  ),
  
  "src/cwb/cl/globals.h" = list(
    insert_before = list('^#ifndef\\s_globals_h_', "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/lexhash.c" = list(
    insert_before = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
    insert_after = list('^#include\\s"lexhash\\.h"', "#include <unistd.h>")
  ),
  
  "src/cwb/cl/macros.c" = list(
    insert_before = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/makecomps.c" = list(
    insert_before = list('#include\\s<ctype\\.h>', c("void Rprintf(const char *, ...);", "")),
    replace = list("^(\\s*)char\\serrmsg\\[CL_MAX_LINE_LENGTH\\];", "/* char errmsg[CL_MAX_LINE_LENGTH]; */", 1)
  ),
  
  "src/cwb/cl/registry.y" = list(
    insert_before = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/special-chars.c" = list(
    insert_before = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/storage.c" = list(
    insert_before = list('^#include\\s<sys/types\\.h>', "void Rprintf(const char *, ...);"),
    insert_before = list("^(\\s*)lseek\\(fd,\\s0,\\sSEEK_SET\\);", '      if (success < 0) Rprintf("Operation not successful");'),
    replace = list("^(\\s+)write\\(fd,\\s&fd,\\ssizeof\\(int\\)\\);", "\\1ssize_t success = write(fd, &fd, sizeof(int));", 1)
  ),
  
  "src/cwb/cl/windows-mmap.c" = list(
    insert_before = list('^#include\\s"windows-mmap\\.h"', "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/registry.tab.c" = list(
    insert_before = list("#define\\sYYBISON\\s1", "void Rprintf(const char *, ...);", 1L)
  ),
  
  "src/cwb/cl/bitfields.c" = list(
    insert_after = list("^static\\sint\\sBaseTypeBits", "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/cdaccess.c" = list(
    insert_after = list('^#include\\s"cdaccess\\.h"', "void Rprintf(const char *, ...);"),
    replace = list("^(\\s*)int\\sregex_result,\\sidx,\\si,\\slen,\\slexsize;", "\\1int idx, i, lexsize;", 1),
    replace = list("^(\\s*)int\\soptimised,\\sgrain_match;", "\\1int optimised;", 1),
    replace = list("^(\\s*)char\\s\\*word,\\s\\*preprocessed_string;", "\\1char *word;", 1),
    replace = list("^(\\s*)int\\soff_start,\\soff_end;", "\\1int off_start;", 1),
    replace = list("^(\\s*)char\\s\\*p;", "\\1/* char *p; */", 1),
    replace = list("^(\\s*)int\\si;", "\\1/* int i; */", 3),
    replace = list("^(\\s*)DynCallResult\\sarg;", "\\1/* DynCallResult arg; */", 1),
    replace = list("^(\\s*)arg\\s=\\sargs\\[argnum\\];", "\\1/* arg = args[argnum]; */", 1),
    replace = list("^(\\s*)fgets\\(call,\\sCL_MAX_LINE_LENGTH,\\spipe\\);", '\\1if (fgets(call, CL_MAX_LINE_LENGTH, pipe) == NULL) Rprintf("fgets failure");', 1),
    replace = list("^(\\s*)off_end\\s=\\sntohl\\(lexidx_data\\[idx\\s\\+\\s1\\]\\)\\s-\\s1;", "\\1/* off_end = ntohl(lexidx_data[idx + 1]) - 1; */", 1)
  ),
  
  "src/cwb/cl/globals.c" = list(
    insert_after = list('^#include\\s"globals\\.h"', "void Rprintf(const char *, ...);")
  ),
  
  "src/cwb/cl/ngram-hash.c" = list(
    insert_after = list("^#include\\s<math\\.h>", "void Rprintf(const char *, ...);"),
    replace = list("^(\\s*)cl_ngram_hash_entry\\sentry,\\stemp;", "\\1cl_ngram_hash_entry entry;", 1),
    replace = list("^(\\s*)temp\\s=\\sentry;", "\\1/* temp = entry; */", 1)
  ),
  
  "src/cwb/cl/regopt.c" = list(
    insert_after = list('^#include\\s"regopt\\.h"', "void Rprintf(const char *, ...);"),
    replace = list("^(\\s*)if\\s\\(ch\\s>=\\s32\\s&\\sch\\s<\\s127\\)", "\\1if ((ch >= 32) & (ch < 127))", 1)
  ),
  
  "src/cwb/cl/class-mapping.h" = list(
    replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 2),
    replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 1)
  ),
  
  "src/cwb/cqp/groups.c" = list(
    delete_line_before = list("^Group\\s\\*", 3L),
    delete_line_before = list("^Group\\s\\*compute_grouping\\(CorpusList\\s\\*cl,", 1L),
    delete_line_before = list("^Group\\s\\*compute_grouping\\(CorpusList\\s\\*cl,", 1L),
    delete_line_before = list("^\\s*if\\s\\(\\(fd\\s=\\sopen_temporary_file\\(temporary_name\\)\\)\\s==\\sNULL\\)\\s\\{", 1L),
    delete_line_before = list("^\\s*if\\s\\(\\(fd\\s=\\sopen_temporary_file\\(temporary_name\\)\\)\\s==\\sNULL\\)\\s\\{", 1L),
    delete_line_before = list("^(\\s*)sprintf\\(sort_call,\\sExternalGroupingCommand,\\stemporary_name\\);", 1L),
    delete_line_before = list("^\\s*return\\sComputeGroupExternally\\(group\\);", 1L),
    insert_before = list("^Group\\s\\*", "/*", 3), # begin of commenting out ComputeGroupExternally(Group *group),
    insert_before = list("^Group\\s\\*compute_grouping\\(CorpusList\\s\\*cl,", c("*/", "")), # end of commenting out ComputeGroupExternally(Group *group)
    insert_after = list("return\\sComputeGroupInternally\\(group\\);", c("   */", "  return group;")),
    replace = list("^(.*?)\\s*/\\*\\s\\(source\\sID,\\starget\\sID)\\s*\\*/", "\\1", 1),
    replace = list("^(.*?)\\s*/\\*\\smodifies\\sGroup\\sobject\\sin\\splace\\sand\\sreturns\\spointer\\s\\*/", "\\1", 1),
    replace = list("^(\\s*)(if\\s\\(UseExternalGrouping\\s&&\\s\\!insecure\\s&&\\s\\!\\(source_is_struc\\s\\|\\|\\starget_is_struc\\s\\|\\|\\sis_grouped\\)\\))", "\\1/* \\2", 1)
  ),
  
  "src/cwb/cqp/output.c" = list(
    delete_line_before = list('^\\s*sprintf\\(prefix,\\s"cqpt\\.%d",\\s\\(unsigned\\sint\\)getpid\\(\\)\\);', 1L, 8L),
    insert_before = list("^FILE\\s\\*\\s*", "/*", 1),
    insert_after = list("^\\}\\s*$", "*/", 3),
    replace = list('^(.*?)\\s*/\\*\\sholds\\s"cqpt\\.\\$\\$",\\sso\\s64\\schars\\sis\\splenty\\sof\\sheadroom\\s\\*/', "\\1", 1),
    replace = list('^(.*?)(\\s*)/\\*\\s"cqpt\\.\\$\\$"\\s\\*/', "\\1", 1),
    replace = list("^(.*?)\\s/\\*\\sstring\\sis\\sallocated\\sby\\stempnam\\(\\),\\sneeds\\sto\\sbe\\sfree'd\\sbelow\\s\\*/", "\\1", 1)
  ),
  
  "src/cwb/cqp/parser.y" = list(
    delete_line_before = list("^\\s*if\\s\\(\\$2\\s&&\\sgenerate_code\\)\\s\\{", 3L),
    replace = list("^(\\s*)int\\sok;", "\\1int ok __attribute__((unused));", 3),
    replace = list("^(\\s*)int\\sok;", "\\1int ok __attribute__((unused));", 2),
    replace = list("^(\\s*)ok\\s=\\sSortSubcorpus\\(\\$2,\\s\\$3,\\s\\(\\$4\\s>=\\s1\\)\\s\\?\\s\\$4\\s:\\s1,\\s&\\(\\$5\\)\\);", "\\1SortSubcorpus($2, $3, ($4 >= 1) ? $4 : 1, &($5));", 1)
  ),
  
  "src/cwb/cqp/ranges.c" = list(
    delete_line_before = list("^\\s*for\\s\\(i\\s=\\s0;\\si\\s<\\sp;\\si\\+\\+\\)", 2L, 5L),
    delete_line_before = list("^\\s*for\\s\\(i\\s=\\s0;\\si\\s<\\sp;\\si\\+\\+\\)", 1L, 5L),
    delete_line_before = list("^\\s*value\\s=\\scl_string_canonical\\(value,\\ssrt_cl->corpus->charset,\\ssrt_flags,\\sCL_STRING_CANONICAL_STRDUP\\);", 1L, 7L),
    insert_before = list("^int", "/*", 9),
    insert_after = list("^\\}\\s*$", "*/", 11),
    replace = list("^(\\s*)line\\s=\\s-1;(\\s*)\\s/\\*\\swill\\sindicate\\ssort\\sfailure\\sbelow\\sif\\stext_size\\s==\\s0\\s\\*/", "\\1line = -1;\\2", 1),
    replace = list("^(\\s*)\\}(\\s)/\\*\\send\\sfor\\seach\\stoken\\s\\*/", "\\1}\\2", 1),
    replace = list("^(\\s*)line\\s=\\s-1;(\\s*)/\\*\\swill\\sindicate\\sfailure\\sof\\sexternal\\ssort\\scommand\\s*\\*/", "\\1line = -1;\\2", 1),
    replace = list("^(\\s*)break;\\s*/\\*\\sabort\\s\\*/", "\\1break;", 1),
    replace = list("^(\\s*)ok\\s=\\sSortExternally\\(\\);", "\\1/* ok = SortExternally(); */", 1),
    # "src/cwb/cqp/regex2dfa.c" = list(
    #  replace = list("^(\\s*)va_start\\(AP,\\sFormat\\);\\vRprintf(Format, AP); va_end(AP);", "\\1va_start(AP, Format); Rprintf(Format, AP); va_end(AP);", 1),
    # ),
    remove_lines = list("^\\s*/\\*\\suses\\ssettings\\sfrom\\sstatic\\ssrt_\\*\\svariables\\s\\*/", 1),
    remove_lines = list("^\\s*/\\*\\sdetermine\\sstart\\sand\\send\\sposition\\sof\\ssort\\sinterval\\sfor\\sthis\\smatch\\s\\*/", 1),
    remove_lines = list("/\\*\\sadjust\\ssort\\sboundaries.*?\\*/", 1),
    remove_lines = list("/*\\sswap\\sstart\\sand\\send\\sof\\sinterval.*?\\*/", 1),
    remove_lines = list("/\\*\\sdetermine\\ssort\\sdirection\\s\\*/", 1),
    remove_lines = list("/\\*\\show\\smany\\stokens\\sto\\sprint\\s\\*/", 1),
    remove_lines = list("/\\*\\swhen\\susing\\sflags,\\sprint\\snormalised\\stoken\\ssequence.*?\\*/", 1),
    remove_lines = list("/\\*\\sprint\\ssequence\\sof\\stokens\\sin\\ssort\\sinterval\\s\\*/", 1),
    remove_lines = list("/\\*\\snow,\\sexecute\\sthe\\sexternal\\ssort\\scommand.*?\\*/", 1),
    remove_lines = list("/\\*\\srun\\ssort\\scmd\\sand\\sread\\sfrom\\spipe\\s\\*/", 1),
    remove_lines = list("/\\*\\snow\\swe\\sshould\\shave\\sread\\sexactly.*?\\*/", 1)
  ),
  
  "src/cwb/cqp/lex.yy.c" = list(
    delete_line_before = list("/\\*\\*\\sImmediately\\sswitch\\sto\\sa\\sdifferent\\sinput\\sstream\\.", 1L, 111L),
    insert_before = list("#ifdef __cplusplus", "/*", 3L),
    insert_before = list("/\\*\\*\\sImmediately\\sswitch\\sto\\sa\\sdifferent\\sinput\\sstream\\.", "", 1L),
    insert_after = list("#endif", "*/", 27L),
    replace = list("^\\s+static\\svoid\\syyunput\\s\\(int\\sc,char\\s\\*buf_ptr\\s+\\);", "  /*  static void yyunput (int c,char *buf_ptr  ); */", 1L)
  ),
  
  "src/cwb/cqp/Makefile" = list(
    delete_line_before = list("^clean:$", 1L, 7L),
    delete_line_before = list("^cqp\\$\\(EXEC_SUFFIX\\):", 1L, 8L),
    insert_before = list("^cqp\\$\\(EXEC_SUFFIX\\):.*", "libcqp.a: $(OBJS) $(CQI_OBJS)\n\t$(RM) $@\n\t$(AR) cq $@ $^\n", 1L),
    replace = list("all:\\s\\$\\(PROGRAMS\\)", "all: libcqp.a", 1L),
    replace = list("^TOP\\s=\\s\\$\\(shell\\spwd\\)/\\.\\.", "TOP = $(R_PACKAGE_SOURCE)", 1L),
    remove_lines = list("\\s+-\\$\\(RM\\)\\slex\\.yy\\.c\\sparser\\.tab\\.c\\sparser\\.tab\\.h", 1L)
  ),
  
  "src/cwb/cqp/hash.c" = list(
    insert_before = list("^\\s*int\\s*$", "/*", 1),
    insert_before = list("^\\s*int\\s*$", "/*", 2),
    insert_before = list("^unsigned\\sint\\s*$", "/*", 1),
    insert_after = list("^\\s*\\}\\s*$", "*/", 1),
    insert_after = list("^\\s*\\}\\s*$", "*/", 2),
    insert_after = list("^\\s*\\}\\s*$", "*/", 3),
    replace = list("^(.*?)\\s*/\\*\\swill\\sexit\\son\\sint\\soverflow\\s\\*/", "\\1", 1)
  ),
  
  "src/cwb/cqp/macro.c" = list(
    insert_before = list('#include\\s"hash\\.h"', '#include "../cl/lexhash.h" /* newly added by AB */', 1)
  ),
  
  "src/cwb/cqp/eval.c" = list(
    replace = list("^(\\s*)if\\s\\(ctptr->pa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->pa_ref.del) {", 2),
    replace = list("^(\\s*)if\\s\\(ctptr->pa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->pa_ref.del) {", 1),
    replace = list("^(\\s*)if\\s\\(ctptr->sa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->sa_ref.del) {", 1),
    replace = list("^(\\s*)if\\s\\(ctptr->idlist\\.delete\\)\\s\\{", "\\1if (ctptr->idlist.del) {", 1),
    replace = list("^(\\s*)int\\ssmallest_col,\\scorpus_size;", "\\1int smallest_col;", 1),
    replace = list("^(\\s*)corpus_size\\s=\\sevalenv->query_corpus->mother_size;", "\\1/* corpus_size = evalenv->query_corpus->mother_size; */", 1),
    replace = list("^(\\s*)assert\\(col->type\\s=\\stabular\\);", "\\1assert((col->type = tabular));", 1)
  ),
  
  "src/cwb/cqp/eval.h" = list(
    replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 3),
    replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 2),
    replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 1),
    extern = list(
      "int eep;",
      "EvalEnvironment Environment[MAXENVIRONMENT];",
      "EEP CurEnv, evalenv;"
    )
  ),
  
  "src/cwb/cqp/hash.h" = list(
    replace = list("(\\s*int\\sis_prime\\(int\\sn\\);)", "/* \\1 */", 1),
    replace = list("^(\\s*int\\sfind_prime\\(int\\sn\\);)", "/* \\1 */", 1),
    replace = list("^(\\s*unsigned\\sint\\shash_string\\(char\\s\\*s\\);)", "/* \\1 */", 1)
  ),
  
  "src/cwb/cqp/html-print.c" = list(
    replace = list("^(\\s*)char\\s\\*s;", "\\1/* char *s; */", 1),
    replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
    replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
    replace = list("^(\\s*)AttributeList\\s\\*strucs;", "\\1/* AttributeList *strucs; */", 1),
    replace = list("^(\\s*)strucs\\s=\\scd->printStructureTags;", "\\1/* strucs = cd->printStructureTags; */", 1)
  ),
  
  "src/cwb/cqp/latex-print.c" = list(
    replace = list("^(\\s*)char\\s\\*s;", "\\1/* char *s; */", 1),
    replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
    replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1)
  ),
  
  "src/cwb/cqp/options.h" = list(
    replace = list("^(\\s*)enum\\s_which_app\\s\\{\\sundef,\\scqp,\\scqpcl,\\scqpserver}\\swhich_app;", "\\1enum _which_app { undef, cqp, cqpcl, cqpserver} extern which_app;", 1),
    extern = list(
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
  ),
  
  "src/cwb/cqp/output.h" = list(
    replace = list("^(\\s*)FILE\\s\\*open_temporary_file\\(char\\s\\*tmp_name_buffer);", "\\1/* FILE *open_temporary_file(char *tmp_name_buffer); */", 1)
  ),
  
  "src/cwb/cqp/parse_actions.c" = list(
    replace = list("^(\\s*)c->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1c->idlist.del = left->pa_ref.del;", 1),
    replace = list("^(\\s*)left->pa_ref\\.delete\\s=\\s0;", "\\1left->pa_ref.del = 0;", 1),
    replace = list("^(\\s*)node->idlist\\.delete\\s=\\s0;", "\\1node->idlist.del = 0;", 1),
    replace = list("^(\\s*)res->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1res->idlist.del = left->pa_ref.del;", 1),
    replace = list("^(\\s*)res->pa_ref\\.delete\\s=\\sauto_delete;", "\\1res->pa_ref.del = auto_delete;", 1),
    replace = list("^(\\s*)res->sa_ref\\.delete\\s=\\sauto_delete;", "\\1res->sa_ref.del = auto_delete;", 1),
    replace = list("^(\\s*)res->pa_ref\\.delete\\s=\\s0;", "\\1res->pa_ref.del = 0;", 1),
    replace = list("^(\\s*)res->pa_ref\\.delete\\s=\\sauto_delete;", "\\1res->pa_ref.del = auto_delete;", 1),
    replace = list("^(\\s*)res->sa_ref\\.delete\\s=\\s0;", "\\1res->sa_ref.del = 0;", 1)
  ),
  
  "src/cwb/cqp/corpmanag.c" = list(
    extern = list("CorpusList *corpuslist;")
  ),

  "src/cwb/cqp/corpmanag.h" = list(
    extern = list("CorpusList *current_corpus;", "CorpusList *corpuslist;")
  ),

  "src/cwb/cqp/regex2dfa.c" = list(
    replace = list("^(\\s*)int\\signore_value;", "\\1int ignore_value __attribute__((unused));", 1)
  ),
  
  "src/cwb/cqp/sgml-print.c" = list(
    replace = list("^(\\s*)AttributeList\\s\\*strucs;", "\\1/* AttributeList *strucs; */", 1),
    replace = list("^(\\s*)strucs\\s=\\scd->printStructureTags;", "\\1/* strucs = cd->printStructureTags; */", 1)
  ),
  
  "src/cwb/cqp/parser.tab.c" = list(
    replace = list('^(\\s*)cqpmessage\\(Error,\\s"CQP\\sSyntax\\sError:.*?",\\ss,\\sQueryBuffer\\);', '\\1cqpmessage(Error, "CQP Syntax Error: %s", s);', 1L),
    replace = list("^(\\s+)int\\sok;", "\\1int ok __attribute__((unused));", 4L)
  ),
  
  "src/cwb/cqp/cqp.h" = list(
    extern = list(
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
    )
  ),
  
  
  "src/cwb/CQi/auth.c" = list(
    insert_before = list("/\\*\\sdata\\sstructures\\s\\(internal\\suse\\sonly\\)\\s\\*/", c("void Rprintf(const char *, ...);", ""), 1)
  ),
  
  "src/cwb/CQi/server.c" = list(
    insert_before = list("^\\/\\*", c("void Rprintf(const char *, ...);", ""), 3L),
    insert_after = list('#include "\\.\\./cqp/hash\\.h"', '#include "../cl/lexhash.h" /* inserted by AB */', 1)
  ),
  
  "src/cwb/CQi/cqpserver.c" = list(
    insert_after = list('#include "\\.\\./cqp/groups\\.h"', "void Rprintf(const char *, ...);", 1)
  ),
  
  "src/cwb/utils/cwb-makeall.c" = list(
    insert_before = list("/\\*\\*\\sThe\\scorpus\\swe\\sare\\sworking\\son\\s\\*/", c("#include <netinet/in.h>", ""), 1)
  ),
  
  "src/cwb/definitions.mk" = list(
    replace = list("(\\$\\(error\\sConfiguration\\svariable\\sRANLIB)", "# \\1", 1L)
  ),
  
  "src/cwb/config/platform/darwin-64" = list(
    replace = list("^(CFLAGS\\s*=.*?)\\s+-march=native\\s+(.*?)$", "\\1 \\2", 1L)
  ),
  
  "src/cwb/config/platform/darwin" = list(
    replace = list("^READLINE_DEFINES\\s:=.*?$", "READLINE_DEFINES =", 1L),
    replace = list("^READLINE_LIBS\\s:=.*?$", "READLINE_LIBS =", 1L)
  ),
  
  "src/cwb/config/platform/darwin-port-core2" = list(
    replace = list("^(CFLAGS\\s*=.*?)\\s+-march=core2\\s+(.*?)$", "\\1 \\2", 1L)
  ),
  
  "src/cwb/config/platform/darwin-universal" = list(
    replace = list("^(CFLAGS\\s*=.*?)\\s+-march=core2\\s+(.*?)$", "\\1 \\2", 1L)
  ),
  
  "src/cwb/config/platform/darwin-brew" = list(
    replace = list("^(CFLAGS\\s*=.*?)\\s+-march=native\\s+(.*?)$", "\\1 \\2", 1L)
  ),
  
  "src/cwb/config/platform/linux-opteron" = list(
    replace = list("^(CFLAGS\\s*=.*?)\\s+-march=opteron\\s+(.*?)$", "\\1 \\2", 1L)
  ),
  
  "src/cwb/config/platform/unix" = list(
    replace = list("^AR\\s+=\\s+ar\\scq\\s*$", "AR = ar", 1L)
  ),
  
  "src/cwb/config/platform/linux" = list(
    insert_before = list("##\\sCPU\\sarchitecture", c("## require position-independent code", "CFLAGS += -fPIC", ""), 1L)
  )
)


# x <- readLines("~/Lab/github/RcppCWB/prep/autopatch/data/patches.R")
# x <- x[10:497]
# x <- x[-grep("^\\s*\\),?\\s*$", x)]
# x <- x[-grep("^\\s*$", x)]
# file_lines <- grep('^\\s*".*?"\\s*=\\s*list', x)
# splitted <- split(
#   x,
#   f = cut(
#     x = 1:length(x),
#     breaks = c(file_lines, length(x)),
#     right = FALSE,
#     include.lowest = TRUE
#   )
# )
# unique_files <- unique(gsub('\\s*"(.*?)"\\s*=\\s*list.*?$', "\\1", x[grep('^\\s*".*?"\\s=\\s*list', x)]))
# 
# file_list <- setNames(lapply(unique_files, function(x) list()), unique_files)
# for (i in 1L:length(splitted)){
#   fname <- gsub('^\\s*"(.*?)"\\s*=\\s*list.*?$', "\\1", splitted[[i]][1])
#   print(fname)
#   if (length(splitted[[i]]) >= 2){
#     file_list[[fname]] <- c(file_list[[fname]], splitted[[i]][2:length(splitted[[i]])])
#   }
# }
# file_list2 <- lapply(file_list, unlist)
# file_list3 <- lapply(
#   file_list2,
#   function(li){
#     li[1:(length(li) - 1)] <- gsub(")$", "),", li[1:(length(li) - 1)])
#     li[length(li)] <- gsub("),$", ")", li[length(li)])
#     li
#   }
# )
# 
# for (n in names(file_list3)){
#   cat(sprintf('  "%s" = list(\n', n), file = "~/Lab/github/RcppCWB/prep/autopatch/data/patches2.R", append = TRUE)
#   cat(paste(file_list3[[n]], collapse = "\n"), file = "~/Lab/github/RcppCWB/prep/autopatch/data/patches2.R", append = TRUE)
#   cat("\n", file = "~/Lab/github/RcppCWB/prep/autopatch/data/patches2.R", append = TRUE)
#   cat("  ),\n", file = "~/Lab/github/RcppCWB/prep/autopatch/data/patches2.R", append = TRUE)
# }