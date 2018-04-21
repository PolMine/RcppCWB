## General remarks

This RcppCWB version (v0.2.1) exposes more CWB functionality than the previous version
on CRAN (0.1.7). For doing this, I have used and modified code included in the 'rcqp' package
authored by Bernard Desgraupes and Sylvain Loiseau (GPL license). To acknowledge the 
work of both colleagues, both are mentioned as authors in the DESCRIPTION file.

Please note: They have not written code actively for RcppCWB. The 'Writing R Extensions'-guide
did not give me guidance whether I should mention Bernard Desgraupes and Sylvain Loiseau
as contributors or authors, so I opt for the option that is going further.

The previous version of the package did not pass tests on macOS. The reason
appears to be that glib (available on CRAN Ubuntu, Fedora and Debian systems)
is not installed on your macOS machine. If glib was installed  (e.g. "brew install glib""),
RcppCWB should pass macOS tests, too. May I kindly ask if you could make glib available on
your macOS machines, so package tests are passed on macOS.

On Debian, you may see a warning that an implicit declaration of the functions time()
and ctime() occurrs in 'ascii-print.c', 'sgml-print.c', 'html-print.c' and 'latex-print.c'.
It might be necessary to include the header file 'time.h' in a different manner, 
but I would refrain from mingling with the CWB C code here, because the print functions 
are not used.

Compilation does not yet work on Solaris. I am working to establish a test environment 
for Solaris. I will attempt to get RcppCWB work on Solaris in the next upcoming version.


## Test environments

* local OS X install, R 3.4.3
* Ubuntu 14.04 (on travis-ci), R 3.4.2
* Ubuntu 14.04 (project server), R 3.4.3
* Debian 9.1 (virtual machine), R 3.4.3
* win-builder (devel and release)


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs. 


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.

