## Patching the CWB for RcppCWB inclusion

The RcppCWB includes a patched version of the Corpus Workbench (CWB). RcppCWB versions up top v0.4.4 included a manually patched version of CWB (v3.4.14 / SVN revision 1069). But this is a serious limitation to incorporate the updates and improvements of the original CWB code. 

Automated patches are not performed using the `PatchEngine` R6 class defined in the file unsurprisingly named `PatchEngine.R`. At the current stage of development, automated patches will yield a 1:1 reconstruction of the manual patches introduced over the years for CWB v3.4.14 (r1069). Catching up with the latest version of CWB (r1690) is still work in progress.


## old and discarded patches

- To avoid warning of implicit function declaration, the header file time.h is added ("#include <time.h>"" in addition to sys/time.h) in cwb/cqp/sgml-print.c ; latex-print.c ; ascii-print.c ; html-print.c
