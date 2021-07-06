if( !file.exists("../windows/libcl-gcc11/include/cl.h") ){
  
  if (getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/PolMine/libcl/archive/gcc11.zip", "libcl.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("libcl.zip", exdir = "../windows")
  unlink("libcl.zip")
  
}
