#' @examples 
#' data_files <- list.files("~/Lab/github/RcppCWB/prep/autopatch/data", full.names = TRUE)
#' for (fname in data_files) source(fname)
#' 
#' P <- PatchEngine$new(
#'   cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
#'   repodir = "~/Lab/github/RcppCWB",
#'   revision = 1069,
#'   global_replacements = global_replacements,
#'   file_patches = file_patches
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
    global_replacements = NULL,
    file_patches = NULL,
    verbose = TRUE,
    
    initialize = function(cwb_dir_svn, revision, repodir, global_replacements, file_patches, verbose = TRUE){
      self$verbose <- verbose
      
      self$repodir <- path.expand(repodir)
      if (self$verbose) message("RcppCWB repository path: ", self$repodir)
      
      self$cwb_dir_svn <- path.expand(cwb_dir_svn)
      if (self$verbose) message("SVN directory with CWB code: ", self$cwb_dir_svn)
      
      self$revision <- as.integer(revision)
      if (self$revision) message("SVN revision to use: ", self$revision)
      
      
      self$branch_of_departure <- self$get_branch_of_departure()
      if (self$verbose) message("The branch of departure is: ", self$branch_of_departure)
      
      self$repository <- repository(repodir)
      self$last_commit <- last_commit(repo = repodir)
      self$global_replacements <- global_replacements
      self$file_patches <- file_patches 
      
      invisible(self)
    },
    
    get_branch_of_departure = function(){
      branches(repodir)[sapply(branches(self$repodir), is_head)][[1]][["name"]]
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
      if (self$verbose) message("Add newly created files to CWB code (overwriting existing files ...")
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
    
    delete_line_before = function(code, action){
      which_position <- if (length(action) > 1L) action[[2]] else 1L
      times <- if (length(action) == 3L) action[[3]] else 1L
      
      message(
        sprintf("action: delete_line_before | regex: %s | match: %d | lines: %d ... ", action[[1]], which_position, times),
        appendLF = FALSE
      )
      
      position <- grep(pattern = action[[1]], code)[which_position]
      
      if (!is.na(position)){
        code <- code[-(position - 1L:times)]
        message("OK")
      } else {
        message("FAIL - no match")
      }
      code
    },
    
    insert_before = function(code, action){
      which_position <- if (length(action) > 2L) action[[3]] else 1
      
      message(
        sprintf(
          "action: insert_before | regex: %s | match: %d | insertion: %s ... ", action[[1]], which_position, paste(action[[2]], collapse = "///")
        ),
        appendLF = FALSE
      )
      
      position <- grep(pattern = action[[1]], code)[which_position]
      if (!is.na(position)){
        code <- c(
          code[1L:(position - 1L)],
          action[[2]],
          code[position:length(code)]
        )
        message("OK")
      } else {
        message("FAIL - no matches for regex")
      }
      code
    },
    
    insert_after = function(code, action){
      which_position <- if (length(action) > 2L) action[[3]] else 1L
      
      message(
        sprintf("action: insert_after | regex: %s | match: %d | insertion: %s ... ", action[[1]], which_position, paste(action[[2]], collapse = "///")),
        appendLF = FALSE
      )
      
      position <- grep(pattern = action[[1]], code)[which_position]
      if (!is.na(position)){
        code <- c(
          code[1L:position],
          action[[2]],
          code[(position + 1L):length(code)]
        )
        message("OK")
      } else {
        message("FAIL - no match")
      }
      code
    },
    
    replace = function(code, action){
      message(
        sprintf("action: replace | regex: %s | match: %d | replacement: %s ... ", action[[1]], action[[3]], paste(action[[2]], collapse = "///")),
        appendLF = FALSE
      )
      position <- grep(pattern = action[[1]], code)[ action[[3]] ]
      if (!is.na(position)){
        code[position] <- gsub(action[1], action[2], code[position])
        message("OK")
      } else {
        message("FAIL - no match")
      }
      code
    },
    
    remove_lines = function(code, action){
      message(
        sprintf("action: remove_lines | regex: %s | match: %d ... ", action[[1]], action[[2]]),
        appendLF = FALSE
      )
      
      position <- grep(pattern = action[[1]], code)[ action[[2]] ]
      if (!is.na(position)){
        code <- code[-position]
        message("OK")
      } else {
        message("FAIL - no match")
      }
      code
    },
    
    extern = function(code, action){
      for (ext in action){
        matches <- which(startsWith(code, ext))
        if (length(matches) > 0L){
          for (position in matches){
            code[position] <- paste("extern", code[position], sep = " ")
          }
          message(sprintf("action: extern | var to extern: %s | n_matches: %d", ext, length(matches)))
        } else {
          message("FAIL - no matches for var: ", ext)
        }
      }
      code
    },
    
    patch_file = function(file){
      
      if (self$verbose) message("... patching file: ", file, appendLF = FALSE)
      
      actions <- self$file_patches[[file]]
      fname_full <- fs::path(self$repodir, file)
      if (!file.exists(file)){
        message(sprintf("FAIL (file does not exist)", file))
        return(FALSE)
      } else {
        code <- readLines(file)
        for (i in 1L:length(actions)){
          code <- self[[ names(actions)[i] ]](code = code, action = actions[[i]])
        }
        writeLines(code, file)
        message("OK")
        return(TRUE)
      }
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
      self$run_bison_flex()
      self$rename_files()
      self$copy_files()
      self$create_dummy_depend_files()
      self$replace_globally()
      for (fname in names(self$file_patches)){
        message("File: ", fname)
        self$patch_file(file = fname)
      }
      
      if (self$verbose) message("Add new and altered files to HEAD in repo: ", self$repodir)
      git2r::add(repo = self$repodir, path = "src/cwb/*")
      if (self$verbose) message("Commit: ", self$repodir)
      commit(self$repository, message = "CWB patched")
      self$patch_commit <- last_commit(repo)
      if (self$verbose) message("Story to be told")
      
      if (self$verbose) message("Return to branch of departure: ", self$branch_of_departure)
      checkout(repo = self$repodir, branch = self$branch_of_departure)
      invisible(self)
    }
    
  )
)


