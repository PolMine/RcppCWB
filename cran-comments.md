## General remarks

This version (v0.2.5) is a maintanence release. It integrates a pull request be Jeroen Ooms to include libiconv in configure.win, so that RcppCWB will be compatible with an upcoming version of Rtools.

In addition, there is only a minor change to make handling registry files more robust. 

Binaries of the package are not available at CRAN for macOS. Compilation fails in this case, because 'Glib is not present on CRAN build machines for macOS. It would be great for RcppCWB, if you could install Glib, which might be needed by other packages as well (such as Rpoppler). On my system, using Homebrow for installing Glib works nicely (brew install glib).

Making RcppCWB compile on Solaris is still on my list of to dos!


## Test environments

* local OS X install, R 3.5.0
* Ubuntu 14.04 (on travis-ci), R 3.5.0
* Ubuntu 14.04 (project server), R 3.4.3
* win-builder (devel and release), R 3.5.0
* Windows AppVeyorR, 3.5.0 Patched


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. 

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib.


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.
