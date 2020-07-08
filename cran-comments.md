## General remarks

This is an immmediate follow-up to issues that occurred after the release
of RcppCWB v0.2.10 (2020-06-25). It tried to remove (most) remaining bashisms 
from the configure and the cleanup scripts, but unperfectly so.

Brian Ripley alerted me in a June 26 mail that a couple of issues are not
in line with CRAN policies and expectations and need to be fixed until
July 10. I do understand all issues raised and I have worked hard to 
improve things. My apologies that parts of my work failed to meet
CRAN standards. I am very grateful for the scrutiny you investd in 
inspecting the RcppCWB package and pinpointing shortcomings.

These are the major changes (and responses to the issues raised by 
Brian Ripley).

0) OSes covered by the the package

All OSes checked by CRAN by Default (Windows, macOS, Linux distros Debian,
Fedora, Ubuntu) are dealt with explicitly by the configure script that
has been reworked thorughly. If the OS is unknown, there is a warning 
message, and a Unix configuration is used as a fallback option.


1) You force -fcommon, which is deprecated for GCC

And it has been placed wrongly, as pointed out by Brian Ripley. I needed
the flag as a murky workaround because the structure of includes is 
fairly complicated. Usage of the -fcommon flag has been dropped.
It is not necessary by using the "extern" whereever necessary. A 
new header file has been written. 


2) Bashisms: -e not portable / avoid using $(cmd)

I had used the checkbashisms script to detect bashisms and I was not 
aware that checks are not comprehensive. My apologies for having missed what
"Writing R extensions" says. -e has been replace by -f which works for
Bourne shell scripts. Backticks are now use throughout to replace $(cmd).


4) Check for pcre and glib-2.0

RcppCWB is a wrapper the Corpus Workbench (CWB) is based on PCRE1. My 
grasp is that pcre has superseded but it is still widely used today. 
The situation will be different two or three years from now. So I am in 
touch with Stefan Evert, the main developer of the CWB to modernize 
the CWB in time, moving from PCRE1 to PCRE2.

In SystemRequirements, the statement is now pcre (>= 7 < 10).

The configure script now checks or the presence of ncurses, pcre and 
glib-2.0 (using pkg-config / pcre-config) and will issue an error 
message with installation instructions if a dependency is missing. 

You also do not check for 'glib-2' (sic).

5) Woes on macOS

There was a build error resulting from the previous changes of the 
configure script that had unintended side effects. Solved.

6) compilation warnings

RcppCWB is a wrapper for the CWB and I was too hesitant to intervene
in the original CWB code. Now, I do not get compiler warnings on the 
test environments used.



## Test environments

* win-builder (R-devel and R-release), R 4.0.2
* Windows AppVeyorR, 4.0.2 Patched
* OS X (local install), R 4.0.2
* Ubuntu 14.04 (on travis-ci), R 4.0.0
* Fedora 32 (docker image), both clang and gcc compilers, R 3.6.3 
* Fedora 31 (R-hub), GCC, R-devel
* Ubuntu 14.04 (project server), R 3.6.3


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used.

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib which are included.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
