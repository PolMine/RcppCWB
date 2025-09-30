# RcppCWB 0.6.10

* Fixes stderr/stout usage in C code of the Corpus Workbench that is issued by
the latest dev version of LLVM #102.

# RcppCWB 0.6.9

* Fixes stderr/stout usage in C code #100.

# RcppCWB 0.6.8

* Fixes null destination pointer warning issued by gcc-ASAN #99.

# RcppCWB 0.6.5

* Fixes a 'exceeds maximum object size'-compiler warning #93.


# RcppCWB 0.6.4

* `cwb_huffcode()` and `cwb_compress_rdx()` did not delete redundant files on 
Windows. Fixed by temporarily unloading the corpus #89.
* `cwb_encode()` failed if argument `s_attributes` was empty list. Fixed, the
default value of `s_attributes` is now `list()` #90.
* `cwb_makeall()` will not reset CORPUS_REGISTY environment variable implicitly
if corpus to process has already been loaded #92.
* Architecture "aarch64"" (equivalent to "amd64" / Apple Silicon) as known 
Linux architecture (= scenario when running a Docker container on MacBook) #91.
* Functions `cwb_makeall()`, `cwb_huffcode()` and `cwb_compress_rdx()` have
new argument `logfile` to redirect output to this file. Requires argument
`quietly`  to be `TRUE` #65.


# RcppCWB 0.6.3

* `cl_struc_values()` does not duplicate registry directories any more #77. 
* Fix format-security issue under r-devel #86.
* `get_region_matrix()` reports NA values for negative strucs #87.
* `region_matrix_to_struc_matrix()` returns NA values for regions without 
nested region as declared in the documentation #88.
* `check_strucs()` issues warning if negative values are passed and if length of
input vector is 0.
* `ranges_to_cpos()` drops rows from input matrix with NA values and issues 
a respective warning.


# RcppCWB 0.6.2

* The configure script now covers the case of Power PCs. Files for the power pc 
scenario have been added to src/cwb/config/platform; darwin-64 has been renamed
to darwin-x86_64 as a matter of consistency #79.
* Warning "variable 'nr_targets' set but not used" for files newly reported by
Apple clang version 14.0.3 (clang-1403.0.22.14.1) is addressed #83.
* Misleading indentation warning issued by clang-15 addressed #85.
* `cwb_encode()`, `cwb_makeall()`, `cwb_huffcode()` and `cwb_compress_rdx()` 
perform tilde expansion on filename provided by argument `registry`, avoiding
a crash #84.



# RcppCWB 0.6.1

* New function `region_to_strucs()` to get minimumum and maximum struc of 
s-attribute within region provided. Works also for nested s-attributes.
* New function `region_matrix_to_struc_matrix()`.
* Functions `cl_cpos2lbound()` and `cl_cpos2rbound()` return NA if corpus
position is outside stru for given s-attribute. #78.
* Functions `cl_cpos2lbound()` and `cl_cpos2rbound()` are exposed directly from
C++ without R wrappers, improving performance. Using the environment variable
'CORPUS_REGISTRY' if argument `registry` is handled implicitly now.


# RcppCWB 0.6.0

* Rcpp wrappers for Corpus Library (CL) functions are exposed directly and  
can be used in C++ functions imported using `Rcpp::sourceCpp()` or
`Rcpp::cppFunction()`.
* Dependency PCRE has been updated to PCRE2 #68.
* The README suggested to install the development version of RcppCWB using the 
snippet `devtools::install_github("PolMine/RcppCWB")`. The missing `ref = "dev"`
has been inserted.
* `cwb_encode()` crashed if arguments `data_dir` and `vrt_dir` include a tilde.
Tilde expansion is now applied to these arguments to avoid this #73.
* A new vignette explains how to write C++ inline functions.

# RcppCWB 0.5.5

* C++ code replaces `sprintf()` with `snprintf()` to address security issue.
* Package now depends on Rcpp v1.0.10, which replaces one remaining `sprintf()`
#70.
* `corpus_properties()` and `corpus_property()` do not crash any more, if corpus
is not loaded or not present #69.
* New function `p_attr_default()` to programmatically extract default
p-attribute #63.

