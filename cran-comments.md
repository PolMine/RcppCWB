## General remarks

Previous versions of RcppCWB downloaded pre-compiled binaries of the corpus library,
including unix-alikes. Uwe Ligges alerted me that this would not be in line with GPL,
and potentially illegal. So this new version of RcppCWB includes the entire (GPL-licensed) 
source code of the  CWB.

The configure script generates the Makevars file; The Makevars file calls minimally
modified Makefiles in the subdirectories of the CWB (i.e. in src/cwb/cl) to generate
a static library of the corpus library first. Then the dynamic library is generated.
I followed section 1.2.1.3 Compiling in sub-directories of 'Writing R extensions'.
In addition, the configure script adjusts the information in the CWB configuration file
(config.mk) concerning the platform. As far as I can see on my test environments, this
works fine.

For Windows, the mechanism is slightly different: configure.win will call the R script 
tools/winlibs.R which will download libcl.a from my GitHub repository 
(https://github.com/PolMine/libcl). I tried to follow the approach of packages that rely
on the repositories at https://github.com/rwinlib.

The library has been cross-compiled for Windows on a Ubuntu machine. A documentation of 
the cross-compilation is explained in the file prep/crosscompilation.Rmd of the git 
repository, see: https://github.com/PolMine/RcppCWB/blob/master/prep/crosscompilation.Rmd

In earlier versions of the package, my documentation of cross-compilation was a vignette,
causing issues with cross-platrom portability. So no vignette any more. 

The primary purpose of developing the package at this stage is to have a backend for the
package 'polmineR' that works on Windows. Once the fundamental design is acceptable for
CRAN, I would improve the documentation, and expose more CWB functionality using Rcpp 
wrappers.


## Test environments

* local OS X install, R 3.4.3
* Ubuntu 14.04 (on travis-ci), R 3.4.2
* Ubuntu 14.04 (project server), R 3.4.3
* Debian 9.1 (virtual machine), R 3.4.3
* win-builder (devel and release)

## R CMD check results

There were no ERRORs or WARNINGs. 

There was 1 NOTE:

* checking CRAN incoming feasibility ... NOTE
Maintainer: ‘Andreas Blaette <andreas.blaette@uni-due.de>’

New submission

Package was archived on CRAN

CRAN repository db overrides:
  X-CRAN-Comment: Archived on 2018-01-19 as was very far from
    cross-platform code.

This new version is intended to make progress towards cross-platform portability.


## Downstream dependencies

I have also checked downstream dependencies using devtools::revdep(),
without seeing ERRORs, WARNINGs, or NOTEs.

