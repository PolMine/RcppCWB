svn_get_revision <- function(svn_dir){
  old_wd <- setwd(svn_dir)
  rev <- as.integer(system("svn info --show-item revision", intern = TRUE))
  setwd(old_wd)
  rev
}


#' @return An `integer` value with the active SVN revision number.
svn_set_revision <- function(svn_dir, revision){
  old_wd <- setwd(svn_dir)
  rev <- svn_get_revision(svn_dir = svn_dir)
  if (rev != as.integer(revision)){
    system(
      sprintf("svn up -r %d", revision),
      intern = TRUE
    )
    rev <- svn_get_revision(svn_dir = svn_dir)
  }
  setwd(old_wd)
  rev
}
