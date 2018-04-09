# v0.2.1
* In previous versions the drive of the working directory and of the 
registry/data directory had to be identical on Windows; this limitation 
does not persist;
* Some utility functions could be removed that were necessary to check the
identity of the drives of the working directory and the data.


# v0.2.0
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


# v0.1.7
* CWB source code included in package to be GPL compliant
* template to adjust HOME and INFO in registry file used (tools/setpaths.R)
* using VignetteBuilder has been removed
* definition of Rprintf in cwb/cl/macros.c

# v0.1.6
* now using configure/configure.win script in combination with setpaths.R

# v0.1.1
* vignette included that explains cross-compiling CWB for Windows
* check in struc2str to ensure that structure has attributes

# v0.1.0
* Windows compatibility (potentially still limited)
