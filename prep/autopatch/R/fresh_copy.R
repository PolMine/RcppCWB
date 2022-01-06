library(git2r)

#' @importFrom git2r branches checkout branch_create branch_delete
#' @importFrom fs path rm_file
cwb_fresh_copy <- function(cwb_dir, repodir, source_branch){
  
  revision <- svn_get_revision(svn_dir = cwb_dir)
  branch <- sprintf("r%d", revision) # name of new branch
  
  git2r::checkout(repodir, branch = source_branch)
  if (branch %in% names(branches(repodir))){
    git2r::branch_delete(branch = branches(repodir)[[branch]])
  }
  git2r::branch_create(commit = last_commit(repo = repodir), name = branch)
  git2r::checkout(repodir, branch = branch) # switch to new branch
  
  # copy CWB files
  repo_cwb_dir <- path(repodir, "src", "cwb")
  cwb_files <- list.files(cwb_dir, recursive = TRUE, full.names = TRUE)
  file.copy(
    from = cwb_files,
    to = gsub(paste("^", cwb_dir, sep = ""), repo_cwb_dir, cwb_files),
    overwrite = TRUE
  )
  
  # Remove files that have been added (and that need to be added explicitly)
  # To restore the state of RcppCWB development, these files need to be re-added or generated later on
  
  cwb_files_svn_truncated <- gsub(cwb_dir, "", cwb_files)
  cwb_files_repo <- list.files(repo_cwb_dir, recursive = TRUE, full.names = TRUE)
  cwb_files_repo_truncated <- gsub(repo_cwb_dir, "", cwb_files_repo)
  
  added_files <-  cwb_files_repo_truncated[!cwb_files_repo_truncated %in% cwb_files_svn_truncated]
  added_files_full_path <- fs::path(repo_cwb_dir, added_files)
  added_files_existing <- added_files_full_path[file.exists(added_files_full_path)]
  git2r::rm_file(repo = repodir, path = added_files_existing)
  
  # add & commit
  git2r::add(repo = repodir, path = unname(unlist(git2r::status(repo = repodir))))
  git2r::commit(repodir, message = "CWB restored")
  list(removed_files = added_files_existing, branch = branch)
}
