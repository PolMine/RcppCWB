library(git2r)
library(fs)
library(magrittr)
library(R6)


source("~/Lab/github/RcppCWB/prep/autopatch/R/PatchEngine.R")

data_files <- list.files("~/Lab/github/RcppCWB/prep/autopatch/data", full.names = TRUE)
for (fname in data_files) source(fname)



PE <- list()

PE[["1069"]] <- PatchEngine$new(
      cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
      repodir = "~/Lab/github/RcppCWB",
      revision = 1069,
      global_replacements = global_replacements,
      file_patches = file_patches
)
PE[["1069"]]$patch_all()
    
update <- "1100"
PE[[update]] <- PatchEngine$new(
  cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
  repodir = "~/Lab/github/RcppCWB",
  revision = as.integer(update),
  global_replacements = global_replacements,
  file_patches = file_patches
)
PE[[update]]$patch_all()



names(PE[[1]]$diff_file_patches) %in% names(PE[[2]]$diff_file_patches)

sapply(
  names(PE[[1]]$diff_file_patches),
  function(x) identical(PE[[1]]$diff_file_patches[[x]], PE[[2]]$diff_file_patches[[x]])
)
