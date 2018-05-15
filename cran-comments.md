## General remarks

This (v0.2.3) is a submission following the previous version (v0.2.2) quickly to solve a potential issue when installing package binaries from CRAN.

The package includes sample data in the package subdirectory ./inst/extdata/cwb/. Upon installation, the configuration files 'configure' and 'configure.win' ensure that paths in the so-called registry files are reset to point to data files within the package. However, the configure stripts are not used when a binary package is installed (from CRAN), and paths are not set correctly. To make sure that users who install a binary package have access to the sample data, .onLoad() checks whether paths are set correctly, and adjusts paths, if necessary.

The new version introduces some checks to avoid crashes. As a consequence, three tests included in the polmineR package (maintained by myself) will fail. I have already prepared a release of the polmineR package that fixes the issue, and will submit it as soon as RcppCWB is admitted.


## Test environments

* local OS X install, R 3.5.0
* Ubuntu 14.04 (on travis-ci), R 3.5.0
* Ubuntu 14.04 (project server), R 3.4.3
* Debian 9.1 (virtual machine), R 3.4.3
* win-builder (devel and release), R. 3.5.0
* Windows/AppVeyor, R 3.5.0


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. 

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib. A potential way to reduce package size may be to find a workaround on the glib dependency in Windows, but I would have to figure out in an upcoming version whether this can be realized.


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.

On the issue with polmineR, see the explanation in the general remarks.
