.onLoad <-function (lib, pkg) {
  if (Sys.getenv("CORPUS_REGISTRY") == ""){
    Sys.setenv("CORPUS_REGISTRY" = system.file(
      package = pkg, "extdata", "cwb", "registry",
      lib.loc = lib
      )
      )
  }
  initialize_cqp()
}