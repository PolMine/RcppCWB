## General remarks

Fixes -Wformat-security issue under r-devel caused by Rcpp.

Previous aspects I repeat:

- CRAN package check results report 'GNU make is a SystemRequirements'. Using
GNU make remains important for the portability of the C code. There would be a
great cost for dropping GNU make and I would not do it unless it is absolutely
necessary.

- Installed package size is slightly larger than 5MB on macOS and Windows
release/oldrel. The size of the libs dir is the culprit here, so this is hard to
change.


## Test environments

* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* R winbuilder (R 4.3.0 release, devel, oldrel)
* local macOS, R 4.3.1 (arm64)
* Debian with R-devel and clang (14.0.6 and 15.0.6)


## R CMD check results

Check status is OK on all test environments, with one exception. On
Windows-oldrel, I see SSL issues with URLs in README.md:

- https://developer.apple.com/xcode/ (self signed certificate in certificate chain)
- https://txm.gitpages.huma-num.fr/textometrie/ (unable to get local issuer certificate)

I do not see these on the  R winbuilder for R release of R devel. My browsers do
not show a problem with these certificates either. 


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, polmineR). I did not see
WARNINGs, or NOTEs, or ERRORs.
