.onAttach <- function (libname, pkgname) {
  pkg_dir <- file.path(libname, pkgname)
  paths_correct <- check_pkg_registry_files(pkg = pkg_dir, set = FALSE)
  if (!paths_correct) use_tmp_registry(pkg = pkg_dir)
}