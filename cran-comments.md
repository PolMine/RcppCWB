## General remarks

Apart from bug fixes, this version (v0.2.7) tries to address the current failure to build RcppCWB on your macOS build machines.

The problem appears to be that glib-2.0 is not installed on the system. As a remedy, the configure script now checks whether glib is present (using pkg-config). If glib is missing, a pre-compiled static library is downloaded from a GitHub repo (). If glib is present, 

This approach is a variation of the approach of packages that rely on https://github.com/rwinlib, already used for build in the Windows binaries.

Making RcppCWB compile on Solaris is still on my list of to dos. Because this is the larger group of users by far, making things smooth for macOS is the priority of this release.


## Test environments

* local OS X install, R 3.5.1
* Ubuntu 14.04 (on travis-ci), R 3.5.2
* Ubuntu 14.04 (project server), R 3.4.3
* win-builder (devel and release), R 3.5.2
* Windows AppVeyorR, 3.5.2 Patched


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. 

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib which are included.


## Downstream dependencies

I have also checked downstream dependencies using revdepcheck::revdep_check(). I did not see WARNINGs, or NOTEs, or ERRORs.
