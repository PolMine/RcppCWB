library(RcppCWB)

testthat::context("cwb_str2id")

test_that(
  "str2id",
  {
    id <- cwb_str2id(
      corpus = "REUTERS",
      p_attribute = "word",
      str = "oil",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(id, 15L)
  }
)
