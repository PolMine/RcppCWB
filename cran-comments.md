## General remarks

- This release replaces dependency PCRE with PCRE2 in the CWB source code 
included in this package. I hereby anticipate that build machines will 
successively not have PCRE installed. Fedora will presumably be first.

- Rcpp wrappers for CWB functionality is now exported such that it is possible 
to write Rcpp inline C++ functions using this functionality.

- This is the first package version that includes a vignette (explaining how to
write inline C++ functions). 

- The cleanup script is extended and reverts modifications that had previously
gone unnoticed. 

- A failure to build Windows binaries for R-oldrel is addressed: Repository 
https://github.com/PolMine/libcl that is used to get static libraries has been 
updated.

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
* R winbuilder (R 4.2 release, devel, oldrel)
* local macOS, R 4.2.2 (arm64)


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
