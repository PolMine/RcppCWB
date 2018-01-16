.onLoad <- function (libname, pkgname) {
  # adjust dataDir in the registry file, if it has not yet been set
  # applies only Windows, on Linux/Mac the directory may be 
  # write protected
  if (.Platform$OS.type == "windows"){
    reutersRegistry <- file.path(libname, pkgname, "extdata", "cwb", "registry", "reuters")
    reutersDataDir <- file.path(libname, pkgname, "extdata", "cwb", "indexed_corpora", "reuters")
    reutersDataDir <- gsub("^[A-Z]?:?(.*)$", "\\1", reutersDataDir) # volume needs to be removed
    if (gsub("^HOME\\s+(.*?)$", "\\1", readLines(reutersRegistry)[10]) != reutersDataDir){
      .adjustRegistry(registryFile = reutersRegistry, dataDir = reutersDataDir)
    }
  }
}
