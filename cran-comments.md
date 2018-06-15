## General remarks

This version (v0.2.4) addresses an issue raised by Kurt Hornik: A required path within a 'registry'-file within the package is not adjusted by the configure/configure.win when installing a binary package. The previous version did the adjustment during .onLoad() mechanism, changing a file in the package directory -- thus violating CRAN requirements. The new version of RcppCWB creates temporary registry files in the temporary session directory. Thanks to Kurt Hornik for suggesting this solution.

To avoid encoding issues, I updated the source code of the Corpus Workbench (CWB) included in the package to v3.5 BETA. The update implies that 'Glib' is now a dependency for compiling the package. 'Glib' had already been a dependency for cross-compiling the static libraries required for Windows binaries, so I do not think changing the DESCRIPTION is necessary. 'Glib' is already addressed.

However, I think I recall that 'Glib' is not present on all build systems, and I am afraid that compiling RcppCWB might fail on either Fedora or Debian. If that happens: I would be very happy, if you could install 'Glib', which might be needed by other packages as well (such as Rpoppler).

On macOS: brew install glib
Debian: apt-get install libglib2.0-dev
Fedora: yum install glib2-devel



## Test environments

* local OS X install, R 3.5.0
* Ubuntu 14.04 (on travis-ci), R 3.5.0
* Ubuntu 14.04 (project server), R 3.4.3
* win-builder (devel and release), R 3.5.0
* Windows on Amazon WorkSpaces, R 3.4.3


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. 

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib.


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.

On the issue with polmineR, see the explanation in the general remarks.
