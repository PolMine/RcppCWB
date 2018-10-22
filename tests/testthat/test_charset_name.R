library(RcppCWB)

testthat::context("cl_charset_name")

test_that(
  "cl_charset_name",
  {
    charset <- cl_charset_name(
      corpus = "REUTERS",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
      
    expect_equal(charset, "latin1")
  }
)
