library(RcppCWB)
use_tmp_registry()
testthat::context("cl_str2id")

test_that(
  "str2id",
  {
    id <- cl_str2id(
      corpus = "REUTERS",
      p_attribute = "word",
      str = "oil",
      registry = get_tmp_registry()
    )
    expect_equal(id, 15L)
  }
)

test_that(
  "str_to_id",
  {
    id_old <- cl_str2id(
      corpus = "REUTERS",
      p_attribute = "word",
      str = "oil",
      registry = get_tmp_registry()
    )
    p <- p_attr(corpus = "REUTERS", p_attribute = "word", registry = get_tmp_registry())
    id_new <- str_to_id(str = "oil", p_attr = p)
    expect_equal(id_old, id_new)
  }
)
