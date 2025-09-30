## General remarks

To explain the quick follow-up:_ RcppCWB v0.6.9 fixed a warning thrown by
Fedora/R-devel/clang: stderr/stdout symbols in RcppCWB.so. CRAN checks show,
that this issue re-occurrs:
https://www.r-project.org/nosvn/R.check/r-devel-linux-x86_64-fedora-clang/RcppCWB-00check.html

There is a side effect of clang version 21.1.2, the NOTE did not show last
week with clang version 21.1.1.

I want to solve/address this issue, before you send out a message. On this 
occasion: v0.6.9 had solved the known issue. LLVM development seems to be 
fairly dynamic, newest dev versions of the clang linker apparently pull in
symbols into the dynamic library that previously remained outside. 

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
