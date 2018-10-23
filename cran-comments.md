## General remarks

This version (v0.2.6) expands the exposed functionality of the Corpus Workbench.

Compilation of RcppCWB fails for macOS because 'Glib' is not present on CRAN build machines for macOS. It would be great, if you could install 'Glib' on your macOS build machine, which might be needed by other packages as well (e.g. 'Rpoppler'). On my system, using Homebrow for installing Glib works nicely (brew install glib).

Making RcppCWB compile on Solaris is still on my list of to dos!


## Test environments

* local OS X install, R 3.5.0
* Ubuntu 14.04 (on travis-ci), R 3.5.0
* Ubuntu 14.04 (project server), R 3.4.3
* win-builder (devel and release), R 3.5.0
* Windows AppVeyorR, 3.5.0 Patched


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. 

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib which are included.


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(). I did not see WARNINGs, or NOTEs, but one ERROR, caused by the package 'polmineR' which I maintain myself. Accidentaly, there is a call to RStudio's `View()` when checking the package in interactive mode using 'devtools::revdep()'. This is an polmineR issue which I can fix as soon as RcppCWB v0.2.6 is on CRAN.
