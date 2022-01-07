library(git2r)
library(fs)
library(magrittr)
library(R6)


source("~/Lab/github/RcppCWB/prep/autopatch/R/PatchEngine.R")

data_files <- list.files("~/Lab/github/RcppCWB/prep/autopatch/data", full.names = TRUE)
for (fname in data_files) source(fname)

PE <- lapply(
  c(1069, 1071),
  function(revision){
    PatchEngine$new(
      cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
      repodir = "~/Lab/github/RcppCWB",
      revision = revision, # see explanation below
      global_replacements = global_replacements,
      file_patches = file_patches
    )$patch_all()
    
  }
)

names(P_1069$diff_file_patches) %in% names(P_1071$diff_file_patches)

sapply(
  names(P_1069$diff_file_patches),
  function(x) identical(P_1069$diff_file_patches[[x]], P_1071$diff_file_patches[[x]])
)