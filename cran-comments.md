## General remarks

This is a maintenance release to address errors reported by CRAN Fedora machines. Brian Ripley sent me a respective alert and asked me to fix the issues by June 25. This version of RcppCWB solves the issues:

- It is now possible to compile everything using clang. The setting of CC in the Makeconf file or ~/.R/Makevars as returned by `R CMD config CC` is used throughout.

- The "multiple definition" errors newly thrown by GCC 10 are addressed by setting the '-fcommon' flag as a compiler option. In a future RcppCWB version I will adjust the the C code such that the root cause is addressed (adjustments of includes etc.). For the time being I think the solution is appropriate as I do not see any risk that conflicting symbol definitions might occurr. (The issue results from the multiple inclusion of the same header files by other header files further down the hierarchy.)


## Test environments

* win-builder (R-devel and R-release), R 4.0.2
* Windows AppVeyorR, 4.0.2 Patched
* OS X (local install), R 4.0.0
* Ubuntu 14.04 (on travis-ci), R 4.0.0
* Fedora 32 (docker image), both clang and gcc compilers, R 3.6.3 
* Fedora 31 (R-hub), GCC, R-devel
* Ubuntu 14.04 (project server), R 3.6.3


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used.

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib which are included. I will try to re-compile the cross-compiled static libraris using the flag '-Os' to reduce the file size. I will be glad for your advice, if compiler optimization is more important than keeping the package below 5Mb.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
