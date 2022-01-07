library(git2r)
library(fs)
library(magrittr)
library(R6)


source("~/Lab/github/RcppCWB/prep/PatchEngine.R")

PE <- list()

PE[["1069"]] <- PatchEngine$new(
      cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
      repodir = "~/Lab/github/RcppCWB",
      revision = 1069
)
PE[["1069"]]$patch_all()
    
update <- "1200"
PE[[update]] <- PatchEngine$new(
  cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
  repodir = "~/Lab/github/RcppCWB",
  revision = as.integer(update)

)
PE[[update]]$patch_all()

names(PE[[1]]$diff_file_patches) %in% names(PE[[2]]$diff_file_patches)

patch_success <- sapply(
  names(PE[[1]]$diff_file_patches),
  function(x) identical(PE[[1]]$diff_file_patches[[x]], PE[[update]]$diff_file_patches[[x]])
)
names(patch_success)[which(patch_success == FALSE)]

