library(RcppCWB)

testthat::context("cwb_regex2id")

test_that(
  "regex2id",
  {
    id <- cwb_regex2id(
      corpus = "REUTERS",
      p_attribute = "word",
      regex = "[oO]il",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(id, c(15L, 306L))
  }
)
