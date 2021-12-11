## General remarks

This is an immediate follow-up to my first submission to fix an issue that has emerged with the 'cwbtools' dependency.

Previous message: This version (v0.4.4) solves issues with setting and re-setting paths. I am confident that this release will solve an issue with the 'cwbtools' package (r-devel-windows-x86_64-new-UL and r-devel-windows-x86_64-new-TK). 


## Test environments

* Standard checks with R-hub
* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* local macOS R 4.1.1


## R CMD check results

I see a NOTE concerning package size on CRAN machines (5.2 MB on r-release-macos-x86_64). I hope this is still tolerable. 


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
