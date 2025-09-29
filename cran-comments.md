## General remarks

This release fixes a warning thrown by Fedora/R-devel(clang: potential
stderr/stdout usage removed from C code.

Previous aspects I repeat:

- CRAN package check results report 'GNU make is a SystemRequirements'. Using
GNU make remains important for the portability of the C code. There would be a
great cost for dropping GNU make and I would not do it unless it is absolutely
necessary.

- Installed package size is slightly larger than 5MB on macOS and Windows
release/oldrel. The size of the libs dir is the culprit here, so this is hard to
change.


## Test environments

* Docker image with Fedora 42
* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* local macOS, R 4.5.1 (arm64)


## R CMD check results

Check status is OK on all test environments.


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, polmineR). I did not see
WARNINGs, or NOTEs, or ERRORs.
