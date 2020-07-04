## General remarks

0) The case in configure only covers 3 OSes, a violation of the policy



1) You force -fcommon, which is deprecated for GCC and not portable so
should only be used for a version of GCC which supports it.  It is a
compiler and not a CPP flag.

2) As the manual told you, test -e is not portable.

3) On Solaris

./configure: syntax error at line 12: `CC_R=$' unexpected
ERROR: configuration failed for package ‘RcppCWB’
where the manual warned that $(cmd) is a bashism and to use backticks.

4) You have

SystemRequirements: GNU make, pcre (>= 7), GLib (>= 2.0.0).

but mean PCRE1 not PCRE2 as the latter does not suffice.  So it needs
'(>= 7 < 10)'.  And this does not check for it in configure (as the
manual requires), just fails to find pcre.h.  (I no longer had PCRE1
installed on macOS: nothing else needs it.)

You also do not check for 'glib-2' (sic).

5) See the results page for woes on macOS.

6) There are many compilation warnings, including:

attributes.c: In function ‘component_full_name’:
macros.h:59:22: warning: the address of ‘rname’ will always evaluate as
‘true’ [-Waddress]

eval.c:1045:17: warning: result of comparison against a string literal
is unspecified (use an explicit string comparison function instead)
[-Wstring-compare]





## Test environments

* win-builder (R-devel and R-release), R 4.0.2
* Windows AppVeyorR, 4.0.2 Patched
* OS X (local install), R 4.0.0
* Ubuntu 14.04 (on travis-ci), R 4.0.0
* Fedora 32 (docker image), both clang and gcc compilers, R 3.6.3 
* Fedora 31 (R-hub), GCC, R-devel
* Ubuntu 14.04 (project server), R 3.6.3


## R CMD check results

There were no ERRORs, WARNINGs or NOTEs on the Linux / macOS environments I used.

On Windows, there is a NOTE concerning package size: "installed size is  5.5Mb | sub-directories of 1Mb or more: libs 5.2Mb". This results from the dependency on pcre, and glib which are included. I will try to re-compile the cross-compiled static libraris using the flag '-Os' to reduce the file size. I will be glad for your advice, if compiler optimization is more important than keeping the package below 5Mb.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
