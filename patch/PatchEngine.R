#' Workflow to patch CWB
#'
#' Flag -fcommon removed from config and config/platform/linux again
#' @examples 
#' source("~/Lab/github/RcppCWB/patch/PatchEngine.R")
#' 
#' P <- PatchEngine$new(
#'   cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
#'   repodir = "~/Lab/github/RcppCWB",
#'   makeheaders = "~/Lab/github_foreign/makeheaders/src/makeheaders",
#'   revision = 1069
#' )
#' P$patch_all()
PatchEngine <- R6Class(
  
  classname = "PatchEngine",
  
  public = list(
    
    #' @field global_replacements Generated `$initialize()` by calling
    #'   `$configure_global_replacements`
    revision = NULL,
    cwb_dir_svn = NULL,
    repodir = NULL,
    repository = NULL,
    branch_of_departure = NULL,
    last_commit = NULL,
    patch_commit = NULL,
    patchbranch = NULL,
    global_replacements = NULL,
    file_patches = NULL,
    verbose = TRUE,
    diff_global_replacements = NULL,
    diff_file_patches = NULL,
    makeheaders = NULL,
    
    initialize = function(cwb_dir_svn, revision, repodir, makeheaders, verbose = TRUE){
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
      
      self$global_replacements <- self$configure_global_replacements(revision)
      self$file_patches <- self$patches(revision)
      
      message("flex version: ", system("flex --version", intern = TRUE))
      message("bison version: ", system("bison --version", intern = TRUE)[1])
      message("makeheaders utility is available: ", identical(system(makeheaders, intern = TRUE), character()))
      self$makeheaders <- makeheaders

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
      
      if (self$verbose) message("* copy unaltered CWB code into RcppCWB repository ... ", appendLF = FALSE)
      
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
      
      # config dir copy of platform-file 'solaris': 'solaris_x86', with modification 'CFLAGS = -Wall -O3 -fPIC'
      added_files <-  cwb_files_repo_truncated[!cwb_files_repo_truncated %in% cwb_files_svn_truncated]
      added_files_full_path <- fs::path(repo_cwb_dir, added_files)
      added_files_existing <- added_files_full_path[file.exists(added_files_full_path)]
      git2r::rm_file(repo = self$repodir, path = added_files_existing)
      
      # add & commit
      git2r::add(repo = self$repodir, path = unname(unlist(git2r::status(repo = self$repodir))))
      git2r::commit(self$repodir, message = "CWB restored")
    },
    
    #' @description 
    #' The flex and bison parsers may be missing on CRAN build machines. To
    #' avoid errors, parsed files files are generated (registry.tab.c,
    #' registry.tab.h, registry.y)
    run_bison_flex = function(){
      
      if (self$verbose) message("* run bison and flex parsers")
      
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
      
      if (self$verbose) message("* rename files cl/endian.h (to endian2.h) and instutils/Makefile (to _Makefile)")
      
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
      if (self$verbose) message("* add newly created files to CWB code (overwriting existing files)")
      new_files <- list.files(path = file.path(self$repodir, "patch", "cwb"), full.names = TRUE, recursive = TRUE)
      for (fname in new_files){
        if (self$verbose) message("... copy file: ", fname)
        file.copy(from = fname, to = gsub("/patch/", "/src/", fname), overwrite = TRUE)
      }
      invisible(self)
    },
    
    create_copy = function(){
      if (self$verbose) message ("* create utils/globals.h as a copy of cwb-encode.c")
      
      file.copy(
        from = file.path(self$repodir, "src", "cwb", "utils", "cwb-encode.c"),
        to = file.path(self$repodir, "src", "cwb", "utils", "globals.h"),
        overwrite = TRUE
      )
      
      file.copy(
        from = file.path(self$repodir, "src", "cwb", "cl", "cl.h"),
        to = file.path(self$repodir, "src", "_cl.h"),
        overwrite = TRUE
      )
      
      file.copy(
        from = file.path(self$repodir, "src", "cwb", "cqp", "eval.h"),
        to = file.path(self$repodir, "src", "_eval.h"),
        overwrite = TRUE
      )
    },
    
    create_globalvars_file = function(){
      if (self$verbose) message("* create _globalvars.h ... ", appendLF = FALSE)
      
      # Strategy 1: Variables that are declared as 'extern' by patch are global vars
      extern_by_patch <- unique(unname(unlist(lapply(self$file_patches, `[[`, "extern"))))

      # Strategy 2: Later version of CWB prepends extern by default. Use makeheaders 
      # utility to autogenerate a header and grep externed vars 
      extern_by_default <- gsub("^\\s*extern\\s+", "", grep("^\\s*extern\\s", system(
        sprintf("%s -h %s", self$makeheaders, file.path(self$repodir, "src", "cwb", "cqp", "options.c")),
        intern = TRUE
      ), value = TRUE))

      # combine results
      extern <- unique(c(extern_by_patch, extern_by_default))
      
      # manual additions
      typedef_CQPOption <- self$get_snippet(
        file = file.path(self$repodir, "src", "cwb", "cqp", "options.h"),
        from = "^\\s*typedef\\senum\\s_opttype\\s\\{\\s*$",
        to = "^\\s*}\\s+CQPOption;\\s*$"
      )
      defs <- self$get_snippet(
        file = file.path(self$repodir, "src", "cwb", "cqp", "options.h"),
        from = "^#define\\sOPTION_VISIBLE_IN_CQP\\s+\\d+\\s*$",
        to = '#define\\sCQP_FALLBACK_PAGER\\s"more"'
      )
      
      # options <- self$get_snippet(
      #     file = file.path(self$repodir, "src", "cwb", "cqp", "options.c"),
      #     from = "^CQPOption\\scqpoptions\\[\\]\\s=\\s\\{\\s*$",
      #     to = '\\s*\\*\\sNON-OPTION\\sGLOBAL\\sVARIABLES\\s*$'
      #   )
      # options <- options[1:(length(options) - 3)]

      extern <- c(
        if (self$revision < 1690) "enum _which_app { undef, cqp, cqpcl, cqpserver} which_app;",
        if (self$revision >= 1690) "typedef enum which_app { undef, cqp, cqpcl, cqpserver} which_app_t;",
        typedef_CQPOption,
        extern,
        defs,
        "extern CQPOption cqpoptions;"
      )

      extern <- extern[!extern %in% c("EvalEnvironment Environment[MAXENVIRONMENT];", "EEP CurEnv, evalenv;", "int eep;", "CQPOption cqpoptions[];")]
      
      writeLines(text = extern, con = file.path(self$repodir, "src", "_globalvars.h"))
      message("OK")
    },
    

    create_dummy_depend_files = function(){
      if (self$verbose) message("* create dummy depend.mk files ...")
      cat("\n", file = file.path(self$repodir, "src", "cwb", "cl", "depend.mk"))
      cat("\n", file = file.path(self$repodir, "src", "cwb", "cqp", "depend.mk"))
      invisible(self)
    },
    
    replace_globally = function(){
      if (self$verbose) message("* perform global replacements ...")
      cwb_pkg_dir <- file.path(self$repodir, "src", "cwb")
      
      files <- c(
        unlist(lapply(
          c("cl", "cqp", "CQi"),
          function(subdir) list.files(file.path(cwb_pkg_dir, subdir), full.names = TRUE)
        )),
        path.expand(
          file.path(cwb_pkg_dir, "utils", c("cwb-encode.c", "cwb-makeall.c", "cwb-huffcode.c", "cwb-compress-rdx.c"))
        )
      )

      for (f in files){
        code <- readLines(f)
        for (i in 1L:length(self$global_replacements)){
          if (length(self$global_replacements[[i]]) > 2L){
            if (endsWith(f, self$global_replacements[[i]][[3]])) next
          }
          code <- gsub(self$global_replacements[[i]][1], self$global_replacements[[i]][2], code)
        }
        writeLines(text = code, con = f)
      }
      
    },
    
    #' @param file A file
    get_snippet = function(file, from, to){
      code <- readLines(file)
      start <- grep(from, code)
      if (!length(start)) warning("Fn get_snippet - no match for query to get start: ", from)
      end <- grep(to, code)
      if (!length(end)) warning("Fn get_snippet - no match for query to get end: ", end)
      code[start:end]
    },
    
    delete_line_before = function(code, action, file){
      which_position <- if (length(action) > 1L) action[[2]] else 1L
      times <- if (length(action) == 3L) action[[3]] else 1L

      position <- grep(pattern = action[[1]], code)[which_position]
      if (is.na(times)) times <- position - 1L
      
      if (!is.na(position)){
        code <- code[-(position - 1L:times)]
      } else {
        message(
          sprintf("Trying to patch file '%s' - no match for action 'delete_line_before' (regex: %s | match: %d | lines: %d) ", file, action[[1]], which_position, times)
        )
      }
      code
    },
    
    
    delete_line_beginning_with = function(code, action, file){
      which_position <- if (length(action) > 1L) action[[2]] else 1L
      times <- if (length(action) == 3L) action[[3]] else 1L
      
      position <- grep(pattern = action[[1]], code)[which_position]
      if (is.na(times)) times <- length(code) - position
      
      if (!is.na(position)){
        code <- code[-(position + 0L:times)]
      } else {
        message(
          sprintf("Trying to patch file '%s' - no match for action 'delete_line_after' (regex: %s | match: %d | lines: %d) ", file, action[[1]], which_position, times)
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
      position <- grep(pattern = action[[1]], code)
      
      if (length(position) == 0L){
        message(
          sprintf("Trying to patch file '%s' - no match for action 'replace' (regex: %s | match: %d | replacement: %s)",
                  file, action[[1]], action[[3]], paste(action[[2]], collapse = "///")),
          appendLF = FALSE
        )
        return(code)
      }
      
      if (!is.na(action[[3]])) position <- position[ action[[3]] ]
      for (p in position){
        code[position] <- gsub(action[1], action[2], code[position])
      }
      
      code
    },
    
    remove_lines = function(code, action, file){
      
      position <- grep(pattern = action[[1]], code)
      position <- if (!is.na(action[[2]])) position[ action[[2]] ] else position
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
      if (length(action) == 0L) return(code)
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
    
    configure_global_replacements = function(revision){
      replacements <- list(
        
        # In revision 1690, there are further targets dst->stream, outfh, tmp, fh
        if (revision == 1069){
          c("(vf|f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|File|rd->stream|redir->stream|debug_output|protocol),\\s*", "Rprintf(")
        } else if (revision >= 1690){
          c("(vf|f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd->stream|redir->stream|debug_output|dst->stream|outfh|tmp|fh|dest|dst|protocol|tmp_dst),\\s*", "Rprintf(")
        },
        if (revision == 1069) c("(vf|f|v)printf\\s*\\(\\s*fd,\\s*", "Rprintf(", "cwb-encode.c"),
        c("YY(F|D)PRINTF\\s*(\\({1,2})\\s*(stderr|yyoutput),\\s*" , "YY\\1PRINTF \\2"),
        c("fprintf\\s*\\(", "Rprintf("),
        c("(\\s+)printf\\(", "\\1Rprintf("),
        c("#(\\s*)define\\sYYFPRINTF\\sfprintf", "#\\1define YYFPRINTF Rprintf"),
        
        # The CWB file 'endian.h' causes issues with the endian.h system file. The
        # best solution I could come up with is to rename endian.h into endian2.h.
        # In addition - turn 'endian.h' into 'endian2.h' in the Makefile - change
        # include statements to 'include "endian2.h"'
        c('^\\s*#include\\s+"endian\\.h"\\s*$', '#include "endian2.h"') # only files in cl, maybe limit this,
#        c("^(\\s*)exit\\(1\\);", "\\1return 1;")
        
      )
      for (i in which(sapply(replacements, is.null) == TRUE)) replacements[[i]] <- NULL
      replacements
    },
    
    # Order to maintain:
    # - delete_line_before
    # - insert_before
    # - insert_after
    # - replace
    # - remove_lines
    patches = function(revision){
      list(
        
        "src/cwb/cl/lex.creg.c" = c(
          list(),
          # These changes are highly specific and are unlikely to work out of the box for r1690
          
          if (revision == 1069) list(
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
            
            # Funktion 'static void yyunput (int c, register char * yy_bp )'
            # auskommentiert, zur Vermeidung Nachricht: lex.creg.c:1410:17:
            # warning: unused function 'yyunput' [-Wunused-function]
            
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
          )
        ),
        
        "src/cwb/cl/endian2.h" = c(
          list(
            insert_before = list("#include\\s<windows\\.h>" , "#include <winsock2.h> /* AB reversed order, in original CWB code windows.h is included first */", 1L)
          ),
          
          # Ensure that order of inclusion is 'winsock2.h' and then 'windows.h'.
          # The windows.h/winsock2.h order has persisted, but the comment after
          # has changed.
          if (revision == 1069) list(
            delete_line_before = list("/* for consistency:", 1L, 1L)
          ),
          if (revision >= 1690) list(
            delete_line_before = list("/\\*\\snote,\\sin\\sorder\\sfor\\sall\\sthis\\sto\\swork,", 1L, 1L)
          )
        ),
        
        "src/cwb/cl/Makefile" = list(
          delete_line_before = list("^libcl.a: \\$\\(OBJS\\)", 1L, if (revision >= 1400) 9L else 6L),
          delete_line_before = list("##\\scl\\.h\\sheader\\s", 1L, 11L),
          delete_line_before = list("^depend:$", 1L, if (revision >= 1400) 14L else 22L),
          replace = list("^TOP\\s=\\s\\$\\(shell\\spwd\\)/\\.\\.", "TOP = $(R_PACKAGE_SOURCE)", 1L),
          replace = list("^(\\s+)endian.h", "\\1endian2.h", 1L),
          replace = list("^(\\s+)\\$\\(AR\\)\\s", "\\1$(AR) cq ", 1L),
          remove_lines = list("^\\s+\\$\\(RANLIB\\)", 1L),
          remove_lines = list('^\\s*@\\$\\(ECHO\\)\\s".*?"\\s*$', NA) # r1690 is beautiful - but we nee verbosity
        ),
        
        "src/cwb/cl/attributes.c" = c(

          list(
            # This will work for r1690 too
            insert_before = list("^#include\\s<ctype\\.h>", c("void Rprintf(const char *, ...);", ""))
          ),
          
          # Cannot find the dollar variabl in r1690 - seems to be gone
          if (revision == 1069) list(
            # attributes.c:755:19: warning: variable ???dollar??? set but not used [-Wunused-but-set-variable]
            # int ppos, bpos, dollar, rpos;
            replace = list("(\\s+)int\\sppos,\\sbpos,\\sdollar,\\srpos;", "\\1int ppos, bpos, rpos;", 1),
            replace = list("^(\\s+)dollar = 0;", "\\1/* dollar = 0; */", 1),
            replace = list("^(\\s+)dollar = ppos;\\s", "\\1/* dollar = ppos; */", 1),
            
            # The STREQ macro is replaced by a cl_str_is() function in r1690

            # attributes.c: In function ???component_full_name???:
            #   macros.h:59:22: warning: the address of ???rname??? will always evaluate as ???true??? [-Waddress]
            # ((a) && (b) && (strcmp((a), (b)) == 0)))
            # attributes.c:807:11: note: in expansion of macro ???STREQ???
            # if (STREQ(rname, "HOME"))
            replace = list('^(\\s+)if\\s\\(STREQ\\(rname,\\s"HOME"\\)\\)', '\\1if (strcmp(rname, "HOME") == 0)', 1),
            
            # attributes.c:809:16: note: in expansion of macro ???STREQ???
            # else if (STREQ(rname, "APATH"))
            replace = list('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"APATH"\\)\\)', '\\1else if (strcmp(rname, "APATH") == 0)', 1),
            
            # attributes.c:812:16: note: in expansion of macro ???STREQ???
            # STREQ macro dissolved to avoid warnings in attributes.c
            # else if (STREQ(rname, "ANAME"))
            replace = list('^(\\s+)else\\sif\\s\\(STREQ\\(rname,\\s"ANAME"\\)\\)', '\\1else if (strcmp(rname, "ANAME") == 0)', 1)
          )
        ),
        
        "src/cwb/cl/bitio.c" = list(
          # Unchanged, will work as in 1069
          insert_before = list("^#include\\s<sys/types\\.h>", "void Rprintf(const char *, ...);")
        ),
        
        "src/cwb/cl/class-mapping.c" = c(
          list(),
          # This file does not exist in r1960
          if (revision == 1069) list(
            insert_before = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
            
            # argumet names class renamed to obj
            replace = list("(\\s*)\\*\\s@param\\sclass\\s+The\\sclass\\s+to\\scheck\\.", "\\1* @param obj  The class to check.", 2),
            replace = list("(\\s*)\\*\\s@param\\sclass\\s+The\\sclass\\s+to\\scheck\\.", "\\1* @param obj  The class to check.", 1),
            replace = list("^(\\s+)return\\smember_of_class_i\\(map,\\sclass,\\sid\\);", "\\1return member_of_class_i(map, obj, id);", 1),
            replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 2),
            replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 1),
            replace = list("^(\\s+)class->tokens,", "\\1obj->tokens,", 1),
            replace = list("^(\\s+)class->nr_tokens,", "\\1obj->nr_tokens,", 1)
          )
        ),
        
        "src/cwb/cl/corpus.c" = c(
          list(
            # Should work in r1690 too
            insert_before = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);")
          ),
          
          # Usage of stderr on a separate line - 3 times in r1069, but only once in r1690
          if (revision == 1069) list(
            remove_lines = list("(\\s+)stderr,", 3),
            remove_lines = list("(\\s+)stderr,", 2)
          ),
          list(
            remove_lines = list("(\\s+)stderr,", 1)
          )
        ),
        
        "src/cwb/cl/fileutils.c" = list(
          
          # All if this should work in r1690 too (though patches for Solares may be obsolete)
          
          insert_before = list('^#include\\s<sys/stat\\.h>', "void Rprintf(const char *, ...);"),
          
          # to satisfy solaris
          # #include <signal.h> /* added by Andreas Blaette  */
          # #include <sys/socket.h> /* added by Andreas Blaette */
          insert_before = list('^#include\\s"globals\\.h"', c("#ifndef __MINGW__", "#include <signal.h> /* added by Andreas Blaette  */", "#include <sys/socket.h> /* added by Andreas Blaette */", "#endif", ""))
        ),
        
        "src/cwb/cl/globals.h" = list(
          # Note the change from _globals_h_ to _cl_globals_h_
          insert_before = list(
            if (revision == 1069) '^#ifndef\\s_globals_h_' else '^#ifndef\\s_cl_globals_h_',
            "void Rprintf(const char *, ...);")
        ),
        
        "src/cwb/cl/lexhash.c" = list(
          insert_before = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);"),
          
          # Do not recall why this include is necessary, but the lexhash.h include (and file)
          # are absent in r1690 - so use another anchor.
          insert_after = list(
            if (revision == 1069) '^#include\\s"lexhash\\.h"' else '#include\\s"globals\\.h"',
            "#include <unistd.h>"
            )
        ),
        
        "src/cwb/cl/macros.c" = list(
          # Unchanged in r1690
          insert_before = list('#include\\s"globals\\.h"', "void Rprintf(const char *, ...);")
        ),
        
        "src/cwb/cl/makecomps.c" = c(
          list(
            # Unchanged in r1690
            insert_before = list('#include\\s<ctype\\.h>', c("void Rprintf(const char *, ...);", ""))
          ),
          
          if (revision == 1069) list(
            # commented out: char errmsg[CL_MAX_LINE_LENGTH] because also defined elsewhere
            replace = list("^(\\s*)char\\serrmsg\\[CL_MAX_LINE_LENGTH\\];", "/* char errmsg[CL_MAX_LINE_LENGTH]; */", 1)
          )
        ),
        
        "src/cwb/cl/registry.y" = list(
          # Stable r1069-1690
          insert_before = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);")
        ),
        
        "src/cwb/cl/special-chars.c" = list(
          # Stable r1069-1690
          insert_before = list('#include\\s<ctype\\.h>', "void Rprintf(const char *, ...);")
        ),
        
        "src/cwb/cl/storage.c" = list(
          # All of this is stable r1069-1690
          insert_before = list('^#include\\s<sys/types\\.h>', "void Rprintf(const char *, ...);"),
          
          # storage.c: In function ???mmapfile???:
          #   storage.c:335:12: warning: ignoring return value of ???write???, declared with attribute warn_unused_result [-Wunused-result]
          # write(fd, &fd, sizeof(int));
          insert_before = list("^(\\s*)lseek\\(fd,\\s0,\\sSEEK_SET\\);", '      if (success < 0) Rprintf("Operation not successful");'),
          replace = list("^(\\s+)write\\(fd,\\s&fd,\\ssizeof\\(int\\)\\);", "\\1ssize_t success = write(fd, &fd, sizeof(int));", 1)
        ),
        
        "src/cwb/cl/windows-mmap.c" = list(
          # stable r1069 and r1690
          insert_before = list('^#include\\s"windows-mmap\\.h"', "void Rprintf(const char *, ...);")
        ),
        
        # This file is not present in r1690
        "src/cwb/cl/registry.tab.c" = list(
          insert_before = list("#define\\sYYBISON\\s1", "void Rprintf(const char *, ...);", 1L)
        ),
        
        
        "src/cwb/cl/bitfields.c" = list(
          # stable r1069-r1690
          insert_after = list("^static\\sint\\sBaseTypeBits", "void Rprintf(const char *, ...);")
        ),
        
        "src/cwb/cl/cdaccess.c" = c(
          
          list(
            # The include of 'cdaccess.h' is gone with r1690 so we use another anchor
            insert_after = list(
              if (revision == 1069) '^#include\\s"cdaccess\\.h"' else '^#include\\s+"compression\\.h"',
              "void Rprintf(const char *, ...);"
            ),
            
            # This code is unchanged with r1690
            
            # cdaccess.c:2697:12: warning: ignoring return value of ???fgets???, declared with attribute warn_unused_result [-Wunused-result]
            # fgets(call, CL_MAX_LINE_LENGTH, pipe);
            replace = list("^(\\s*)fgets\\(call,\\sCL_MAX_LINE_LENGTH,\\spipe\\);", '\\1if (fgets(call, CL_MAX_LINE_LENGTH, pipe) == NULL) Rprintf("fgets failure");', 1)
          ),
          
          # These are patches to omit 'unused variable' warnings gone with r1690 (vars commented out, for instance)
          if (revision == 1069) list(
            # unchanged in r1690? But does not match
            replace = list("^(\\s*)off_end\\s=\\sntohl\\(lexidx_data\\[idx\\s\\+\\s1\\]\\)\\s-\\s1;", "\\1/* off_end = ntohl(lexidx_data[idx + 1]) - 1; */", 1),
            
            replace = list("^(\\s*)int\\sregex_result,\\sidx,\\si,\\slen,\\slexsize;", "\\1int idx, i, lexsize;", 1),
            replace = list("^(\\s*)int\\soptimised,\\sgrain_match;", "\\1int optimised;", 1),
            replace = list("^(\\s*)char\\s\\*word,\\s\\*preprocessed_string;", "\\1char *word;", 1),
            
            # cdaccess.c: In function ???cl_regex2id???:
            #   cdaccess.c:1392:20: warning: variable ???off_end??? set but not used [-Wunused-but-set-variable]
            # int off_start, off_end;     /* start and end offset of current lexicon entry */
            replace = list("^(\\s*)int\\soff_start,\\soff_end;", "\\1int off_start;", 1),
            replace = list("^(\\s*)char\\s\\*p;", "\\1/* char *p; */", 1),
            replace = list("^(\\s*)int\\si;", "\\1/* int i; */", 3),
            
            # cdaccess.c: In function ???cl_dynamic_call???:
            #   cdaccess.c:2533:17: warning: variable ???arg??? set but not used [-Wunused-but-set-variable]
            # DynCallResult arg;
            replace = list("^(\\s*)DynCallResult\\sarg;", "\\1/* DynCallResult arg; */", 1),
            replace = list("^(\\s*)arg\\s=\\sargs\\[argnum\\];", "\\1/* arg = args[argnum]; */", 1)
          )
        ),
        
        "src/cwb/cl/globals.c" = list(
          # stable r10690 - r1690
          insert_after = list(
            '^#include\\s"globals\\.h"',
            c(
              "void Rprintf(const char *, ...);",
              "",
              "char* cl_get_version(){",
              "  #ifdef CWB_VERSION",
              "  char* version = CWB_VERSION;",
              "  #else",
              '  char* version = "";',
              "  #endif",
              "  return version;",
              "}"
            )
          )
        ),
        
        "src/cwb/cl/ngram-hash.c" = c(
          list(
            insert_after = list("^#include\\s<math\\.h>", "void Rprintf(const char *, ...);")            
          ),

          # The variable 'temp' is unused only in r1069?!
          if (revision == 1069) list(
            # ngram-hash.c: In function ???cl_delete_ngram_hash???:
            #   ngram-hash.c:146:30: warning: variable ???temp??? set but not used [-Wunused-but-set-variable]
            # cl_ngram_hash_entry entry, temp;
            replace = list("^(\\s*)cl_ngram_hash_entry\\sentry,\\stemp;", "\\1cl_ngram_hash_entry entry;", 1),
            replace = list("^(\\s*)temp\\s=\\sentry;", "\\1/* temp = entry; */", 1)
          )
        ),
        
        "src/cwb/cl/regopt.c" = c(
          list(
            insert_after = list(
              if (revision == 1069) '^#include\\s"regopt\\.h"' else '^#include\\s"globals\\.h"',
              "void Rprintf(const char *, ...);"
            ) 
          ),
          
          # The issue may still be present but the syntax has changed - so we do not apply this for now
          if (revision == 1069) list(
            # regopt.c: In function ???make_jump_table???:
            #   regopt.c:1148:18: warning: suggest parentheses around comparison in operand of ???&??? [-Wparentheses]
            # if (ch >= 32 & ch < 127)
            replace = list("^(\\s*)if\\s\\(ch\\s>=\\s32\\s&\\sch\\s<\\s127\\)", "\\1if ((ch >= 32) & (ch < 127))", 1)
          )
        ),
        
        "src/cwb/cl/class-mapping.h" = c(
          list(),
          # The file is not there in r1690
          if (revision == 1069) list(
            replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 2),
            replace = list("^(\\s+)SingleMapping\\sclass,", "\\1SingleMapping obj,", 1)
          )
        ),
        
        # This file has appeared along the way, it just needs the minimal header
        "src/cwb/cl/ui-helpers.c" = c(
          list(),
          if (revision >= 1690) list(
            insert_before = list('^#include\\s"cl\\.h"', c("void Rprintf(const char *, ...);", ""), 1L)
          )
        ),
        
        "src/cwb/cl/cwb-globals.h" = c(
          list(),
          if (revision >= 1690) list(
            delete_line_beginning_with = list("#if\\s__STDC_VERSION__\\s>=\\s199901L", 1L, 22L)
          )
        ),
        
        
        "src/cwb/cqp/groups.c" = c(
          list(),
          if (revision == 1069) list(
            # Comment out open_temporary_file(char *tmp_name_buffer) in
            # cqp/output.c, in cqp/output.h and in usage in functions calling
            # this function: ComputeGroupExternally (cqp/groups.c) and
            # SortExternally (cqp/ranges.c)
            
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
          if (revision == 1690) list(
            delete_line_beginning_with = list("^\\s*if\\s\\(\\!\\(tmp_dst\\s=\\sopen_temporary_file\\(temporary_name\\)\\)\\)\\s\\{", 1L, 5L),
            delete_line_beginning_with = list("^\\s*fclose\\(tmp_dst\\);\\s*$", 1L, 0L),
            delete_line_beginning_with = list("^\\s*FILE\\s\\*tmp_dst;\\s*$", 1L, 0L)
          )
        ),
        
        "src/cwb/cqp/output.c" = c(
          list(),
          if (revision < 1400) list(
            
            # Comment out open_temporary_file(char *tmp_name_buffer) in
            # cqp/output.c, in cqp/output.h and in usage in functions calling
            # this function: ComputeGroupExternally (cqp/groups.c) and
            # SortExternally (cqp/ranges.c)
            delete_line_before = list('^\\s*sprintf\\(prefix,\\s"cqpt\\.%d",\\s\\(unsigned\\sint\\)getpid\\(\\)\\);', 1L, 8L),
            insert_before = list("^FILE\\s\\*\\s*", "/*", 1),
            insert_after = list("^\\}\\s*$", "*/", 3),
            replace = list('^(.*?)\\s*/\\*\\sholds\\s"cqpt\\.\\$\\$",\\sso\\s64\\schars\\sis\\splenty\\sof\\sheadroom\\s\\*/', "\\1", 1),
            replace = list('^(.*?)(\\s*)/\\*\\s"cqpt\\.\\$\\$"\\s\\*/', "\\1", 1),
            replace = list("^(.*?)\\s/\\*\\sstring\\sis\\sallocated\\sby\\stempnam\\(\\),\\sneeds\\sto\\sbe\\sfree'd\\sbelow\\s\\*/", "\\1", 1)
          )
        ),
        
        "src/cwb/cqp/parser.y" = c(
          list(),
          
          # Want to see issues with r1690 before adapting / checking these patches
          if (revision == 1069) list(
            delete_line_before = list("^\\s*if\\s\\(\\$2\\s&&\\sgenerate_code\\)\\s\\{", 3L),
            replace = list("^(\\s*)int\\sok;", "\\1int ok __attribute__((unused));", 3),
            replace = list("^(\\s*)int\\sok;", "\\1int ok __attribute__((unused));", 2),
            
            replace = list("^(\\s*)ok\\s=\\sSortSubcorpus\\(\\$2,\\s\\$3,\\s\\(\\$4\\s>=\\s1\\)\\s\\?\\s\\$4\\s:\\s1,\\s&\\(\\$5\\)\\);", "\\1SortSubcorpus($2, $3, ($4 >= 1) ? $4 : 1, &($5));", 1)
          )
        ),
        
        "src/cwb/cqp/ranges.c" = c(
          list(),
          
          # This is highly specific - want to see what happens before adapting things to r1690
          if (revision == 1069) list(
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
          )
        ),
        
        "src/cwb/cqp/lex.yy.c" = c(
          list(),
          
          # See what happens before taking pains to fit this on r1690
          if (revision == 1069) list(
            # Funktion yyunput auskommentiert, zur Vermeidung von FEhler:
            #   lex.yy.c:2459:17: warning: unused function 'yyunput' [-Wunused-function]
            # auskommentiert: static int input  (void)
            # um zu vermeiden:
            #   lex.yy.c:2500:16: warning: function 'input' is not needed and will not be emitted [-Wunneeded-internal-declaration]
            delete_line_before = list("/\\*\\*\\sImmediately\\sswitch\\sto\\sa\\sdifferent\\sinput\\sstream\\.", 1L, 111L),
            insert_before = list("#ifdef __cplusplus", "/*", 3L),
            insert_before = list("/\\*\\*\\sImmediately\\sswitch\\sto\\sa\\sdifferent\\sinput\\sstream\\.", "", 1L),
            insert_after = list("#endif", "*/", 27L),
            replace = list("^\\s+static\\svoid\\syyunput\\s\\(int\\sc,char\\s\\*buf_ptr\\s+\\);", "  /*  static void yyunput (int c,char *buf_ptr  ); */", 1L)
          )
        ),
        
        "src/cwb/cqp/Makefile" = list(

          delete_line_before = list("^clean:$", 1L, 7L),
          
          # remove targets parser.tab.c and lex.yy.c
          delete_line_before = list("^cqp\\$\\(EXEC_SUFFIX\\):", 1L, 8L),
          
          # insert new target to generate libcqp.a
          # libcqp.a: $(OBJS) $(CQI_OBJS)
          #     $(RM) $@
          #     $(AR) $@ $^
          #     $(RANLIB) $@
          insert_before = list("^cqp\\$\\(EXEC_SUFFIX\\):.*", "libcqp.a: $(OBJS) $(CQI_OBJS)\n\t$(RM) $@\n\t$(AR) cq $@ $^\n", 1L),
          
          # Define target 'all' as follows:
          # all: libcqp.a
          replace = list("all:\\s\\$\\(PROGRAMS\\)", "all: libcqp.a", 1L),
          replace = list("^TOP\\s=\\s\\$\\(shell\\spwd\\)/\\.\\.", "TOP = $(R_PACKAGE_SOURCE)", 1L),
          remove_lines = list("\\s+-\\$\\(RM\\)\\slex\\.yy\\.c\\sparser\\.tab\\.c\\sparser\\.tab\\.h", 1L),
          remove_lines = list('^\\s*@\\$\\(ECHO\\)\\s".*?"\\s*$', NA) # r1690 is beautiful - but we nee verbosity
        ),
        
        "src/cwb/cqp/hash.c" = c(
          # File not there any more at revision 1690
          list(),
          if (revision == 1069) list(
            # The symbols/functions 'hash_string', 'is_prime' and 'find_prime' are
            # defined exactly the same way in cl/lexhash.c and cqp/hash.c. To
            # avoid linker errors, the functions are commented out in cqp/hash.c
            
            insert_before = list("^\\s*int\\s*$", "/*", 1),
            insert_before = list("^\\s*int\\s*$", "/*", 2),
            insert_before = list("^unsigned\\sint\\s*$", "/*", 1),
            insert_after = list("^\\s*\\}\\s*$", "*/", 1),
            insert_after = list("^\\s*\\}\\s*$", "*/", 2),
            insert_after = list("^\\s*\\}\\s*$", "*/", 3),
            replace = list("^(.*?)\\s*/\\*\\swill\\sexit\\son\\sint\\soverflow\\s\\*/", "\\1", 1)
            
          )
        ),
        
        "src/cwb/cqp/macro.c" = c(
          # The include of hash.h is gone in revision 1690 and there is a macro.h
          list(),
          if (revision == 1069) list(
            insert_before = list('#include\\s"hash\\.h"', '#include "../cl/lexhash.h" /* newly added by AB */', 1)
          )
          
        ),
        
        "src/cwb/cqp/eval.c" = c(
          list(
            replace = list("^(\\s*)if\\s\\(ctptr->pa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->pa_ref.del) {", 2),
            replace = list("^(\\s*)if\\s\\(ctptr->pa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->pa_ref.del) {", 1),
            replace = list("^(\\s*)if\\s\\(ctptr->sa_ref\\.delete\\)\\s\\{", "\\1if (ctptr->sa_ref.del) {", 1),
            replace = list("^(\\s*)if\\s\\(ctptr->idlist\\.delete\\)\\s\\{", "\\1if (ctptr->idlist.del) {", 1)
          ),
          
          # The corpus_size variable is used in revision 1690. This is commented out to see whether problems persist.
          if (revision == 1069) list(
            # eval.c: In function ???cqp_run_tab_query???:
            #   eval.c:2911:21: warning: variable ???corpus_size??? set but not used [-Wunused-but-set-variable]
            # int smallest_col, corpus_size;
            replace = list("^(\\s*)int\\ssmallest_col,\\scorpus_size;", "\\1int smallest_col;", 1),
            replace = list("^(\\s*)corpus_size\\s=\\sevalenv->query_corpus->mother_size;", "\\1/* corpus_size = evalenv->query_corpus->mother_size; */", 1),
            
            # The line 'assert(col->type = tabular)' (line 2891) is turned into
            # 'assert((col->type = tabular));' to avoid -Wparentheses error (on
            # Debian CRAN machine)
            replace = list("^(\\s*)assert\\(col->type\\s=\\stabular\\);", "\\1assert((col->type = tabular));", 1)
          ),
          if (revision >= 1690) list(
            # eval.c:190:23: error: unknown type name 'bool'
            # bool keep_old_ranges)
            # eval.c:2815:27: error: use of undeclared identifier 'false'
            # false);
            # eval.c:3249:64: error: use of undeclared identifier 'false'
            # set_corpus_matchlists(evalenv->query_corpus, &result, 1, false); /* return empty result set */
            # eval.c:3397:60: error: use of undeclared identifier 'false'
            # set_corpus_matchlists(evalenv->query_corpus, &result, 1, false);
            replace = list("bool\\skeep_old_ranges)\\s*$", "int keep_old_ranges)", 1),
            replace = list("^(\\s*)false);", "\\10);", 1),
            replace = list("^(\\s*set_corpus_matchlists\\(evalenv->query_corpus,\\s&result,\\s1,\\s)false);", "\\10);", 1),
            replace = list("^(\\s*set_corpus_matchlists\\(evalenv->query_corpus,\\s&result,\\s1,\\s)false);", "\\10);", 1)
          )
        ),
        
        "src/cwb/cqp/eval.h" = c(
          list(
            replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 3),
            replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 2),
            replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 1)
          ),
          # No matches for 'int delete' in revision 1690
          if (revision == 1069) list(
            # global variables prepended by "extern"
            extern = list(
              "int eep;", # no match in r1690
              "EvalEnvironment Environment[MAXENVIRONMENT];", # extern'ed in r1690
              "EEP CurEnv, evalenv;" # extern'ed in r1690
            )
          )

        ),
        
        "src/_eval.h" = c(
          list(
            delete_line_before = list("^/\\*\\*\\sNumber\\sof\\sAVStructures", 1L, NA),
          insert_before = list(
            "^/\\*\\*\\sNumber\\sof\\sAVStructures",
            c(
              '#include "corpmanag.h"',
              if (revision >= 1690) '#include "symtab.h"' else '',
              '',
              if (revision < 1690) c('typedef struct _label_entry {',
                '  int        flags;',
                '  char      *name;',
                '  int        ref;             /**< array index the label refers to */',
                '    struct _label_entry *next;',
                '} *LabelEntry;',
                '',
                'typedef struct _symbol_table {',
                '  LabelEntry  user;                /**< user namespace */',
                '    LabelEntry  rdat;                /**< namespace for LAB_RDAT labels */',
                '    int next_index;                  /**< next free reference table index */',
                '} *SymbolTable;',
                '') else c(),
              'typedef struct dfa {',
              '  int Max_States;         /**< max number of states of the current dfa;',
              '  state no. 0 is the initial state.             */',
              '    int Max_Input;          /**< max number of input chars of the current dfa. */',
              '    int **TransTable;       /**< state transition table of the current dfa.    */',
              '    Boolean *Final;         /**< set of final states.                          */',
              '    int E_State;            /**< Error State -- it is introduced in order to',
              '  *   make the dfa complete, so the state transition',
              '  *   is a total mapping. The value of this variable',
              '  *   is Max_States.',
              '  */',
              '} DFA;'
            ),
            1L
          ),
          replace = list("^\\s*Boolean\\seval_bool\\(Constrainttree\\sctptr,\\sRefTab\\srt,\\sint\\scorppos\\);", "", 1L),
          replace = list("^#endif.*$", "", 1L),
          
          replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 3),
          replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 2),
          replace = list("^(\\s*)int(\\s+)delete;", "\\1int\\2del;", 1)
          
          )
        ),
        
        "src/cwb/cqp/hash.h" = c(
          list(),
          
          # This file is not there with revision r1690 any more
          if (revision == 1069) list(
            replace = list("(\\s*int\\sis_prime\\(int\\sn\\);)", "/* \\1 */", 1),
            replace = list("^(\\s*int\\sfind_prime\\(int\\sn\\);)", "/* \\1 */", 1),
            replace = list("^(\\s*unsigned\\sint\\shash_string\\(char\\s\\*s\\);)", "/* \\1 */", 1)
          )
        ),
        
        "src/cwb/cqp/html-print.c" = c(
          list(),
          
          # None of these lines is findable in r1690
          if (revision == 1069) list(
            # html-print.c: In function ???html_print_context???:
            #   html-print.c:317:9: warning: variable ???s??? set but not used [-Wunused-but-set-variable]
            # char *s;
            replace = list("^(\\s*)char\\s\\*s;", "\\1/* char *s; */", 1),
            replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
            replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
            
            # html-print.c: In function ???html_print_output???:
            #   html-print.c:418:18: warning: variable ???strucs??? set but not used [-Wunused-but-set-variable]
            # AttributeList *strucs;
            replace = list("^(\\s*)AttributeList\\s\\*strucs;", "\\1/* AttributeList *strucs; */", 1),
            replace = list("^(\\s*)strucs\\s=\\scd->printStructureTags;", "\\1/* strucs = cd->printStructureTags; */", 1)
          )
        ),
        
        "src/cwb/cqp/latex-print.c" = c(
          list(),
          # Not findable in r1690
          if (revision == 1069) list(
            # latex-print.c: In function ???latex_print_context???:
            #   latex-print.c:236:9: warning: variable ???s??? set but not used [-Wunused-but-set-variable]
            # char *s;
            replace = list("^(\\s*)char\\s\\*s;", "\\1/* char *s; */", 1),
            replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1),
            replace = list('^(\\s*)s\\s=\\s"error";', '\\1/* s = "error"; */', 1)
          )
        ),
        
        "src/cwb/cqp/options.h" = c(
          list(),
          
          # Prepending 'extern' not necessary in r1690 - is there already
          if (revision == 1069) list(
            replace = list("^(\\s*)enum\\s_which_app\\s\\{\\sundef,\\scqp,\\scqpcl,\\scqpserver}\\swhich_app;", "\\1enum _which_app { undef, cqp, cqpcl, cqpserver} extern which_app;", 1),
            extern = list(
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
              "int handle_sigpipe;"
            )
          )
        ),
        
        "src/cwb/cqp/output.h" = list(
          # Unchanged for r1690 - is it really harmful?
          
          # Comment out open_temporary_file(char *tmp_name_buffer) in
          # cqp/output.c, in cqp/output.h and in usage in functions calling
          # this function: ComputeGroupExternally (cqp/groups.c) and
          # SortExternally (cqp/ranges.c)
          # replace = list("^(\\s*)FILE\\s\\*open_temporary_file\\(char\\s\\*tmp_name_buffer);", "\\1/* FILE *open_temporary_file(char *tmp_name_buffer); */", 1)
        ),
        
        "src/cwb/cqp/parse_actions.c" = c(
          if (revision >= 1690) list(
            replace = list("^(\\s*)c->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1c->idlist.del = left->pa_ref.del;", 1),
            
            replace = list("^(\\s*)left->pa_ref\\.delete\\s=\\s0;", "\\1left->pa_ref.del = 0;", 1),
            replace = list("^(\\s*)node->idlist\\.delete\\s=\\s0;", "\\1node->idlist.del = 0;", 1),
            
            replace = list("^(\\s*)result->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1result->idlist.del = left->pa_ref.del;", 1),
            replace = list("^(\\s*)result->pa_ref\\.delete\\s=\\sauto_delete;", "\\1result->pa_ref.del = auto_delete;", 1),
            replace = list("^(\\s*)result->sa_ref\\.delete\\s=\\sauto_delete;", "\\1result->sa_ref.del = auto_delete;", 1),
            
            replace = list("^(\\s*)result->pa_ref\\.delete\\s=\\s0;", "\\1result->pa_ref.del = 0;", 1),
            replace = list("^(\\s*)result->pa_ref\\.delete\\s=\\sauto_delete;", "\\1result->pa_ref.del = auto_delete;", 1),
            replace = list("^(\\s*)result->sa_ref\\.delete\\s=\\s0;", "\\1result->sa_ref.del = 0;", 1)
            
          ),
          if (revision == 1069) list(
            replace = list("^(\\s*)c->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1c->idlist.del = left->pa_ref.del;", 1),
            replace = list("^(\\s*)left->pa_ref\\.delete\\s=\\s0;", "\\1left->pa_ref.del = 0;", 1),
            replace = list("^(\\s*)node->idlist\\.delete\\s=\\s0;", "\\1node->idlist.del = 0;", 1),
            replace = list("^(\\s*)res->idlist\\.delete\\s=\\sleft->pa_ref\\.delete;", "\\1res->idlist.del = left->pa_ref.del;", 1),
            replace = list("^(\\s*)res->pa_ref\\.delete\\s=\\sauto_delete;", "\\1res->pa_ref.del = auto_delete;", 1),
            replace = list("^(\\s*)res->sa_ref\\.delete\\s=\\sauto_delete;", "\\1res->sa_ref.del = auto_delete;", 1),
            replace = list("^(\\s*)res->pa_ref\\.delete\\s=\\s0;", "\\1res->pa_ref.del = 0;", 1),
            replace = list("^(\\s*)res->pa_ref\\.delete\\s=\\sauto_delete;", "\\1res->pa_ref.del = auto_delete;", 1),
            replace = list("^(\\s*)res->sa_ref\\.delete\\s=\\s0;", "\\1res->sa_ref.del = 0;", 1)
          )
        ),
        
        "src/cwb/cqp/corpmanag.c" = c(
          list(),
          
          # In r1690, this is not "extern" but "static"
          if (revision == 1069) list(
            # global variable corpuslist prepended by extern statement
            extern = list("CorpusList *corpuslist;")
            
          )
        ),
        
        "src/cwb/cqp/corpmanag.h" = c(
          list(),
          
          # extern'ed in original CWB code
          if (revision == 1069) list(
            # global variables current_corpus and corpusList prepended by "extern" statement
            extern = list(
              "CorpusList *current_corpus;",
              "CorpusList *corpuslist;"
            )
          )
          
        ),
        
        "src/cwb/cqp/regex2dfa.c" = c(
          
          list(),
          if (revision == 1069) list(
            # regex2dfa.c: In function ???Parse???:
            #   regex2dfa.c:544:7: warning: variable ???ignore_value??? set but not used [-Wunused-but-set-variable]
            # int ignore_value;             /* ignore value of POP() macro */
            replace = list("^(\\s*)int\\signore_value;", "\\1int ignore_value __attribute__((unused));", 1)
          )
        ),
        
        "src/cwb/cqp/sgml-print.c" = c(
          list(),
          
          # No matches for these patches in r1690
          if (revision == 1069) list(
            # sgml-print.c: In function ???sgml_print_output???:
            #   sgml-print.c:325:18: warning: variable ???strucs??? set but not used [-Wunused-but-set-variable]
            # AttributeList *strucs;
            replace = list("^(\\s*)AttributeList\\s\\*strucs;", "\\1/* AttributeList *strucs; */", 1),
            
            replace = list("^(\\s*)strucs\\s=\\scd->printStructureTags;", "\\1/* strucs = cd->printStructureTags; */", 1)
            
          )
        ),
        
        "src/cwb/cqp/parser.tab.c" = c(
          list(
            # cqpmessage(Error, "CQP Syntax Error: %s\n\t%s <--", s, QueryBuffer);
            # replaced by:
            #   cqpmessage(Error, "CQP Syntax Error: %s\n <--", s);
            # The previous version resulted in an awful total crash. QueryBuffer causes the problem! 
            replace = list('^(\\s*)cqpmessage\\(Error,\\s"CQP\\sSyntax\\sError:.*?",\\ss,\\sQueryBuffer\\);', '\\1cqpmessage(Error, "CQP Syntax Error: %s", s);', 1L)
          ),
          
          if (revision == 1069) list(
            replace = list("^(\\s+)int\\sok;", "\\1int ok __attribute__((unused));", 4L)
          )
        ),
        
        "src/cwb/cqp/cqp.h" = c(
          
          list(),
          if (revision == 1069) list(
            # global variables prepended by "extern"
            extern = list(
              "CYCtype LastExpression;",
              "int exit_cqp;",
              "char *cqp_input_string;", # starting from r1075 externed anyway
              "int cqp_input_string_position;", # starting from r1075 externed anyway
              "int initialize_cqp(int argc, char **argv);",
              "int cqp_parse_file(FILE *fd, int exit_on_parse_errors);",
              "int cqp_parse_string(char *s);",
              "int EvaluationIsRunning;",
              "int setInterruptCallback(InterruptCheckProc f);",
              "void CheckForInterrupts(void);",
              "int signal_handler_is_installed;",
              "void install_signal_handler(void);"
            )
          )
        ),
        
        
        "src/cwb/CQi/auth.c" = c(
          list(),
          if (revision >= 1690) list(
            insert_before = list('^#include\\s"auth\\.h"', c("void Rprintf(const char *, ...);", ""), 1L)
          ),
          if (revision == 1069) list(
            insert_before = list("/\\*\\sdata\\sstructures\\s\\(internal\\suse\\sonly\\)\\s\\*/", c("void Rprintf(const char *, ...);", ""), 1)
          )
        ),
        
        "src/cwb/CQi/server.c" = c(
          list(
            insert_before = list("^\\/\\*", c("void Rprintf(const char *, ...);", ""), 3L)
          ),
          
          # File 'lexhash.h' not there any more at r1690
          if (revision == 1069) list(
            insert_after = list('#include "\\.\\./cqp/hash\\.h"', '#include "../cl/lexhash.h" /* inserted by AB */', 1)
          )
        ),
        
        "src/cwb/CQi/cqpserver.c" = list(
          # stable r1069 - r1690
          insert_after = list('#include "\\.\\./cqp/groups\\.h"', "void Rprintf(const char *, ...);", 1)
        ),
        
        "src/cwb/utils/cwb-encode.c" = c(
          list(
            # leaves things somewhere in between in r1069 - but does not matter (?)
            insert_before = list(
              "^#include\\s<ctype.h>",
              c(
                "/* included by AB to ensure that winsock2.h is included before windows.h */",
                "#ifdef __MINGW__",
                "#include <winsock2.h> /* AB reversed order, in original CWB code windows.h is included first */",
                "#endif",
                ""
              ),
              1L
            ),
            
            
            delete_line_before = list("^/\\*\\s-*\\s\\*/", 1L, 1L), # purely cosmetic
            insert_before = list(
              "^/\\*\\s-*\\s\\*/",
              c("void Rprintf(const char *, ...); /* alternative to include R_ext/Print.h */", ""),
              1L
            ),
            
            replace = list('^#include\\s"\\.\\./cl/endian\\.h"\\s*', '#include "../cl/endian2.h"', 1L),
            
            replace = list("^encode_print_time\\(FILE\\s\\*stream,\\schar\\s\\*msg\\)", "encode_print_time(char *msg)", 1L),
            replace = list("^(\\s*)encode_print_time\\(stderr,\\s*", "\\1encode_print_time(", NA),
            
            # return value of encode_error should be 'int' (not 'void') - stable 1069-1690
            delete_line_before = list("^\\s*encode_error\\(char\\s+\\*format,\\s\\.\\.\\.\\)", 1L, 1L),
            insert_before = list("^\\s*encode_error\\(char\\s+\\*format,\\s\\.\\.\\.\\)", "int", 1L),
            
            replace = list("^(\\s*)exit\\(1\\);", "\\1return 1;", NA),
            
            replace = list('^(\\s*)(progname\\s=\\s.*?;)\\s*$', "\\1/* \\2 */", 1L),
            replace = list("^(\\s*)char\\s\\*progname\\s=\\s(.*?);", "\\1/* char *progname = \\2; */", 1L),
            
            replace = list("^\\s*(input_files\\s=\\scl_new_string_list\\(\\);)\\s*", "/* \\1 */", 1L),
            
            # extern variables
            replace = list("^(\\s*)char\\s\\*field_separators\\s=\\s.*?;", "\\1extern char *field_separators;", 1L),
            replace = list("^(\\s*)char\\s\\*undef_value\\s*=.*?;", "\\1extern char *undef_value;", 1L),
            replace = list("^(\\s*)int\\sdebug\\s*=.*?;", "\\1extern int debugmode;", 1L),
            replace = list("^(\\s*)int\\ssilent\\s*=.*?;", "\\1extern int quietly;", 1L),
            replace = list("^(\\s*)int\\sverbose\\s*=.*?;", "\\1extern int verbose;", 1L),
            replace = list("^(\\s*)int\\sxml_aware\\s*=.*?;", "\\1extern int xml_aware;", 1L),
            replace = list("^(\\s*)int\\sskip_empty_lines\\s*=.*?;", "\\1extern int skip_empty_lines;", 1L),
            replace = list("^(\\s*)unsigned\\sline\\s*=.*?;", "\\1extern unsigned line;", 1L),
            replace = list("^(\\s*)int\\sstrip_blanks\\s*=.*?;", "\\1extern int strip_blanks;", 1L),
            replace = list("^(\\s*)cl_string_list\\sinput_files\\s*=.*?;", "\\1extern cl_string_list input_files;", 1L),
            replace = list("^(\\s*)int\\snr_input_files\\s*=.*?;", "\\1extern int nr_input_files;", 1L),
            replace = list("^(\\s*)int\\scurrent_input_file\\s*=.*?;", "\\1extern int current_input_file;", 1L),
            replace = list("^(\\s*)char\\s\\*current_input_file_name\\s*=.*?;", "\\1extern char *current_input_file_name;", 1L),
            replace = list("^(\\s*)FILE\\s\\*input_(fd|fh)\\s*=.*?;", "\\1extern FILE *input_\\2;", 1L),
            replace = list("^(\\s*)unsigned\\slong\\sinput_line\\s*=.*?;", "\\1extern unsigned long input_line;", 1L),
            replace = list("^(\\s*)char\\s\\*registry_file\\s*=.*?;", "\\1extern char *registry_file;", 1L),
            replace = list("^(\\s*)char\\s\\*directory\\s*=.*?;", "\\1extern char *directory;", 1L),
            replace = list("^(\\s*)CorpusCharset\\s*encoding_charset;", "\\1extern CorpusCharset encoding_charset;", 1L),
            replace = list("^(\\s*)int\\sclean_strings\\s*=.*?;", "\\1extern int clean_strings;", 1L),
            
            replace = list("^(\\s*)int\\ss_encoder_ix\\s=\\s0;", "\\1extern int s_encoder_ix;", 1L),
            replace = list("^(\\s*)int\\sp_encoder_ix\\s=\\s0;", "\\1extern int p_encoder_ix;", 1L),
            replace = list("^(\\s*)s_att_builder\\s\\*conll_sentence_satt\\s=\\sNULL;", "\\1extern s_att_builder *conll_sentence_satt;", 1L),
            replace = list("^(\\s*)cl_lexhash\\sundeclared_sattrs\\s=\\sNULL;", "\\1extern cl_lexhash undeclared_sattrs;", 1L),
            replace = list("^(\\s*)int\\sauto_null\\s=\\s0;", "\\1extern int auto_null;", 1L),

            # delete_line_beginning_with = list("^/\\*\\*\\sname\\sof\\sthe\\scurrently\\srunning\\sprogram", 1L, 1L),
            
            replace = list("\\(\\!(\\s*)silent", "(!\\1quietly", NA),
            replace = list("\\(debug\\)", "(debugmode)", NA),
            
            replace = list("register\\s+", "", NA), # not well understood
            
            delete_line_before = list("^encode_usage\\(void\\)\\s*$", 1L, 4L), # stable r1069/r1690
            delete_line_beginning_with = list(
              "^encode_usage\\(void\\)\\s*$",
              1L,
              if (revision == 1069) 72L else 78L # longer in revision 1690!
            ),
            
            delete_line_before = list("^encode_parse_options\\(int\\sargc,\\schar\\s\\*\\*argv\\)\\s*$", 1L, 10L), # r1069 and r1690
            delete_line_beginning_with = list(
              "^encode_parse_options\\(int\\sargc,\\schar\\s\\*\\*argv\\)\\s*$",
              1L,
              if (revision == 1069) 237L else 279L
            ), 
            
            # turn main() into function cwb_encode_worker()
            delete_line_before = list("^\\s*\\*\\s+MAIN\\(\\)\\s+\\*\\s*$", 1L, 2L),
            insert_before = list("^\\s*\\*\\s+MAIN\\(\\)\\s+\\*\\s*$", c("int cwb_encode_worker(cl_string_list input_files){"), 1L),
            delete_line_beginning_with = list("^\\s*\\*\\s+MAIN\\(\\)\\s+\\*\\s*$", 1L, 17),
            
            replace = list("^(\\s*)encode_parse_options\\(argc,\\sargv\\);", "\\1/* encode_parse_options(argc, argv); */", 1L),
            replace = list("Rprintf\\(registry_f(d|h),", "fprintf(registry_f\\1,", NA)
          ),
          if (revision == 1069) list(
            replace = list('^#include\\s"\\.\\./cl/lexhash\\.h"\\s*$', '/* #include "../cl/lexhash.h" */ ', 1L),
            
            replace = list("(struct\\s_|}\\s|^\\s*|\\()Range", "\\1SAttEncoder", NA),
            replace = list("'children'\\s\\(SAttEncoder\\s\\*\\)", "'children' (Range *)", 1L),
            
            replace = list("Pointer\\sto\\sthe\\snew\\sRange", "Pointer to the new SAttEncoder", 1L),
            
            # r1690 has cl.h as an include and uses cl_string_chomp()
            insert_before = list(
              "^/\\*\\*\\sname\\sof\\sthe\\scurrently\\srunning\\sprogram",
              c(          
                "/* ----------------- cl/special_chars.c - is here temporarily ------------- */",
                "",            
                "/**",
                " * Removes all trailing CR and LF characters from specified string (in-place).",
                " *",
                " * The main purpose of this function is to remove trailing line breaks from input",
                " * lines regardless of whether a text file is in Unix (LF) or Windows (CR-LF) format.",
                " * All text input except for simple numeric data should be passed through cl_string_chomp().",
                " *",
                " * @param s     String to chomp (modified in-place).",
                " */",
                "void",
                "  string_chomp(char *s) {",
                "    char *point = s;",
                "    /* advance point to NUL terminator */",
                "    while (*point)",
                "      point++;",
                "    point--; /* now points at last byte of string */",
                "    /* delete CR and LF, but don't move beyond start of string */",
                "    while (point >= s && (*point == '\\r' || *point == '\\n')) {",
                "      *point = '\\0';",
                "      point--;",
                "    }",
                "  }",
                "",
                "/* ----------------- END ------------- */"
              ),
              1L, 1L
            ),
            
            replace = list("^(\\s*)char\\s\\*corpus_character_set\\s*=.*?;", "\\1extern char *corpus_character_set;", 1L),
            
            replace = list("^(\\s*)int\\srange_ptr\\s*=.*?;", "\\1extern int range_ptr;", 1L),
            replace = list("^(\\s*)SAttEncoder\\sranges\\[MAXRANGES\\];", "extern SAttEncoder ranges[MAXRANGES];", 1L),
            replace = list("^(\\s*)cl_lexhash\\sundeclared_sattrs\\s*=.*?;", "\\1extern cl_lexhash undeclared_sattrs;", 1L),
            replace = list("^(\\s*)int\\swattr_ptr\\s=\\s0;", "\\1extern int wattr_ptr;", 1L),
            replace = list("Rprintf\\(f(d|h)", "fprintf(f\\1", NA),
            replace = list('Rprintf\\(rng->avs', 'fprintf(rng->avs', 1L)
            
          ),
          if (revision >= 1690) list(
            replace = list("^(\\s*)const\\schar\\s\\*encoding_charset_name\\s*=.*?;", "\\1extern const char *encoding_charset_name;", 1L), # 1069 encoding_character_set!
            replace = list("^(\\s*)int\\snumbered\\s=\\s0;", "\\1extern int numbered;", 1L),
            replace = list("^(\\s*)int\\sencode_token_numbers\\s=\\s0;", "\\1extern int encode_token_numbers;", 1L),
            replace = list("^(\\s*)char\\s\\*conll_sentence_attribute\\s=\\sNULL;", "\\1extern char *conll_sentence_attribute;", 1L),
            replace = list('Rprintf\\(encoder->avs_fh', 'fprintf(encoder->avs_fh', 1L)
          )
        ),

        "src/cwb/utils/globals.h" = c(
          
          list(
            delete_line_before = list("^/\\*\\s-*\\s\\*/", 1L, NA),
            delete_line_beginning_with = list("^\\s*/\\*\\*\\sname\\sof\\sthe\\scurrently\\srunning\\sprogram\\s\\*/\\s*$", 1L, NA),
            
            replace = list('^(#define\\sFIELDSEPS\\s*)(".*?")', '\\1(char*)\\2', 1L),
            
            replace = list("^(\\s*)int\\sdebug\\s*=\\s*(.*?);", "\\1int debugmode = \\2;", 1L),
            replace = list("^(\\s*)int\\ssilent\\s*=\\s(.*?);", "\\1int quietly = \\2;", 1L),
            
            # To avoid warning: ISO C++11 does not allow conversion from string literal to 'char *'
            replace = list("^\\s*char\\s*\\*\\s*undef_value\\s*=\\s*CWB_PA_UNDEF_VALUE;(.*?)$", "char *undef_value = (char *)CWB_PA_UNDEF_VALUE;\\1", 1L)
            
          ),
          
          if (revision < 1690) list(
            # things that are different in the later revision
            replace = list("(struct\\s_|}\\s|^\\s*|\\()Range", "\\1SAttEncoder", NA),
            replace = list("'children'\\s\\(SAttEncoder\\s\\*\\)", "'children' (Range *)", 1L),
            replace = list('^(#define\\sUNDEF_VALUE\\s*)("__UNDEF__")', '\\1(char*)\\2', 1L),
            replace = list('^(char\\s\\*corpus_character_set\\s*=\\s*)("latin1";)', '\\1(char*)\\2', 1L)
          )
        ),
        
                
        "src/cwb/utils/cwb-compress-rdx.c" = list(
          
          insert_before = list("#include <math.h>", c("void Rprintf(const char *, ...);", ""), 1L),
          
          replace = list("^char\\s\\*progname\\s=\\sNULL;", "/* char *progname = NULL; */", 1L),
          replace = list("^char\\s\\*corpus_id\\s=\\sNULL;", "/* char *corpus_id = NULL; */", 1L),
          replace = list('^FILE\\s\\*debug_output;', "/* FILE *debug_output; */", 1L),
          replace = list("^int\\sdebug\\s=\\s0;", "/* extern int debug = 0; */", 1L),
          
          replace = list("^(\\s*)exit\\(.*?\\);", "\\1return;", NA),
          
          extern = list("Corpus *corpus;"),

          # remove function 'usage()' - works for r1069 and r1690
          delete_line_beginning_with = list("^\\s*/\\*\\s-+\\s\\*/\\s*$", 2L, 33L),
          
          
          # remove main() function
          delete_line_beginning_with = list(
            if (revision == 1069) "^\\s*/\\*\\s\\*+\\s\\*\\\\\\s*$" else "^\\s*/\\*\\s=+\\s\\*\\s*$",
            1L,
            NA
          ),
          
          # * - global variable 'debug' replaced by local variable that is passed around
          replace = list(
            "^(\\s*compress_reversed_index\\(Attribute\\s\\*attr,\\schar\\s\\*output_fn)\\)\\s*$",
            "\\1, char *corpus_id, int debug)",
            1L
          ),
          # global variable corpus_id commented out, passed expressively into functions
          replace = list(
            "^(\\s*decompress_check_reversed_index\\(Attribute\\s\\*attr,\\schar\\s\\*output_fn)\\)",
            "\\1, char *corpus_id, int debug)",
            1L
          ),
          
          # causes a crash beause debug_output is commented out / undefined
          replace = list("^(\\s*)(if\\s\\(debug_output\\s\\!=\\sstderr\\))", "\\1/* \\2 */", 1L),
          replace = list("^(\\s*)(fclose\\(debug_output\\));", "\\1/* \\2 */", 1L)
        ),
        
        "src/cwb/utils/cwb-huffcode.c" = c(
          list(
            insert_before = list(
              if (revision >= 1690) '#include "../cl/cl.h"' else '#include\\s"\\.\\./cl/globals\\.h"',
              c("void Rprintf(const char *, ...);", ""),
              1L
            ),
            
            replace = list("^char\\s\\*progname;", "/* char *progname; */", 1L), # unchanged in r1690
            
            # all uses of 'protocol' commented out, starting with a loop
            replace = list("^(\\s*)int\\si;", "\\1/* int i; */", 1L), 
            replace = list("^(\\s*)(for\\s\\(i\\s=\\s0;\\si\\s<\\sindent\\s\\*\\s3;\\si\\+\\+\\))", "\\1/* \\2", 1L),
            replace = list("^(\\s*putc\\(\\(i\\s%\\s3\\)\\s==\\s0\\s\\?\\s'\\|'\\s:\\s'\\s',\\sprotocol\\);)", "\\1 */", 1L),
            
            # return value of decode_check_huff turned into 'int'
            # corpus_id is passed explicitly into decode_check_huff
            delete_line_before = list("^(\\s*)decode_check_huff\\(Attribute\\s\\*attr,\\schar\\s\\*fname\\)", 1L, 1L),
            insert_before = list("^(\\s*)decode_check_huff\\(Attribute\\s\\*attr,\\schar\\s\\*fname\\)", "int ", 1L),
            replace = list("^(\\s*)decode_check_huff\\(Attribute\\s\\*attr,\\schar\\s\\*fname\\)", "\\1decode_check_huff(Attribute *attr, char *corpus_id, char *fname)", 1L),
            insert_after = list('printf\\("\\!\\!\\sYou\\scan\\sdelete\\sthe\\sfile', "  return 1;", 1L),

            replace = list("^(\\s*)exit\\(1\\);", "\\1return 1;", NA),
            
            # int return value. not void
            replace = list("^(\\s*)return;\\s*$", "\\1return 0;", 1L),
            
            extern = list("Corpus *corpus;"),
            
            delete_line_before = list("^\\s*/\\*\\s\\*+\\s\\*(\\\\|)\\s*$", 1L, 37L),
            
            # Functions usage() and main() deleted
            delete_line_beginning_with = list("^\\s*/\\*\\s\\*+\\s\\*(\\\\|)\\s*$", 1L, NA)
          ),
          if (revision < 1690) list(
            replace = list("^(\\s*)bprintf\\(heap\\[i\\],\\scodelength\\[i\\],\\sprotocol\\);", "\\1/* bprintf(heap[i], codelength[i], protocol); */", 1L),
            replace = list("^(\\s*)return;\\s*$", "\\1return 0;", 1L), # deliberately a second time!
            
            replace = list("^(\\s*)int\\snode,\\sdepth;", "\\1/* int node, depth; */", 1L),
            replace = list("^(\\s*)node\\s=\\s1;", "\\1/* node = 1; */", 1L),
            replace = list("^(\\s*)depth\\s=\\s0;", "\\1/* depth = 0; */", 1L),
            replace = list("^(\\s*)int\\snr_codes\\s=\\s0;", "\\1/* int nr_codes = 0; */", 1L),
            replace = list("^(\\s*)nr_codes\\s=\\s0;", "\\1/* nr_codes = 0; */", 1L)
          ),
          if (revision >= 1690) list(
            # The compute_code_lengths() function is declared 'static int', but that prevents the makeheaders
            # utility from including it in the autogenerated header.
            delete_line_before = list("^\\s*compute_code_lengths\\(Attribute\\s\\*attr,\\sHCD\\s\\*hc,\\schar\\s\\*fname\\)\\s*$", 1L, 1L),
            insert_before = list("^\\s*compute_code_lengths\\(Attribute\\s\\*attr,\\sHCD\\s\\*hc,\\schar\\s\\*fname\\)\\s*$", "int", 1L)
          )
        ),

        "src/cwb/utils/cwb-makeall.c" = list(
          
          insert_before = list(
            '#include\\s"\\.\\./cl/(cwb-|)globals\\.h"',
            c(
              "void Rprintf(const char *, ...); /* alternative to include R_ext/Print.h */",
              "",
              "/* included by AB to ensure that winsock2.h is included before windows.h */",
              "#ifdef __MINGW__",
              "#include <winsock2.h> /* AB reversed order, in original CWB code windows.h is included first */",
              "#endif",
              "", "", "",
              "#include <stdio.h>",
              "#include <stdlib.h>",
              "#include <string.h>",
              if (revision >= 1690) NULL else '#include "../cl/cl.h"' # cl.h is already included in r1690
            ),
            1L
          ),
          
          delete_line_beginning_with = list('#include\\s"\\.\\./cl/corpus\\.h', 1L, 0L),
          replace = list('^#include\\s"\\.\\./cl/endian\\.h"\\s*', '#include "../cl/endian2.h"', 1L),
          
          # exit(1) -> return 1 
          replace = list("^(\\s*)exit\\(1\\);", "\\1return 1;", NA),
          
          # return -> return 0
          replace = list("^(\\s*)return;(\\s*)$", "\\1return 0;\\2", NA),
          
          # stable r1069 - r1690. But maybe obsolete, because cwb-makeall.c is not compiled?
          replace = list("^\\s*fflush\\(stdout\\);\\s*$", "/* fflush(stdout); */", NA),

          # change return value 'void' of makeall_do_attribute() to 'int'
          delete_line_before = list("makeall_do_attribute\\(Attribute\\s\\*attr,\\sComponentID\\scid,\\sint\\svalidate\\)", 1L, 1L),
          insert_before = list("makeall_do_attribute\\(Attribute\\s\\*attr,\\sComponentID\\scid,\\sint\\svalidate\\)", "int", 1L),
          
          # change return value 'void' of makeall_make_component() to 'int'
          delete_line_before = list("makeall_make_component\\(Attribute\\s\\*attr,\\sComponentID\\scid\\)", 1L, 1L),
          insert_before = list("makeall_make_component\\(Attribute\\s\\*attr,\\sComponentID\\scid\\)", "int", 1L),
          
          delete_line_before = list("^Corpus\\s\\*corpus;", 1L, 1L), # Necessary? Removes a comment?
          delete_line_beginning_with = list("/\\*\\*\\sName\\sof\\sthis\\sprogram\\s\\*/", 1L, 1L), # Necessary? Removes only a comment?
          
          # Changes return value of makeall_do_attribute() to int (works for r1069 and r1690)
          delete_line_before = list("\\*\\sPrints\\sa\\susage\\smessage\\sand\\sexits\\sthe\\sprogram\\.", 1L, 4L),
          insert_before = list("\\*\\sPrints\\sa\\susage\\smessage\\sand\\sexits\\sthe\\sprogram\\.", c("    return 0;", "", "  }"), 1L),
          
          
          delete_line_beginning_with = list("\\*\\sPrints\\sa\\susage\\smessage\\sand\\sexits\\sthe\\sprogram\\.", 1L, NA),
          
          # changes the return value of makeall_make_component() from void to int
          delete_line_before = list(
            "\\*\\sValidates\\sthe\\sREVCORP\\scomponent\\sof\\sthe\\sgiven\\sattribute\\.",
            1L,
            if (revision >= 1690) 5L else 6L
            ),
          insert_before = list(
            "\\*\\sValidates\\sthe\\sREVCORP\\scomponent\\sof\\sthe\\sgiven\\sattribute\\.",
            c("  return 0;", "", "  }", "", "", "", "/**"),
            1L
          )
          
        ),
        
        "src/cwb/definitions.mk" = c(
          # stable r1069 - r1690
          list(
            replace = list("(\\$\\(error\\sConfiguration\\svariable\\sRANLIB)", "# \\1", 1L)
          ),
          if (revision >= 1690) list(
            delete_line_beginning_with = list('^\\s+@\\$\\(ECHO\\)\\s"\\s+\\.+\\scompile\\ssource\\sfile"', 1L, 1L)
          )
          
        ),
        
        "src/cwb/config/platform/darwin-64" = list(
          # stable r1069-r1690
          replace = list("^(CFLAGS\\s*=.*?)\\s+-march=native\\s+(.*?)$", "\\1 \\2", 1L)
        ),
        
        "src/cwb/config/platform/darwin" = list(
          # stable r1069-r1690
          replace = list("^READLINE_DEFINES\\s:=.*?$", "READLINE_DEFINES =", 1L),
          replace = list("^READLINE_LIBS\\s:=.*?$", "READLINE_LIBS =", 1L)
        ),
        
        "src/cwb/config/platform/darwin-port-core2" = c(
          # This file is gone in r1690
          list(),
          if (revision == 1069) list(
            replace = list("^(CFLAGS\\s*=.*?)\\s+-march=core2\\s+(.*?)$", "\\1 \\2", 1L)
          )
        ),
        
        "src/cwb/config/platform/darwin-universal" = list(
          # stable r1069 - r1690
          replace = list("^(CFLAGS\\s*=.*?)\\s+-march=core2\\s+(.*?)$", "\\1 \\2", 1L)
        ),
        
        "src/cwb/config/platform/darwin-brew" = list(
          # stable r1069-r1690
          replace = list("^(CFLAGS\\s*=.*?)\\s+-march=native\\s+(.*?)$", "\\1 \\2", 1L)
        ),
        
        "src/cwb/config/platform/linux-opteron" = list(
          # stable r1069-r1690
          replace = list("^(CFLAGS\\s*=.*?)\\s+-march=opteron\\s+(.*?)$", "\\1 \\2", 1L)
        ),
        
        "src/cwb/config/platform/unix" = list(
          # stable r1069-r1690
          replace = list("^AR\\s+=\\s+ar\\scq\\s*$", "AR = ar", 1L)
        ),
        
        "src/cwb/config/platform/linux" = list(
          # stable r1069-r1690
          insert_before = list("##\\sCPU\\sarchitecture", c("## require position-independent code", "CFLAGS += -fPIC", ""), 1L)
        ),
        
        "src/cwb/config.mk" = c(
          # stable r1069-r1690
          list(
            replace = list("^PLATFORM=darwin-brew\\s*$", "PLATFORM=darwin-64", 1L)
          ),
          if (revision >= 1690) list(
            # Un-comment FULL_MESSAGES, this will re-activate full output
            replace = list("^\\s*#\\s+FULL_MESSAGES\\s+=\\s+1\\s*$", "FULL_MESSAGES = 1", 1L)
          )
        ),
        
        "src/_cl.h" = list(
          replace = list("^\\s*(typedef\\sstruct\\sClAutoString\\s\\*ClAutoString;)\\s*$", "/* \\1 */", 1L),
          insert_before = list("\\s*/\\*\\sThe\\sactual\\scode\\sof\\sthe\\sheader\\sfile\\sbegins\\shere\\.\\s\\*/\\s*$", c("char* cl_get_version();", ""), 1L)
        )
      )
    },
    
    make_utils_header = function(){
      message("* create header file utils/utils.h ... ", appendLF = FALSE)
      utils <- c("cwb-encode.c", "cwb-compress-rdx.c", "cwb-huffcode.c", "cwb-makeall.c")
      
      h_li <- lapply(
        utils,
        function(util){
          util_full_name <- file.path(self$repodir, "src", "cwb", "utils", util)
          cmd <- sprintf("%s -h %s", self$makeheaders, util_full_name)
          defs <- unique(system(cmd, intern = TRUE))
          fns <- grep("\\)\\s*;\\s*$", defs, value = TRUE)
          c(sprintf("/* functions defined in %s */", util), "", fns, "", "")
        }
      )
      
      h <- c(
        "/* Do not edit by hand! This file has been automatically generated */",
        "",
        "extern Corpus *corpus;",
        "typedef struct _SAttEncoder SAttEncoder;",
        "", "",
        unlist(h_li)
      )
      
      writeLines(h, con = file.path(self$repodir, "src", "cwb", "utils", "utils.h"))
      message("OK")
      invisible(h)
    },
    
    patch_file = function(file){
      
      fname_full <- fs::path(self$repodir, file)
      if (!file.exists(fname_full)){
        return(FALSE)
      } else if (length(self$file_patches[[file]]) == 0L){
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
      src_files <- list.files(path = path(self$repodir, "src"), full.names = TRUE, recursive = TRUE)
      missing_files_with_actions <- files_with_actions[!files_with_actions %in% src_files]
      
      if (length(missing_files_with_actions) > 0L){
        message("* actions defined for the following files, but file not present:")
        for (f in missing_files_with_actions) message("- ", f)
      }
      
      results <- sapply(names(self$file_patches), self$patch_file)
      if (self$verbose) message("* number of files successfully patched: ", table(results)[["TRUE"]])
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
      self$create_copy()
      
      self$diff_global_replacements <- self$get_difflist()
      git2r::add(repo = self$repodir, path = "src/cwb/*")
      commit(self$repository, message = "global replacements applied")
      
      self$patch_files()
      
      self$make_utils_header()
      self$create_globalvars_file()
      self$diff_file_patches <- self$get_difflist()

      if (self$verbose) message("* add new and altered files to HEAD")
      git2r::add(repo = self$repodir, path = "src/cwb/*")
      git2r::add(repo = self$repodir, path = "src/_cl.h")
      git2r::add(repo = self$repodir, path = "src/_eval.h")
      git2r::add(repo = self$repodir, path = "src/_globalvars.h")
      if (self$verbose) message("* commit: ", self$repodir)
      commit(self$repository, message = "CWB patched")
      self$patch_commit <- last_commit(self$repository)

      if (self$verbose) message("* return to branch of departure: ", self$branch_of_departure)
      checkout(repo = self$repodir, branch = self$branch_of_departure)
      invisible(self)
    }
    
  )
)


