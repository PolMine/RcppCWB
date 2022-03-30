## Patching the CWB for RcppCWB inclusion

The RcppCWB includes a patched version of the Corpus Workbench (CWB). RcppCWB versions up top v0.4.4 included a manually patched version of CWB (v3.4.14 / SVN revision 1069). But this is a serious limitation to incorporate the updates and improvements of the original CWB code. 

Automated patches are not performed using the `PatchEngine` R6 class defined in the file unsurprisingly named `PatchEngine.R`. At the current stage of development, automated patches will yield a 1:1 reconstruction of the manual patches introduced over the years for CWB v3.4.14 (r1069). Catching up with the latest version of CWB (r1690) is still work in progress.


## Prerequisites

- [bison](https://www.gnu.org/software/bison/): The PatchCWB mechanism has been developed using GNU Bison 2.3. The latest version installable via `brew` is v3.7.4. Note that the latest version available would be v3.8.

- [flex](https://github.com/westes/flex): The PatchCWB mechanism has been developed using flex 2.5.35 Apple(flex-32). An Update of the Xcode command line developer tools includes flex 2.6.4 Apple(flex-34), which is the latest version to date. This (v2.6.4) is the version used for the patches.

- [makeheaders](): Utility to generate a C header file from the original C code. Available via [Fossil](https://fossil-scm.org/home/file/src/makeheaders.c) and at [GitHub](https://github.com/bjconlan/makeheaders.git). To build the executable, cd into the directory with `makeheaders.c` and run:

```{sh}
gcc -c makeheaders.c
gcc makeheaders.o -o makeheaders
```

## Notes

- We do not patch the general Makefile but copy a manually patched version of r1069: The diffs between r1069 and r1690 show differences with new targets, but not relevant here.
- The Makefile utils/Makefile is copied in - r1690 has better verbosity but does not change substantially anything



## old and discarded patches

- To avoid warning of implicit function declaration, the header file time.h is added ("#include <time.h>"" in addition to sys/time.h) in cwb/cqp/sgml-print.c ; latex-print.c ; ascii-print.c ; html-print.c


## To do

- check whether newer version of flex and bison yield different results (does flex/bison version matter? require different patches?)