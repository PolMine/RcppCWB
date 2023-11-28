library(RcppCWB)
use_tmp_registry()
testthat::context("struc2cpos")

test_that(
  "cl_struc_values",
  {
    # This addresses issue #77: cl_struc_values() would have the result that
    # a corpus is loaded twice.
    
    regdir <- corpus_registry_dir("REUTERS")
    
    expect_identical(
      length(corpus_registry_dir("REUTERS")),
      1L
    )
    
    cl_struc_values(corpus = "REUTERS", s_attribute = "id", registry = regdir)
    
    expect_identical(
      length(corpus_registry_dir("REUTERS")),
      1L
    )
  }
)
