version <- commandArgs(TRUE)

if ( !file.exists(sprintf("../windows/libcl-%s/include/cl.h", version))){
  
  if (getRversion() < "3.3.0") setInternet2()
  download.file(sprintf("https://github.com/PolMine/libcl/archive/v%s.zip", version), "lib.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("lib.zip", exdir = "../windows")
  unlink("lib.zip")
  
}
