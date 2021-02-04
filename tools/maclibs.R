# If the glib-2.0 static library is not present, the configure script calls
# this script to download if from a GitHub repository 'libglib'. Inspired by
# the rwinlib repositories (https://github.com/rwinlib), tje repo offers static
# libraries that are not present locally. This is particularly important for
# building RcppCWB binaries on CRAN Macs which do not have glib-2.0 installed.
#
# Relevant information on libglib-2.0 is derived from the pkgconfig file that
# needs to be adapted to supply correct information.

instdir <- commandArgs()[6]
target_tarball <- file.path(instdir, "libglib.tar.gz")
target_dir <- file.path(instdir, "macos")
  
if (getRversion() < "3.3.0") setInternet2()
  
download.file(
  "https://github.com/PolMine/libglib/archive/master.tar.gz",
  target_tarball, method = "libcurl", quiet = FALSE
)
  
dir.create(target_dir, showWarnings = FALSE)
  
untar(target_tarball, exdir = target_dir)
unlink(target_tarball)
  
pkgconfig_file <- file.path(target_dir, "libglib-master/pkgconfig/glib-2.0.pc")
pc <- readLines(pkgconfig_file)
pc[1] <- sprintf("prefix=%s/macos/libglib-master", getwd())
pc[3] <- sprintf("libdir=${exec_prefix}/lib/%s", Sys.info()[["machine"]])

# The pkgconfig file for glib-2.0 makes a strong assumption that the gettext library
# will be /usr/local/opt/gettext/lib. This is not necessarily so and gettext
# functionality may be available elsewhere. So we remove this library from the
# flags to avoid warnings.
gettext_libdir <- "/usr/local/opt/gettext/lib"
if (!dir.exists(gettext_libdir)){
  libs_line <- grep("^Libs:\\s+", pc)
  pc[libs_line] <- gsub(sprintf("-L%s", gettext_libdir), "", pc[libs_line])
}

writeLines(text = pc, con = pkgconfig_file)

