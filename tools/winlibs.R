if( !file.exists("../windows/libcl-v3.4.33/include/cl.h") ){
  
  if (getRversion() < "3.3.0") setInternet2()
  download.file("https://github.com/PolMine/libcl/archive/v3.4.33.zip", "v3.4.33.zip", quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  unzip("v3.4.33.zip", exdir = "../windows")
  unlink("v3.4.33.zip")
  
}
