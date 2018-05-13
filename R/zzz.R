.write_registry <- function(text, con) writeLines(text = text, con = con, sep = "\n")

.onAttach <- function (libname, pkgname) {
  pkg_cwb_dir <- file.path(libname, pkgname, "extdata", "cwb")
  pkg_registry_dir <- file.path(pkg_cwb_dir, "registry")
  pkg_indexed_corpora_dir <- file.path(pkg_cwb_dir, "indexed_corpora")
  for (corpus in list.files(pkg_registry_dir)) {
    registry_file <- file.path(pkg_registry_dir, corpus)
    registry <- readLines(registry_file)
    home_line_no <- grep("^HOME", registry)
    info_line_no <- grep("^INFO", registry)
    registry_home_dir <- gsub("^HOME\\s+\"*(.*?)\"*\\s*$", "\\1", registry[home_line_no])
    registry_info_file <- gsub("^INFO\\s+\"*(.*?)\"*\\s*$", "\\1", registry[info_line_no])
    pkg_home_dir <- file.path(pkg_indexed_corpora_dir, corpus)
    if (!identical(x = registry_home_dir, y = pkg_home_dir)) {
      packageStartupMessage(sprintf("[Adjusting data directory in registry for corpus '%s' in package '%s']", corpus, pkgname))
      info_file_new <- file.path(pkg_home_dir, basename(registry_info_file), fsep = "/")
      if (.Platform$OS.type == "windows") {
        registry[home_line_no] <- sprintf("HOME \"%s\"", pkg_home_dir)
        registry[info_line_no] <- sprintf("INFO \"%s\"", info_file_new)
      } else {
        if (grepl("\\s+", pkg_home_dir)) {
          registry[grep("^HOME", registry)] <- sprintf("HOME \"%s\"",  pkg_home_dir)
          registry[info_line_no] <- sprintf("INFO \"%s\"", info_file_new)
        } else {
          registry[grep("^HOME", registry)] <- sprintf("HOME %s", pkg_home_dir)
          registry[info_line_no] <- sprintf("INFO %s", info_file_new)
        }
      }
      if (file.access(registry_file, mode = 2) == -1){
        warning(sprintf("Not sufficient permissions to modify registry file %s", registry_file),
                " which would be necessary to have access to sample corpora in package. ",
                "Consider loading package with admin rights one."
        )
      }
      .write_registry(text = registry, con = registry_file)
    }
  }
  invisible(NULL)
}