## General remarks

The RcppCWB package is a wrapper for the Corpus Workbench (CWB). Previous versions
included the C code of an increasingly outdated version of CWB (CWB v3.4.14).
This version re-aligns RcppCWB with upstream CWB development (CWB v3.4.44).

Previous message: This version (v0.4.4) solves issues with setting and re-setting paths. I am confident that this release will solve an issue with the 'cwbtools' package (r-devel-windows-x86_64-new-UL and r-devel-windows-x86_64-new-TK). 


## Test environments

* Standard checks with R-hub
* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* local macOS R 4.1.1


## R CMD check results

I see a NOTE concerning package size on CRAN machines (5.2 MB on r-release-macos-x86_64). I hope this is still tolerable. 


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
