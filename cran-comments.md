## General remarks

[This is the 2nd submission. Automated patches for UCRT were still in place this
morning. As I gather from a check with win-builder.r-project.org a few minutes ago,
skipping these patches is effective now.]

The RcppCWB package is a wrapper for the Corpus Workbench (CWB). Previous versions
included the C code of an increasingly outdated version of CWB (CWB v3.4.14).
This version re-aligns RcppCWB with upstream CWB development (CWB v3.4.44).

It uses automated patches (see code in ./patch) to make the CWB fit into this R
package.  It will now be painless to  include the most recent (patched)
CWB code in this package.

The most important immediate purpose of this release is to integrate
patches developed by Tomas Kalibera, bringing it up the the requirements of the
UCRT toolchain. It is not necessary to apply patches at CRAN
(installation with _R_INSTALL_TIME_PATCHES_=no)

On this occassion, I would like to gratefully acknowledge the inredible helpful
guidance and support of Tomas Kalibera, and support and advice offered by Brian
Ripley.


## Test environments

* Standard checks with R-hub
* CI checks with GitHub Actions (Windows/macOS/Ubuntu)
* local macOS R 4.1.2 (both x86_64 and arm64)
* local Windows machine (both R 4.1.2 and R 4.2)


## R CMD check results

Compatibility with the UCRT toolchain has been the primary concern of this
release. I applied patches to remove all compiler warnings I saw. Yet there is
one remaining 'stringop-overflow' compiler warning indicating a fairly
unlikely scenario that I find hard to address at this time. 

gcc -c -o cdaccess.o -O2 -Wall -D__MINGW__ -DEMULATE_SETENV -DCOMPILE_DATE=\""Sat Jan 29 07:12:07 WEST 2022"\" -DCWB_VERSION=\"3.4.33\" -IC:/rtools42/x86_64-w64-mingw32.static.posix/include/glib-2.0 -IC:/rtools42/x86_64-w64-mingw32.static.posix/lib/glib-2.0/include -I/rtools42/x86_64-w64-mingw32.static.posix/include/glib-2.0 -DPCRE_STATIC cdaccess.c
cdaccess.c: In function 'cl_read_stream':
cdaccess.c:982:5: warning: 'memcpy' specified bound between 18446744065119617024 and 18446744073709551612 exceeds maximum object size 9223372036854775807 [-Wstringop-overflow=]
 | memcpy(buffer, ps->base + ps->nr_items, items_to_read * sizeof(int));
 | ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


## Downstream dependencies

I have also checked downstream dependencies (cwbtools, GermaParl, polmineR). I did not see WARNINGs, or NOTEs, or ERRORs.
