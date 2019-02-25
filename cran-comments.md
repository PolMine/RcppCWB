## General remarks

This version of RcppCWB has finally seen the modifications required to make it installable on Solaris.

A remaining requirement for Solaris is the use of GNU make. The reason is an include statement in the Makefiles that is hard to omit. The GNU make requirement is stated in the 'SystemRequirements' field of the DESCRIPTION file. In our conversation, Brian Ripley confirmed that this would be ok.

On a Solaris VMWare image (prepared by Jeroen Ooms, R 3.2.0), this works with the current RcppCWB version:
MAKE=gmake R CMD INSTALL RcppCWB_0.2.8.tar.gz
MAKE=gmake R CMD check RcppCWB_0.2.8.tar.gz

Brian Ripley alerted me that there has been a recent ERROR to build RcppCWB on Fedora with the clang compiler. As explained to me by Tomas Kalibera, this will be a problem throughout when staged installs are introduced for all build systems.

The problem causing the issue was that the configure script sets a hardcoded path to the package (in so-called 'registry' files describing sample data/corpora in the package) during the installation process, which will not work with staged installs. The solution is that the necessity for hardcoded paths is omitted by working with temporary 'registry'-files in the temporary directory provided by 'tempdir()'.

Finally, Brian Ripley alerted me that the '-march' compiler option is not compatible with requirements for portability. I have removed the option wherever it occurred.


## Test environments

* OS X (local install), R 3.5.1
* OX X (on travis-ci), R 3.5.2
* Solaris (VMWare virtual machine), R 3.2.0
* Ubuntu 14.04 (on travis-ci), R 3.5.2
* Ubuntu 14.04 (project server), R 3.4.3
* win-builder (devel and release), R 3.5.2
* Windows AppVeyorR, 3.5.2 Patched


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used. On the Solaris 
virtual machine, there was a WARNING on the UTF-8 encoding used.

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib which are included. I will try to re-compile the cross-compiled static libraris using the flag '-Os' to reduce the file size. I will be glad for your advice, if compiler optimization is more important than keeping the package below 5Mb.


## Downstream dependencies

I have also checked RcppCWB v0.2.8 against the polmineR package, the existing reverse dependency at this time. I did not see WARNINGs, or NOTEs, or ERRORs.
