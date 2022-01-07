library(git2r)
library(fs)
library(magrittr)
library(R6)

source("~/Lab/github/RcppCWB/prep/autopatch/R/PatchEngine.R")

data_files <- list.files("~/Lab/github/RcppCWB/prep/autopatch/data", full.names = TRUE)
for (fname in data_files) source(fname)

P_1069 <- PatchEngine$new(
  cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
  repodir = "~/Lab/github/RcppCWB",
  revision = 1069, # see explanation below
  global_replacements = global_replacements,
  file_patches = file_patches
)
P_1069$patch_all()


P_1071 <- PatchEngine$new(
  cwb_dir_svn = "~/Lab/tmp/cwb/trunk",
  repodir = "~/Lab/github/RcppCWB",
  revision = 1071, # see explanation below
  global_replacements = global_replacements,
  file_patches = file_patches
)
P_1071$patch_all()
