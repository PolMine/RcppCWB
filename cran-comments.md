## General remarks

This release fixes a compiler warning 'argument 1 value '18446744073709551615' exceeds maximum object size 9223372036854775807 [-Walloc-size-larger-than=]'. I have been notified that the fix is expected by September 28.

The (significant) compiler warning is newly thrown by GCC 14. I have used a Docker image with Fedora 40, R-devel (r87186) and GCC 14 to reproduce and fix the issue.

Previous aspects I repeat:

- CRAN package check results report 'GNU make is a SystemRequirements'. Using
GNU make remains important for the portability of the C code. There would be a
great cost for dropping GNU make and I would not do it unless it is absolutely
necessary.

- Installed package size is slightly larger than 5MB on macOS and Windows
release/oldrel. The size of the libs dir is the culprit here, so this is hard to
change.


## Test environments

* Docker image with Fedora 40, R-devel r87186 and GCC 14
* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* R winbuilder (R 4.3.3, R 4.4.1, R-devel r87186 ucrt)
* local macOS, R 4.3.1 (arm64)


## R CMD check results

Check status is OK on all test environments. A warning I have seen but that I cannot reproduce results from this website:

https://txm.gitpages.huma-num.fr/textometrie/ (unable to get local issuer certificate)

I do not see these on the  R winbuilder for R release of R devel. My browsers do
not show a problem with these certificates either. 


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, polmineR). I did not see
WARNINGs, or NOTEs, or ERRORs.
