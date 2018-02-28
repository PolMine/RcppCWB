library(RcppCWB)

testthat::context("struc2cpos")

test_that(
  "struc2cpos",
  {
    cpos <- cl_struc2cpos(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry"),
      struc = 1L
    )
    expect_equal(cpos, c(92L, 535L))
  }
)
