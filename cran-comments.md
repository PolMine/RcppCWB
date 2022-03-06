## General remarks

After the previous major release that ensured RcppCWB compatibility with UCRT, 
this is a "standard" release with some new features, improvements and bug fixes.
A remaining compiler warning I had seen and reported has been fixed.

CRAN package check results report 'GNU make is a SystemRequirements'. Using GNU
make remains important for the portability of the C code. There would be a great
cost for dropping GNU make and I would not do it unless it is absolutely
necessary.

Installed package size is slightly larger than 5MB on macOS and Windows
release/oldrel. The size of the libs dir is the culprit here, so this is hard to
change.


## Test environments

* Standard checks with R-hub
* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* local macOS R 4.1.2 (arm64)
* R winbuilder (devel and release)


## R CMD check results

Temporarily I have seen a NOTE that the URL
https://doi.org/10.5281/zenodo.4501838 is not available. Seems to work again.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, polmineR). I did not see
WARNINGs, or NOTEs, or ERRORs.
