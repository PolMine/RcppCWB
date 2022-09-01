## General remarks

I realized ERRORS on the macOS and Windows build machines: Fixed as follows:

- For Windows, I add '-luuid' to PKG_FLAGS (necessary to meet new dependency of 
glib2).
- For macOS, the CC defined in Makeconf was not passed to the compilation of
the CWB source files. Changed the configure scripts accordingly.


Previous aspects I repeat:

- CRAN package check results report 'GNU make is a SystemRequirements'. Using GNU
make remains important for the portability of the C code. There would be a great
cost for dropping GNU make and I would not do it unless it is absolutely
necessary.

- Installed package size is slightly larger than 5MB on macOS and Windows
release/oldrel. The size of the libs dir is the culprit here, so this is hard to
change.


## Test environments

* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* R winbuilder (R 4.2 release and devel)
* local macOS, still R 4.1.3 (arm64)


## R CMD check results

Temporarily I have seen a NOTE that the URL
https://doi.org/10.5281/zenodo.4501838 is not available. Seems to work again.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, polmineR). I did not see
WARNINGs, or NOTEs, or ERRORs.
