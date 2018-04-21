## General remarks

This RcppCWB version (v0.2.1) exposes more CWB functionality than the previous version on CRAN (0.1.7). For doing this, I have used and modified code included in the 'rcqp' package authored by Bernard Desgraupes and Sylvain Loiseau (GPL license). To acknowledge the work of both colleagues, both are mentioned as authors in the DESCRIPTION file.

Note that Bernard Desgraupes and Sylvain Loiseau have not written code actively for RcppCWB. However, 'Writing R Extensions' did not give me guidance whether I should mention Bernard Desgraupes and Sylvain Loiseau as contributors or authors, so I opt for the option that is going further.

The previous version of the package did not pass tests on macOS. The cause is that glib (available on CRAN Ubuntu, Fedora and Debian systems) is not installed on your macOS machine. If glib was installed  (e.g. using homebrew, "brew install glib"), RcppCWB should pass macOS tests, too. I would be very grateful, if you could make glib available on CRAN macOS machines. 

Compilation does not yet work on Solaris. I am working to establish a test environment 
for Solaris. I will attempt to get RcppCWB work on Solaris in the next upcoming version.


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

