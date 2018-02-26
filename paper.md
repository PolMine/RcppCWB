The RcppCWB Package: Towards Text as Linguistic Data
====================================================

The Open Corpus Workbench (CWB) is an early development of an indexing and querying engine.
Its core functionality is to efficiently store large structurally and linguistically annotated corpora and to offer the query language of the Corpus Query Processor (CQP) to run simple to
complex queries that make use of the annotations of the corpus. The CWB has inspired a set of
developments such as Manatee, SketchEngine, and Finlib. But the CWB is still used as a 
backend for popular tools such as TXM or CQPweb. It is an old tool, yet it is actively maintained and developed, and offers a mature and efficient pure C implementation. LICENSE

The RcppCWB is the second approach to create a wrapper library to make the functionality 
of the CWB accessible from R. Indeed, the package rcqp, issued first at CRAN in 2015, still offers an extremely sound access to CWB functionality from R. However, the pure C implementation
of the rcqp package comes with high barriers to add further functionality to the package, and 
restricts possibilities to make it portable for Windows machines. The primary purpose of the RcppCWB package is to reimplement a wrapper library for the CWB using a design that makes it 
easy to maintain the package and to make it portable to Windows. The C libraries of the CWB 
are compiled as a first step as static libraries, here it is Rcpp wrappers that expose the 
functionality. In addition, RcppCWB includes a set of higher level functions implemented 
using Rcpp for standard performance criticial tasks.

Even though RcppCWB may be used in an analytical workflow directly, the best use of it may be
to use it as an interface for packages  with higher-level functionality that offer a portable R environment to work efficiently with large, linguistically and structurally annotated corpora.