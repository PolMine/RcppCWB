## General remarks

This is an immediate follow-up to the release of RcppCWB v0.4.0 on June 26. The primary purpose of this version (v0.4.1) is to fix an error at the linker stage on Solaris: Brian Ripley wrote me on June 26th asking me to fix the issue until July 7th. 

In addition, a usage of grep in the configure file not in line with POSIX specifiations has been replaced (see NEWS.md).

Brian Ripley also brought to my attention that the pcretest utility may not necessarily be available. Messages issued by the configure file now include an explicit information whether pcretest is available. 

I refrain from lengthy warnings if pcretest is absent, and instructions how pcretest might be installed. The most likely scenario that pcretest is missing is that on Ubuntu the libpcre3-dev package has been installed, which does not include pcretest. But it will include pcre with unicode properties, which is what I test for. The relevant scenario when unicode properties might be missing is when a user has built pcre himself. But then pcretest will also be available by default, and a warning is issued if pcre has been compiled without unicode properties.

A final change of the configure script is a somewhat relaxed warning that pkg-config cannot find ncurses. There are quite a few scenarios when ncurses libraries will be present and an overly alarmistic warning is confusing rather than helpful.

The importance of r-devel-windows-x86_64-gcc10-UCRT is perfectly clear to me. Tomas Kalibera has offered comprehensive and extremeley helpful instructions on this. Unfortunately, I cannot accomplish the fix before July 7. And it has been confirmed to me that it will not threaten the presence of RcppCWB at CRAN if I approach this fix in August/September. So I see this as the next step. I appreciate your patience and support ...


## Test environments

* R-hub (Oracle Solaris 10, x86, 32 bit, R release, Oracle Developer Studio 12.6)
* Solaris 10 on local VirtualBox
* Ubuntu 20.04 (R 4.1.0)
* Fedora with gcc10 (R 4.1.0)
* win-builder (R 4.1.0 and R-devel)
* macOS Intel, R 4.1.0
* macOS arm64, R-devel


## R CMD check results

I see a NOTE concerning package size on CRAN machines (5.2 MB on r-release-macos-x86_64). I hope this is still tolerable. 

The latest check for r-release-macos-arm64 is on v0.3.2, not yet for v0.4.0, but I addressed the difficulty to compile RcppCWB on CRAN. It works nicely on my Mac mini.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
