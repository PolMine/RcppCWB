library(RcppCWB)

testthat::context("cwb_lexicon_size")

test_that(
  "lexicon size",
  {
    N <- cwb_lexicon_size(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(N, 1192)
  }
)
