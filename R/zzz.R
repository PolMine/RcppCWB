.onLoad <- function (libname, pkgname) {
  # adjust dataDir, if it has not yet been set
  cwbTmpDir <- file.path(libname, pkgname, "extdata", "cwb")
  if (.Platform$OS.type == "windows") cwbTmpDir <- gsub("^[A-Z]?:?(.*)$", "\\1", cwbTmpDir)
  
  reutersTmpRegistry <- file.path(cwbTmpDir, "registry", "reuters")
  reutersTmpDataDir <- file.path(cwbTmpDir, "indexed_corpora", "reuters")
  # checking whether the registry file exists is necessary to circumvent a 
  # devtools::document-problem
  .adjustRegistry(registryFile = reutersTmpRegistry, dataDir = reutersTmpDataDir)
}
