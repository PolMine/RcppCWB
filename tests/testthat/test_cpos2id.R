library(RcppCWB)
use_tmp_registry()
testthat::context("cl_cpos2id")

test_that(
  "cpos2id",
  {
    ids <- cl_cpos2id(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      cpos = 0L:15L
    )
    expect_equal(ids, 0L:15L)
  }
)

test_that(
  "cpos_to_id",
  {
    ids_old <- cl_cpos2id(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      cpos = 0L:15L
    )
    
    p <- p_attr(corpus = "REUTERS", p_attribute = "word", registry = get_tmp_registry())
    ids_new <- cpos_to_id(cpos = 0:15L, p_attr = p)
    expect_equal(ids_old, ids_new)
  }
)
