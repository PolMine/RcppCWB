.onLoad <- function (libname, pkgname) {
  # adjust dataDir in the registry file, if it has not yet been set
  # applies only Windows, on Linux/Mac the directory may be 
  # write protected
  cwbDir <- file.path(libname, pkgname, "extdata", "cwb")
  if (.Platform$OS.type == "windows"){
    cwbDir <- gsub("^[A-Z]?:?(.*)$", "\\1", cwbDir) # volume needs to be removed
    reutersRegistry <- file.path(cwbDir, "registry", "reuters")
    if (!file.exists(reutersRegistry)){
      # hiddenRegistryDir <- file.path(cwbDir, "registry", ".reuters")
      # file.rename(from = hiddenRegistryDir, to = reutersRegistry)
      reutersDataDir <- file.path(cwbDir, "indexed_corpora", "reuters")
      .adjustRegistry(registryFile = reutersRegistry, dataDir = reutersDataDir)
    }
  }
}
