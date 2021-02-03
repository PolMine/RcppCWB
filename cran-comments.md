## General remarks

Update: This is the second submission. The logs for the Mac M1 reported an issue with header includes. I do thing that I solved the issue by consistently putting variables in curly braces or by quotating them where necessary. If another submission should be necessary, additional verbosity of the configure script will help me to locate the bug.


This release picks up an alert of Brian Ripley (January 26) that RcppCWB fails to compile on an M1 Mac, see: https://www.stats.ox.ac.uk/pub/bdr/M1mac/RcppCWB.log. Very precise analysis: "You are attempting to use files of the wrong architecture: that needs checking at the configure stage before you download some of them.  And you have passed the wrong architecture to the compiler ...."

Both issues are resolved with this RcppCWB version. Please note that part of the work to get glib-2.0 for the correct architecture is done be the R file ./tools/maclibs.R called by the configure script.

There are two further changes of the configure script to improve things:

- Apart from making RcppCWB compatible with M1, I use `pcre-config` now to get the location of PCRE headers more reliably.

- An additional check using `pcretest` (if available) issues a warning if PCRE has not been compiled with the Unicode support required.




## Test environments

* R-hub (Fedora Linux, R-devel, clang, gfortran)
* win-builder (R-devel [2021-01-31 r79912] and R 4.0.3)
* macOS Catalina (local install), R 4.0.2
* macOS Big Sur 11.0 (MacStadium, Mac mini with M1 chip) with R-devel
* Ubuntu 14.04 (project server), R 3.6.3


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS / Windows environments I used. A NOTE concerning package size I used to see on Windows machines does not occur .


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
