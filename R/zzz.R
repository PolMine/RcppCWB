.onLoad <- function (libname, pkgname) {
  if (.Platform$OS.type == "windows"){
    registryFile <- file.path(libname, pkgname, "inst", "extdata", "cwb", "registry", "reuters")
    if (file.exists(registryFile)){
      registry <- readLines(registryFile)
      libDir <- gsub("^[A-Z]?:?(.*)$", "\\1", file.path(libname, pkgname))
      print(libname)
      print(libDir)
      homeDir <- file.path(libDir, "extdata", "cwb", "indexed_corpora", "reuters")
      print(homeDir)
      registry[10] <- paste("HOME", homeDir, sep = " ")
      writeLines(text = registry, con = registryFile)
    }
  }
}
