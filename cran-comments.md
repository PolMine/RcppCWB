## General remarks

The previous version of the package did not pass tests on macOS. The reason
appears to be that glib (apparantly available on Ubuntu, Fedora and Debian)
is missing on your macOS machine. If glib was installed in macOS 
(e.g. "brew install glib""), RcppCWB may pass macOS tests, too. It would be
great if you could make glib available on your machines.

This RcppCWB version (v0.2.0) exposes further CWB functionality than the previous version
(0.1.7). A second part of the Corpus Workbench, the Corpus Query Processor (CQP)
is compiled as a static library. Rcpp wrappers expose the functionality.

For doing this, I rely on - modified - code and approaches of the rcqp package
authored by Bernard Desgraupes and Sylvain Loiseau (GPL license). The rcqp package 
has been archived on 2018-03-06 for policy violation.  To acknowledge the original 
work of Bernard Desgraupes and Sylvain Loiseau, both are mentioned as authores 
in the DESCRIPTION file.


I am working to establish a test environment for Solaris. I will attempt to get 
RcppCWB work on Solaris in the next upcoming version.


## Test environments

* local OS X install, R 3.4.3
* Ubuntu 14.04 (on travis-ci), R 3.4.2
* Ubuntu 14.04 (project server), R 3.4.3
* Debian 9.1 (virtual machine), R 3.4.3
* win-builder (devel and release)


## R CMD check results

There were no ERRORs, WARNINGs of NOTEs. 


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.

