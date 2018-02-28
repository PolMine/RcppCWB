library(RcppCWB)

testthat::context("cl_cpos2id")

test_that(
  "cpos2id",
  {
    ids <- cl_cpos2id(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry"),
      cpos = 0L:15L
    )
    expect_equal(ids, 0L:15L)
  }
)
