library(RcppCWB)

testthat::context("regex2id")

test_that(
  "regex2id",
  {
    id <- regex2id(
      corpus = "REUTERS",
      p_attribute = "word",
      regex = "[oO]il",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(id, c(15L, 306L))
  }
)
