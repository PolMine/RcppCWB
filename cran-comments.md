## General remarks

This is a follow-up to the submission of RcppCWB v0.4.1. Checks on Fedora with GCC 11 resulted in a warning on a violation of the C++ One Definition Rule. This is fixed.

In the meantime, I have also been working hard on the UCRT issue. The importance of r-devel-windows-x86_64-gcc10-UCRT is perfectly clear to me. Tomas Kalibera has offered comprehensive and extremeley helpful instructions on this. Unfortunately, I cannot accomplish the fix before July 7. And it has been confirmed to me that it will not threaten the presence of RcppCWB at CRAN if I approach this fix in August/September. So I see this as the next step. I appreciate your patience and support ...


## Test environments

* R-hub (Oracle Solaris 10, x86, 32 bit, R release, Oracle Developer Studio 12.6)
* Solaris 10 on local VirtualBox
* Ubuntu 20.04 (R 4.1.0)
* Fedora with gcc11 (R-devel)
* win-builder (R 4.1.0 and R-devel)
* macOS Intel, R 4.1.0
* macOS arm64, R-devel


## R CMD check results

I see a NOTE concerning package size on CRAN machines (5.2 MB on r-release-macos-x86_64). I hope this is still tolerable. 

The latest check for r-release-macos-arm64 is on v0.3.2, not yet for v0.4.0, but I addressed the difficulty to compile RcppCWB on CRAN. It works nicely on my Mac mini.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
