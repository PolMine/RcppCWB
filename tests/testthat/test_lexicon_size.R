library(RcppCWB)
use_tmp_registry()
testthat::context("cl_lexicon_size")

test_that(
  "lexicon size",
  {
    N <- cl_lexicon_size(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry()
    )
    expect_equal(N, 1192)
  }
)

test_that(
  "lexicon size works for experimental functionality",
  {
    old <- cl_lexicon_size(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry()
    )
    new <- p_attr_lexicon_size(
      p_attr("REUTERS", "word", registry = get_tmp_registry())
    )
    expect_identical(old, new)
  }
)
