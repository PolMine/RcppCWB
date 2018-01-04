library(RcppCWB)

testthat::context("cpos2lbound")

test_that(
  "id2str",
  {
    cpos <- cpos2lbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 5L,
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(cpos, 0)
  }
)
