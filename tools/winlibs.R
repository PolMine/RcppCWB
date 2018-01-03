# Build against libxml2 from Rtools
if (TRUE) {
  dir.create("../inst/libs", showWarnings = FALSE)
  dir.create("../inst/libs/x64", showWarnings = FALSE)
  dir.create("../inst/libs/i386", showWarnings = FALSE)
 
  if (getRversion() < "3.3.0") setInternet2()
 
  
  ## i386 - files ----------------------

   download.file(
     "http://polmine.sowi.uni-due.de/public/dlls/i386/libcl.a",
     "../inst/libs/i386/libcl.a", quiet = TRUE,
    method = "curl"
   )
  download.file(
     "http://polmine.sowi.uni-due.de/public/dlls/i386/libglib-2.0.a", 
     "../inst/libs/i386/libglib-2.0.a", quiet =  TRUE,
     method = "curl"
    )
  download.file(
     "http://polmine.sowi.uni-due.de/public/dlls/i386/libpcre.a", 
    "../inst/libs/i386/libpcre.a", quiet = TRUE,
    method = "curl"
   )
  download.file(
    "http://polmine.sowi.uni-due.de/public/dlls/i386/libintl.a", 
    "../inst/libs/i386/libintl.a", quiet = TRUE,
    method = "curl"
  )
 
  #download.file( 
  # 
# "http://polmine.sowi.uni-due.de/public/dlls/i386/libwinpthread-1.dll",
  #  "../inst/libs/i386/libwinpthread-1.dll", quiet = TRUE,
  #  method = "curl"
  #)



  ## x64 - files -----------------------
   download.file(
    "http://polmine.sowi.uni-due.de/public/dlls/x86/libglib-2.0.a",
   "../inst/libs/x64/libglib-2.0.a", quiet = TRUE,
    method = "curl"
   )
  # download.file(
  #  "http://polmine.sowi.uni-due.de/public/dlls/x86/libiconv.a", 
  #  "../inst/libs/x64/libiconv.a", quiet = TRUE,
  #  method = "curl"
  # )
  download.file(
    "http://polmine.sowi.uni-due.de/public/dlls/x86/libintl.a",
    "../inst/libs/x64/libintl.a", quiet = TRUE,
    method = "curl"
  )
  download.file(
    "http://polmine.sowi.uni-due.de/public/dlls/x86/libpcre.a", 
    "../inst/libs/x64/libpcre.a", quiet = TRUE,
    method = "curl"
   )
  # download.file(
  #  "http://polmine.sowi.uni-due.de/public/dlls/x86/libpcreposix.a", 
  #   "../inst/libs/x64/libpcreposix.a", quiet = TRUE,
  #   method = "curl"
 # )
 # download.file(   
 # "http://polmine.sowi.uni-due.de/public/dlls/x86/libwinpthread-1.dll", 
 #   "../inst/libs/x64/libwinpthread-1.dll", quiet = FALSE,
 #   method = "curl"
 # )
  download.file(
    "http://polmine.sowi.uni-due.de/public/dlls/x86/libcl.a", 
    "../inst/libs/x64/libcl.a", quiet = TRUE, 
    method = "curl"
  )
}

