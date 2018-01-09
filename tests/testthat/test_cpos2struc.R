library(RcppCWB)

testthat::context("cpos2struc")

test_that(
  "cpos2struc",
  {
    N <- cwb_cpos2struc(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 100L,
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(N, 1L)
  }
)
