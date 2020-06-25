## General remarks

This is a very quick follow up to an earlier submission of RcppCWB today. 
Among others, the earlier submission should remove bashisms from
the configure and the cleanup script.

The earlier version did not take into accout sufficiently that 'echo' works 
differently across systems if bash is not used. The are errors I see in the 
check results on Fedora and Solaris. This version of RcppCWB 
uses 'printf' rather than 'echo' in the shell scripts. I do not see errors
in the checks I could run.


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
