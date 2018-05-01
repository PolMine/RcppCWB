## General remarks

This (v0.2.2) is a submission following the previous version (v0.2.1) less than two weeks to solve remaining issues that restrict cross-platform portability.

I assume this version will now pass tests on your macOS build machine. Version 0.2.1 of the package did not pass tests, the cause being that glib is not available on the CRAN macOS build machine. I realized that the glib dependency can be circumvented on unlix-like systems. 

Initially, package tests for v0.2.1 failed on the CRAN Linux build machine, because the bison parser was not found. After a few days, the package did pass tests, so my interpretation is that you might have installed bison. 

To omit difficulties for users, I added the files parsed by bison to the package, while keeping the original input files. Distributing the input source and the resulting C code is the recommendation I found here (section 'Distribution of packages using Bison'):  https://en.wikipedia.org/wiki/GNU_bison#Distribution_of_packages_using_Bison

Compilation does not yet work on Solaris. I am (still) working to establish a test environment 
for Solaris. Solving issues to pass tests on macOS has been my priority this time.


## Test environments

* local OS X install, R 3.4.3
* Ubuntu 14.04 (on travis-ci), R 3.4.2
* Ubuntu 14.04 (project server), R 3.4.3
* Debian 9.1 (virtual machine), R 3.4.3
* win-builder (devel and release)


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. 

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib, and I do not see a way how to minimize package size.


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.

