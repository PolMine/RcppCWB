library(RcppCWB)

testthat::context("cwb_cpos2str")

test_that(
  "cpos2str",
  {
    token <- cwb_cpos2str(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry"),
      cpos = 0L:3L
    )
    expect_equal(token, c("Diamond", "Shamrock", "Corp", "said"))
  }
)
