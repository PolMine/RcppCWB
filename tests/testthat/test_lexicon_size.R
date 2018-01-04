library(RcppCWB)

testthat::context("lexicon_size")

test_that(
  "lexicon size",
  {
    N <- lexicon_size(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(N, 1192)
  }
)
