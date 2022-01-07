#' Workflow to patch CWB
#'
#' @examples 
#' data_files <- list.files("~/Lab/github/RcppCWB/prep/autopatch/data", full.names = TRUE)
#' for (fname in data_files) source(fname)
#' 
#' P <- PatchEngine$new(
#'   cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
#'   repodir = "~/Lab/github/RcppCWB",
#'   revision = 1200
#' )
#' P$patch_all()
PatchEngine <- R6Class(
  
  classname = "PatchEngine",
  
  public = list(
    
    revision = NULL,
    cwb_dir_svn = NULL,
    repodir = NULL,
    repository = NULL,
    branch_of_departure = NULL,
    last_commit = NULL,
    patch_commit = NULL,
    patchbranch = NULL,
    global_replacements = list(
      c("(vf|f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd->stream|redir->stream),\\s*", "Rprintf("),
      c("YY(F|D)PRINTF\\s*(\\({1,2})\\s*(stderr|yyoutput),\\s*" , "YY\\1PRINTF \\2"),
      c("fprintf\\s*\\(", "Rprintf("),
      c("(\\s+)printf\\(", "\\1Rprintf("),
      c("#(\\s*)define\\sYYFPRINTF\\sfprintf", "#\\1define YYFPRINTF Rprintf"),
      c('^\\s*#include\\s+"endian\\.h"\\s*$', '#include "endian2.h"') # only files in cl, maybe limit this
    ),
    file_patches = NULL,
    verbose = TRUE,
    diff_global_replacements = NULL,
    diff_file_patches = NULL,
    
    initialize = function(cwb_dir_svn, revision, repodir, verbose = TRUE){
      self$verbose <- verbose
      
      self$repodir <- path.expand(repodir)
      if (self$verbose) message("RcppCWB repository path: ", self$repodir)
      
      self$cwb_dir_svn <- path.expand(cwb_dir_svn)
      if (self$verbose) message("SVN directory with CWB code: ", self$cwb_dir_svn)
      
      self$revision <- as.integer(revision)
      if (self$revision) message("SVN revision to use: ", self$revision)
      
      
      self$branch_of_departure <- self$get_branch_of_departure()
      if (self$verbose) message("The branch of departure is: ", self$branch_of_departure)
      
      self$repository <- repository(self$repodir)
      self$last_commit <- last_commit(repo = self$repodir)
      self$file_patches <- self$patches(revision)
      
      message("flex version: ", system("flex --version", intern = TRUE))
      message("bison version: ", system("bison --version", intern = TRUE)[1])
      
      invisible(self)
    },
    
    get_branch_of_departure = function(){
      branches(self$repodir)[sapply(branches(self$repodir), is_head)][[1]][["name"]]
    },
    
    svn_get_revision = function(){
      old_wd <- setwd(self$cwb_dir_svn)
      rev <- as.integer(system("svn info --show-item revision", intern = TRUE))
      setwd(old_wd)
      rev
    },
    
    #' @return An `integer` value with the active SVN revision number.
    svn_set_revision = function(){
      
      old_wd <- setwd(self$cwb_dir_svn)
      rev <- self$svn_get_revision()
      if (self$verbose) message("Active revision of CWB SVN repository: ", rev)
      if (rev != as.integer(self$revision)){
        system(
          sprintf("svn up -r %d", self$revision),
          intern = TRUE
        )
        rev <- self$svn_get_revision()
        if (self$verbose) message("Updated revision of CWB SVN repository: ", rev)
      }
      setwd(old_wd)
      rev
    },
    
    cwb_fresh_copy = function(){
      
      if (self$verbose) message("Copy unaltered CWB code into RcppCWB repository ...", appendLF = FALSE)
      
      self$patchbranch <- sprintf("r%d", self$svn_get_revision()) # name of new branch
      
      git2r::checkout(self$repodir, branch = self$branch_of_departure)
      if (self$patchbranch %in% names(branches(self$repodir))){
        git2r::branch_delete(branch = branches(self$repodir)[[self$patchbranch]])
      }
      git2r::branch_create(commit = last_commit(repo = self$repodir), name = self$patchbranch)
      git2r::checkout(self$repodir, branch = self$patchbranch) # switch to new branch
      
      # copy CWB files
      repo_cwb_dir <- path(self$repodir, "src", "cwb")
      cwb_files <- list.files(self$cwb_dir_svn, recursive = TRUE, full.names = TRUE)
      file.copy(
        from = cwb_files,
        to = gsub(paste("^", self$cwb_dir_svn, sep = ""), repo_cwb_dir, cwb_files),
        overwrite = TRUE
      )
      
      if (self$verbose) message(sprintf("%d files copied", length(cwb_files)))
      
      # Remove files that have been added (and that need to be added explicitly)
      # To restore the state of RcppCWB development, these files need to be re-added or generated later on
      
      cwb_files_svn_truncated <- gsub(self$cwb_dir_svn, "", cwb_files)
      cwb_files_repo <- list.files(repo_cwb_dir, recursive = TRUE, full.names = TRUE)
      cwb_files_repo_truncated <- gsub(repo_cwb_dir, "", cwb_files_repo)
      
      added_files <-  cwb_files_repo_truncated[!cwb_files_repo_truncated %in% cwb_files_svn_truncated]
      added_files_full_path <- fs::path(repo_cwb_dir, added_files)
      added_files_existing <- added_files_full_path[file.exists(added_files_full_path)]
      git2r::rm_file(repo = self$repodir, path = added_files_existing)
      
      # add & commit
      git2r::add(repo = self$repodir, path = unname(unlist(git2r::status(repo = self$repodir))))
      git2r::commit(self$repodir, message = "CWB restored")
    },
    
    run_bison_flex = function(){
      
      if (self$verbose) message("Run bison and flex parsers")
      
      cwb_pkg_dir <- file.path(self$repodir, "src", "cwb")
      old_wd <- setwd(path(cwb_pkg_dir, "cl"))
      system("bison -d -t -p creg registry.y")
      system("flex -8 -Pcreg registry.l")
      
      setwd(path(cwb_pkg_dir, "cqp"))
      system("bison -d -t parser.y")
      system("flex -8 parser.l")
      setwd(old_wd)
      invisible(self)
    },
    
    rename_files = function(){
      
      if (self$verbose) message("Rename files cl/endian.h (to endian2.h) and instutils/Makefile (to _Makefile)")
      
      cwb_pkg_dir <- file.path(self$repodir, "src", "cwb")
      file.rename(
        from = path(cwb_pkg_dir, "cl", "endian.h"),
        to = path(cwb_pkg_dir, "cl", "endian2.h")
      )
      
      file.rename(
        from = path(cwb_pkg_dir, "instutils", "Makefile"),
        to = path(cwb_pkg_dir, "instutils", "_Makefile")
      )
      invisible(self)
    },
    
    copy_files = function(){
      if (self$verbose) message("Add newly created files to CWB code (overwriting existing files)")
      new_files <- list.files(path = file.path(self$repodir, "prep", "cwb"), full.names = TRUE, recursive = TRUE)
      for (fname in new_files){
        if (self$verbose) message("... copy file: ", fname)
        file.copy(from = fname, to = gsub("/prep/", "/src/", fname), overwrite = TRUE)
      }
      invisible(self)
    },
    
    create_dummy_depend_files = function(){
      if (self$verbose) message("Create dummy depend.mk files ...")
      cat("\n", file = file.path(self$repodir, "src", "cwb", "cl", "depend.mk"))
      cat("\n", file = file.path(self$repodir, "src", "cwb", "cqp", "depend.mk"))
      invisible(self)
    },
    
    replace_globally = function(){
      if (self$verbose) message("Perform global replacements ...")
      cwb_pkg_dir <- file.path(self$repodir, "src", "cwb")
      for (subdir in c("cl", "cqp", "CQi")){
        files <- list.files(file.path(cwb_pkg_dir, subdir), full.names = TRUE)
        for (f in files){
          code <- readLines(f)
          for (i in 1L:length(self$global_replacements)){
            code <- gsub(self$global_replacements[[i]][1], self$global_replacements[[i]][2], code)
          }
          writeLines(text = code, con = f)
        }
      }
      
    },
    
    delete_line_before = function(code, action, file){
      which_position <- if (length(action) > 1L) action[[2]] else 1L
      times <- if (length(action) == 3L) action[[3]] else 1L

      position <- grep(pattern = action[[1]], code)[which_position]
      if (!is.na(position)){
        code <- code[-(position - 1L:times)]
      } else {
        message(
          sprintf("Trying to patch file '%s' - no match for action 'delete_line_before' (regex: %s | match: %d | lines: %d) ", file, action[[1]], which_position, times)
        )
      }
      code
    },
    
    insert_before = function(code, action, file){
      which_position <- if (length(action) > 2L) action[[3]] else 1
      
      position <- grep(pattern = action[[1]], code)[which_position]
      if (!is.na(position)){
        code <- c(
          code[1L:(position - 1L)],
          action[[2]],
          code[position:length(code)]
        )
      } else {
        message(
          sprintf(
            "Trying to patch file '%s' - no match for action 'insert_before' (regex: %s | match: %d | insertion: %s)",
            file, action[[1]], which_position, paste(action[[2]], collapse = "///")
          )
        )
      }
      code
    },
    
    insert_after = function(code, action, file){
      which_position <- if (length(action) > 2L) action[[3]] else 1L
      
      position <- grep(pattern = action[[1]], code)[which_position]
      if (!is.na(position)){
        code <- c(
          code[1L:position],
          action[[2]],
          code[(position + 1L):length(code)]
        )
      } else {
        message(
          sprintf(
            "Trying to patch file '%s' - no match for action 'insert_after' (regex: %s | match: %d | insertion: %s)",
            file, action[[1]], which_position, paste(action[[2]], collapse = "///")
          )
        )
      }
      code
    },
    
    replace = function(code, action, file){
      position <- grep(pattern = action[[1]], code)[ action[[3]] ]
      if (!is.na(position)){
        code[position] <- gsub(action[1], action[2], code[position])
      } else {
        message(
          sprintf("Trying to patch file '%s' - no match for action 'replace' (regex: %s | match: %d | replacement: %s)",
                  file, action[[1]], action[[3]], paste(action[[2]], collapse = "///")),
          appendLF = FALSE
        )
      }
      code
    },
    
    remove_lines = function(code, action, file){
      
      position <- grep(pattern = action[[1]], code)[ action[[2]] ]
      if (!is.na(position)){
        code <- code[-position]
      } else {
        message(
          sprintf("Trying to patch file '%s' - no match for action 'remove_lines' (regex: %s | match: %d)", file, action[[1]], action[[2]]),
          appendLF = FALSE
        )
      }
      code
    },
    
    extern = function(code, action, file){
      for (ext in action){
        matches <- which(startsWith(code, ext))
        if (length(matches) > 0L){
          for (position in matches){
            code[position] <- paste("extern", code[position], sep = " ")
          }
        } else {
          message(sprintf("Trying to patch file '%s' - no match for action 'extern' (var to extern: %s | n_matches: %d", file, ext, length(matches)))
        }
      }
      code
    },
    
    # Order to maintain:
    # - delete_line_before
    # - insert_before
    # - insert_after
    # - replace
    # - remove_lines
    patches = function(revision){
      list(
        
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
          extern = as.list(c(
            
            "char *def_unbr_attr;",
            "enum _matching_strategy { traditional, shortest_match, standard_match, longest_match } matching_strategy;",
            "char *matching_strategy_name;",
            "char *pager;",
            "char *tested_pager;",
            "char *less_charset_variable;",
            "char *printModeString;",
            "char *printModeOptions;",
            "char *printStructure;",
            "char *left_delimiter;",
            "char *right_delimiter;",
            "char *registry;",
            "char *LOCAL_CORP_PATH;",
            "char *cqp_init_file;",
            "char *macro_init_file;",
            "char *cqp_history_file;",
            "char *default_corpus;",
            "char *query_string;",
            "char *ExternalSortingCommand;",
            "char *progname;",
            "char *licensee;",
            "FILE *batchfd;",

            if (revision < 1500)
              c(
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
                if (revision <= 1137) "int subquery;" else "int auto_subquery;",
                if (revision >= 1165) "int anchor_number_target;",
                if (revision >= 1171) "int anchor_number_keyword;",
                "int query_optimize;",
                "int strict_regions;",
                "int use_readline;",
                "int highlighting;",
                "int paging;",
                "int use_colour;",
                "int progress_bar;",
                "int pretty_print;",
                "int autoshow;",
                "int timing;",
                "int show_tag_attributes;",
                "int show_targets;",
                "int printNrMatches;",
                "int auto_save;",
                "int save_on_exit;",
                "int write_history_file;",
                "int batchmode;",
                "int silent;",
                "int UseExternalSorting;",
                "int UseExternalGrouping;",
                "char *ExternalGroupingCommand;",
                "int user_level;",
                "int rangeoutput;",
                "int child_process;",
                "ContextDescriptor CD;",
                "int handle_sigpipe;",
                if (revision >= 1291) "char *attribute_separator;"
              )
            )
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
            if (revision < 1075) "char *cqp_input_string;", # starting from r1075 externed anyway
            if (revision < 1075) "int cqp_input_string_position;", # starting from r1075 externed anyway
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
        ),
        
        "src/cwb/config.mk" = list(
          replace = list("^PLATFORM=darwin-brew\\s*$", "PLATFORM=darwin-64", 1L)
        )
      )
    },
    
    patch_file = function(file){
      
      fname_full <- fs::path(self$repodir, file)
      if (!file.exists(fname_full)){
        return(FALSE)
      } else {
        code <- readLines(fname_full)
        for (i in 1L:length(self$file_patches[[file]])){
          new_code <- self[[ names(self$file_patches[[file]])[i] ]](code = code, action = self$file_patches[[file]][[i]], file = file)
          if (identical(code, new_code)){
            message(sprintf("Patch #%d for file '%s' does not change code", i, file))
          } else {
            code <- new_code
          }
        }
        writeLines(code, fname_full)
        return(TRUE)
      }
    },
    
    patch_files = function(){
      
      files_with_actions <- fs::path(path.expand(self$repodir), names(self$file_patches))
      cwb_files <- list.files(path = path(self$repodir, "src", "cwb"), full.names = TRUE, recursive = TRUE)
      missing_files_with_actions <- files_with_actions[!files_with_actions %in% cwb_files]
      
      if (length(missing_files_with_actions) > 0L){
        message("Actions defined for the following files, but file not present:")
        for (f in missing_files_with_actions) message("- ", f)
      }
      
      results <- sapply(names(self$file_patches), self$patch_file)
      if (self$verbose) message("Number of files successfully patched: ", table(results)[["TRUE"]])
    },
    
    get_difflist = function(){
      diff <- git2r::diff(self$repository, context_lines = 0)
      setNames(
        lapply(
          diff$files,
          function(f){
            lapply(
              f$hunks,
              function(h) setNames(
                sapply(h$lines, `[[`, "content"),
                ifelse(sapply(h$lines, `[[`, "old_lineno") == -1, "new", "old")
                )
            )
          }
        ),
        sapply(diff$files, `[[`, "old_file")
      )
    },
    
    get_diffs = function(dirs = c("cl", "cqp", "CQi", "utils")){
      
      lapply(
        dirs,
        function(dir){
          fs::path(self$repodir, "src", "cwb", dir)
          git_cmd <- sprintf(
            "git diff --ignore-space-at-eol %s..%s .",
            self$last_commit[["sha"]], self$patch_commit[["sha"]]
          )
          result <- system(git_cmd, intern = TRUE)
          cat(paste(result, collapse = "\n"))
        }
      )

    },
    
    patch_all = function(){
      
      setwd(self$repodir)
      
      self$svn_set_revision()
      self$cwb_fresh_copy()
      
      if (!is.null(unlist(status(self$repository)))){
        stop("cannot proceed - current branch contains work that would be lost")
      }
      
      checkout(self$repodir, branch = self$patchbranch)
      self$copy_files()
      self$run_bison_flex()
      self$rename_files()
      self$create_dummy_depend_files()
      
      git2r::add(repo = self$repodir, path = "src/cwb/*")
      commit(self$repository, message = "before global replacements")
      
      self$replace_globally()
      self$diff_global_replacements <- self$get_difflist()
      git2r::add(repo = self$repodir, path = "src/cwb/*")
      commit(self$repository, message = "global replacements applied")
      
      self$patch_files()
      self$diff_file_patches <- self$get_difflist()

      if (self$verbose) message("Add new and altered files to HEAD in repo: ", self$repodir)
      git2r::add(repo = self$repodir, path = "src/cwb/*")
      if (self$verbose) message("Commit: ", self$repodir)
      commit(self$repository, message = "CWB patched")
      self$patch_commit <- last_commit(self$repository)
      if (self$verbose) message("Story to be told")
      
      if (self$verbose) message("Return to branch of departure: ", self$branch_of_departure)
      checkout(repo = self$repodir, branch = self$branch_of_departure)
      invisible(self)
    }
    
  )
)