# RcppCWB 0.5.4

* Fixed package configuration that prevented that compiler is used for compiling
CWB C scripts as intended #66.
* Adding '-luuid' to PKG_FLAGS in Makevars solves linker issue FOLDERID_ #67.
* GitHub Actions now working for Windows #47.


# RcppCWB 0.5.3

* Fixed a bug in the `region_matrix_corpus()` C++ code that would not show any
context at all if s_attribute expansion transgressed start or end of corpus.
* Fixed a bug in the `region_matrix_corpus()` C++ code that would result from 
not considering that query matches may go cover more than one strucs of a 
structural attribute.
* `corpus_info_file()` does not crash if INFO is not defined in the registry
file (#62).
* Implicit processing of arguments `sAttribute` and `pAttribute` as `s_attribute`
or `p_attribute` respectively is now accompanied by a warning that arguments 
are deprectated.
* The `check_corpus()` function distinguishes between whether a corpus is loaded
in the CL and/or CQP context.
* `cwb_huffcode()` and `cwb_compress_rdx()` have argument `delete` to trigger
deleting redundant files after compression (#60).
* `cqp_load_corpus` will internally upper corpus ID as required in the CQP
context (#64).

# RcppCWB 0.5.2

* The example for `corpus_data_dir()` dir not work as intended without
explicitly setting the `registry` argument. Fixed.
* New functions `corpus_info_file()`, `corpus_full_name()`,
`corpus_p_attributes()`, `corpus_s_attributes()`, `corpus_properties()` and
`corpus_property()` to retrieve registry file data.
* New function `corpus_registry_dir()`.
* The path to the info file in the registry file of the REUTERS corpus was
broken. Fixed.


# RcppCWB 0.5.1

## New Features

* New auxiliary function `cwb_charsets()` reports the charsets supported by CWB.
* New functions `cl_load_corpus()` and `cqp_load_corpus()` do what the functions
suggests.
* New function `cl_list_corpora()` complements existing function
`cqp_list_corpora()` for the CL context.
* New arguments `skip_blank_lines`, `strip_whitespace` and `xml` of
`cwb_encode()` open configuration options of `cwb_encode()`, overcoming the
previously hard-coded equivalent to the command-line option "-xsB".(#38)
* Unexported functions `.cpos_to_id()`, `.cl_find_corpus()` and
`.cl_new_attribute()` are an entry to passing around pointers, rather than
re-creating objects whenever switching from R to C.
* Functions `.s_attr()` and `.p_attr()` return pointers for a s- or
p-attribute.
* Functions `cl_*` are now available with pointer as input (e.g. `cpos_to_id()`).
* The CORPUS_REGISTRY environment variable is not set to the temporary registry,
to avoid often confusing behavior and collissions whent loading RcppCWB and 
polmineR at the same time (#13).
* The `cqp_drop_subcorpus()` function that has been disabled temporarily is
usable again (#34).
* `cqp_query()` is now able to process subcorpora.
* `RcppCWB:::.cqp_subcropus()` will construct a subcorpus from a region matrix.
* The `check_corpus()` does not re-set the registry directory and more, but tries
to load the checked corpus if it has not yet been loaded.
* A new function `s_attr_relationship()` will detect whether two s-attributes are
siblings, or in a descendent or ancestor relationship.
* Functions `cwb_encode()`, `cwb_huffcode()`, `cwb_makeall()` and
`cwb_compress_rdx()` now have an argument `quietly` to control display of output
messages. `cwb_encode()` has an argument `verbose` to control whether counter on
the number of tokens processed is dislpayed.


## Minor improvements

* Difficulties of `cwb_encode()` to digest variations of path statements between
macOS and Windows are addressed using a reliable normalization of paths with
`fs::path()` (#48).
* Argument `encoding` is checked for the validity of the encoding passed in
(#34).
* A patch introducing a sanity check omits 'stringop-overflow'  compiler warning
thrown by file cl/cdaccess.c on Windows (#45).
* An update of Xcode command line developer tools includes flex 2.6.4
Apple(flex-34), and this is the version used not, resulting and extensive code
changes in cl/lex.creg.c and cqp/lex.yy.c, yet without causing new errors or
changing the functionality.
* `check_cpos()` issues a warning if argument `cpos` is `NULL` (#21).
* Functions `cl_cpos2id()`, `cl_cpos2lbound()`, `cl_cpos2rbound()`,
`cl_cpos2str()` and `cl_cpo2struc()` will return an empty, zero-length integer
vector if argument `cpos` is `NULL` (#21).
* Warnings issued by `check_corpus()` (used internally by many functions)
resulted from slightly differing representations of otherwise identical 
paths. Using `fs::path()` for path for normalization internally will omit
misleading warning messages.
* `cqp_get_registry()` will now return a `fs::path` object, as a safeguard for
a consistent normalization of paths.
* Function `cl_delete_corpus()` will now (visibly) return a `logial` value.
* The check for the availability of ncurses is omitted in the configure file
and the editline subdirectory of src/cwb is included in .Rbuildignore to 
minimize the size of the tarball. The ncurses library is a dependency of 
editline, but editline is not built in the context of this package (#26).
* `cqp_load_corpus()` will return `FALSE` if corpus has not been loaded
successfully.
* Disaggregated `wrappers.cpp` into `cl.cpp`, `cqp.cpp` and `utils.cpp`, so that
the code is organized more coherently corresponding to the different logics.
* Function `check_cqp_query()` renamed to `check_query()` to avoid a conflict
with a function defined in the polmineR package.
* `cqp_list_subcorpora()` returns a `character` vector. Previously, we just had
obscure printed messages.
* `s_attribute_decode()` will not break if s-attribute has no values (#54).
* Functions `cl_struc2str()` and `cl_struc2cpos()` may now include negative
values, the vectors returned will have `NA` values at respective positions. The
check against negative values in `check_strucs` is dropped accordingly.


## Bux fixes

* The `cwb_encode()` function did not declare structural attributes in the
registry and mistakenly channeled output for the file to the terminal (#49).
Fixed.
* Re-running `cwb_encode()` did not reset global variables, which resulted in a
set of errors. Solved. (#51)



# RcppCWB 0.5.0

## New Features

* The CWB code is updated to v3.4.33 / r1690 (#29). Automated patches that have
been developed are a safeguard that it will be painless in the future to align
RcppCWB with upstream CWB development.
* The C code in the files `cwb-huffcode.c`, `cwb-compress-rdx.c` and `cwb-makeall.c`
was not in line with the CWB version of the rest of the code (v3.4.14 / SVN
revision 1069) but rather v2.2.b99 or v3.0.0. All code changes up to v3.4.14 were
reconstructed and implemented (#35). Note that `cwb-encode.c` was at CWB v3.4.14,
as the encoding functionality was exposed at a later stage.
* A new function `cwb_version()` will report the version of the CWB source code.
* The `cwb_encode()` function now has a previously missing argument `encoding`
to state the encoding of the corpus to be indexed.
* Reduced number of  example *.vrt-files to one to keep package size below 5GB.

## Minor Improvements

* Encoding a cropus using `cwb_encode()` now assumes implicitly that input files
are XML files and remove blank lines and leading and trailing whitespace. This
is equivalent to the option "-xsB" of the command line utility `cwb-encode`.
* The C++ code of `cwb_encode()` is now a patch of the `main()` function of
`cwb-encode.c`, so that code in the *.cpp file can be limited to a slim wrapper,
limiting the risk that the code in RcppCWB looses touch with CWB upstream
development.
* Header files `_eval.h`, `_globalvars.h` and `_cl.h` in the `./src` directory
are autogenerated files now, not to be edited by hand.
* The C++ code of the `cqp_drop_subcorpus()` function is temporarily disabled to
ensure that the package can be built (#34).


# RcppCWB 0.4.4

* Fixed a mishandling of paths on Windows in `check_corpus()` that would trigger resetting the registry unintendendly and potentially falsely.
* To avoid a compiler warning (unused variable) issued by Rcpp solved by Rcpp v1.0.7, this version of Rcpp is now required (#22).
* In `use_tmp_dir()`, `normalizePath()` is applied on the `tempdir()` result to avoid confusion with symbolic links on macOS.
* New unit test for `cwb_encode()` (not yet run on Windows).
* A C-level inconsistency in `cqp_get_registry()` that would sometimes result in a wrong return value (i.e. registry path) has been fixed (#14).
* To avoid an unintended behavior of `cwb_makeall()`, an internal check is performed whether the corpus has been loaded already and whether the home directory of the loaded corpus and defined in the registry file are identical (#31).
* The link to the TXM project has been removed from the documentation to avoid the error 'SSL certificate problem: unable to get local issuer certificate' (#32).
* The `cl_delete_corpus()` function crashed when trying to delete a corpus that has not been loaded (#33). The function now aborts gracefully returning 0 when trying to delete a corpus that has not been loaded.
* A new function `corpus_is_loaded()` can be used to check whether a corpus is loaded.


# RcppCWB 0.4.3

* Unused file '_options.h' removed from src/cwb/cl/cqp 
* Targets 'lex.creg.c', 'registry.tab.c' and 'registry.tab.h' removed from cl/Makefile to avoid an unwanted call of flex which is not necessarily present (#30).

# RcppCWB 0.4.2

* Windows builds will be linked with a fresh and fully reproducible cross-compilation of CWB static libraries, see the PolMine/libcl repository. The consolidation of the workflow to prepare cross-compiled static libraries is a preparatory step to enable UCRT builds on Windows.
* The Range struc in the code for util functionality (encode and more, files utils.h, utils.cpp and _cwb_encode.c) has been renamed as SAttrEncoder to avoid a C++ One Definition Rule warning resulting for a struc with the same name in the CL context (#28).


# RcppCWB 0.4.1

* A shortcoming when passing in variables into the format string to construct the PKG_LIBS variable resulted in a faulty call of the linker on Solaris and a compilation error. Fixed (#25).
* A hacky and recently unnecessary LDFLAG "-Wl,--allow-multiple-definition" on Solaris has been dropped.
* Usage and evaluation of the pcretest utility is now in line with POSIX requirements, omitting an error on Solaris. A statement on the availability of the tool provides information whether it is available at all (#24).
* The message on the findability of ncurses is more telling now, avoiding a "mission critial"-style alarm when ncurses may be present but is not findable by pkg-config (#26).


# RcppCWB 0.4.0

## New Features

* Encode XML (vrt file format) with new function `cwb_encode()` that exposes functionality of cwb-encode CWB utility.
* Functions `cl_cpos2lbound()` and `cl_cpos2rbound()` will now accept an integer vector with length > 1 as argument `cpos` and return a vector with the same length. Useful to speed up iterated queries for left and right boundaries of regions (#19).
* A new function `cl_struc_values()` exposes the corresponding C function of the Corpus Library (CL). The previous implicit assumption that all structural attributes have values can thus be tested. Intended to work with annotations of sentences and paragraphs, i.e. common structural attributes that do usually not have values.
* A new function `corpus_data_dir()` will derive the data directory from the internal C representation of a corpus.
* New function `s_attr_regions()` will derive regions defined by a structural attribute from the *.rng file. Fastest option for large corpora.
* New functions `s_attr_is_sibling()` and `s_attr_is_descendent()` test the sibling/descendent relationship of structural attributes.


## Minor Improvements

* Function `check_corpus()` now includes checks whether the registry provided (argument `registry`) is identical with the registry defined internally by CQP. The registry is reset if directories are not identical.
* Minor adjustments of configure script for aarch64, adding -fPIC to CFLAGS so that this flag will be used when Linux default configuration is used as fallback.
* The implementation of the `s_attribute_decode()` method was incomplete for method "Rcpp". This alternative to the "pure R" approach is now implemented (#2).
* The unused file 'setpaths.R' has been removed from the tools directory (#10).
* The argument `method` previously setting "wininet" in ./tools/winlibs.R is omitted to avoid the warning "the 'wininet' method is deprecated for http:// and https:// URLs" on Windows.
* The configure script will print the libdirs derived using pcre-config and link against libintl on macOS by default.


# RcppCWB 0.3.2

* If RcppCWB is compiled on macOS, the package configure script checks the architecture of the machine and ensures that (if glib-2.0 is not yet present) a version of glib-2.0 compiled for Apple Silicon/the M1 chip is loaded in case an amd64 architecture is detected.
* The package configure script now uses `pcre-config` to locate header files of PCRE.
* The configure script checks whether pcre has been compiled with Unicode properties support. If not, a warning is issued that also explains the recommended solution to use '--enable-unicode-properties' when calling configure.

# RcppCWB 0.3.0

* To avoid warnings when running R CMD check, the http://pcre.org is used rather than https://pcre.org in the DESCRIPTION and the README file.
* To overcome a somewhat dirty solution for multiple symbol definitions, adding the 
'fcommon' flag to the CFLAGS in the configure script has been removed. The C code 
has been modified such that multiple symbol definitions are omitted.
* The macOS image used for test on Travis CI is now 'xcode9.4'
* On Solaris, the configure script would define the flag "-Wl,--allow-multiple-definition" to be passed 
to the linker flags. The rework of the CWB includes and the inclusion of the header file 'env.h' makes it
possible to drop this flag. It was defined at a confusing place anyway.
* Using the compiler desired by the user (in Makeconf, Makevars file) is now there for all OSes.
* If pkg-config is not present on macOS, a warning is issued; the user gets the advice to use the brew 
package manager to install pkg-config.
* There is an explicit check in the configure script whether the dependencies ncurses, pcre and glib-2.0
are present. If not, a telling error with installation instructions is displayed.
* When unloading the package, the dynamic library RcppCWB.so is unloaded.
* When loading the package, CQP is initialized by default (call `cqp_initialize()`)


# RcppCWB 0.2.9

* Starting with GCC 10, the compiler defaults to -fno-common, resulting in error messages during the linker stage, see [the change log of the GCC compiler](https://gcc.gnu.org/gcc-10/changes.html). To address this issue, the -fcommon option is now used by default when compiling the CWB C files on Linux 64bit systems. The CWB code includes header files multiple times, causing multiple definitions.
* On Linux systems, the hard-coded definition as the preferred C compiler in the CWB configuration sripts will be replaced by what the CC variable defines (in ~/.R/Makevars or the Makeconf file, the result returned by R CMD config CC).
* Remaining bashisms have been removed from the cleanup file. The shebang line of the
cleanup and the configure file is now #!/bin/sh, to avoid any reliance on bash.

# RcppCWB 0.2.8

* There have been (minor) modifiations of the C code of the CWB so that compilation succeeds on Solaris.
* Using the '-C' flag in the CWB Makefiles has been replaced by 'cd cl' / 'cd cqp' to avoid dependence on GNU make. GNU make is still required, because of 'include' statements in the Makefiles.
* Removed an action on 'depend.mk' from 'cleanup' script to avoid error messages that depend.mk is not present when Makefiles are first loaded.
* Dummy depend.mk files will satisfy include statement in Makefiles when running 'make clean' (depend.mk files are created only when running depend.mk)
* For creating index of static archives (libcl, libcqb, libcwb), a call to 'ranlib' has been replaced by an equivalent 'ar -s' in the Makefiles, but commented out.
* In the platform-specific config files of the CWB, the '-march'-option has been taken out, to safeguard portability.
* To meet the requirements of the upcoming changes in the CRAN check process to use staged installs, the procedure to reset the paths in the test data within the package has been replaced throughout by using a temporary registry directory. The `get_tmp_registry()` will return the whereabouts of this directory.


# RcppCWB 0.2.7

* If glib-2.0 is not present on macOS, binaries of the static library and 
header files are downloaded from a GitHub repo. This prepares to get RcppCWB
pass macOS checking on CRAN machines.
* A slight modification of the C code will now prevent previous crashes resulting
from a faulty CQP syntax. The solution will not yet be effective for Windows
systems until we have recompiled the libcqp static library that is downloaded
during the installation process.
* A new C++-level function 'check_corpus' checks whether a given corpus is
available and is used by the `check_corpus()`-function. Problems with 
the previous implementation that relied on files in the registry directory to
ensure the presence of a corpus hopefully do not occur.
* Calling the 'find_readline.perl' utility script is omitted on macOS, so 
previous warning messages when running the makefile do not show up any more.

# RcppCWB 0.2.6

* Function `cl_charset_name()` is exposed, it will return the charset of a 
corpus. Faster than parsing the registry file again and again.
* A new `cl_delete_corpus()`-function can remove loaded corpora from memory.


# RcppCWB 0.2.5

* In Makevars.win, libiconv is explicitly linked, to make RcppCWB compatible with new
release of Rtools.
* regex in check_s_attribute() for parsing registry file improved so that it does not
produce an error if '# [attribute]' follows after declaration of s_attribute


# RcppCWB 0.2.4
* for linux and macOS, CWB 3.4.14 included, so that UTF-8 support is realized
* bug removed in check_cqp_query that would prevent special characters from working
in CQP queries
* check_strucs, check_cpos and check_id are checking for NAs now to avoid crashes
* cwb command line tools cwb-makeall, cwb-huffcode and cwb-compress-rdx exposed
  as cwb_makeall, cwb_huffcode and cwb_compress_rdx

# RcppCWB 0.2.3

* when loading the package, a check is performed to make sure that paths in the 
registry files point to the data files of the sample data (issues may occur when
installing binaries)
* auxiliary functions to check whether input to Rcpp-wrappers/C functions is valid
are now exported and documented
* more consistent validity checks of input to functions for structural attributes


# RcppCWB 0.2.2

* Compiling RcppCWB on unix-like systems (macOS, Linux) will work now without
the presence of glib (on Windows, the dependency persists).#
* The presence of the bison parser is not required any more. The package includes 
the C source generated by the bison parser along with the original input files.
* Functionality to generate CWB-indexed corpora and to generate and manipulate
the registry file describing a corpus has been moved to a new package 'cwbtools'
(see https://www.github.com/PolMine/cwbtools) in order to maintain a clearly
defined scope of RcppCWB to expose functionality of the C code of the CWB.
* Minor intervention in function 'valid_subcorpus_name' to omit a -Wtautological-pointer-compare warning leading to a WARNING when checking package
for R 3.5.0 with option --as-cran

# RcppCWB 0.2.1

* In previous versions the drive of the working directory and of the 
registry/data directory had to be identical on Windows; this limitation 
does not persist;
* Some utility functions could be removed that were necessary to check the
identity of the drives of the working directory and the data.


# RcppCWB 0.2.0

* In addition to low-level functionality of the corpus library (CL), functions
of the Corpus Query Processor (CQP) are exposed, building  on C wrappers in the
rcqp package;
* The authors of the rcqp package (Bernard Desgraupes and Sylvain Loiseau) are
mentioned as package authors and as authors of functions using CQP, as the code
used to expose CQP functionality is a modified version of rcqp code;
* Extended package description explaining the rationale for developing the 
RcppCWB package;
* Documentation of functions has been rearranged, many examples have been 
included;
* Renaming of exposed functions of corpus library from cwb_... to cl_...;
* sanity checks in R wrappers for Rcpp functions.


# RcppCWB 0.1.7

* CWB source code included in package to be GPL compliant
* template to adjust HOME and INFO in registry file used (tools/setpaths.R)
* using VignetteBuilder has been removed
* definition of Rprintf in cwb/cl/macros.c

# RcppCWB 0.1.6

* now using configure/configure.win script in combination with setpaths.R


# RcppCWB 0.1.1

* vignette included that explains cross-compiling CWB for Windows
* check in struc2str to ensure that structure has attributes


# RcppCWB 0.1.0

* Windows compatibility (potentially still limited)
