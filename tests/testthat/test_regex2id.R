library(RcppCWB)
use_tmp_registry()
testthat::context("cl_regex2id")

test_that(
  "regex2id",
  {
    id <- cl_regex2id(
      corpus = "REUTERS",
      p_attribute = "word",
      regex = "[oO]il",
      registry = get_tmp_registry()
    )
    expect_equal(id, c(15L, 306L))
  }
)

test_that(
  "regex_to_id",
  {
    id_old <- cl_regex2id(
      corpus = "REUTERS",
      p_attribute = "word",
      regex = "[oO]il",
      registry = get_tmp_registry()
    )
    p_attr <- p_attr(corpus = "REUTERS", p_attribute = "word", registry = get_tmp_registry())
    id_new <- regex_to_id(regex = "[oO]il", p_attr = p_attr)
    expect_identical(id_old, id_new)
  }
)
