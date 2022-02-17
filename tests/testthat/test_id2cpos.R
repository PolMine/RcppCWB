library(RcppCWB)
use_tmp_registry()
testthat::context("cl_id2cpos")

test_that(
  "id2cpos",
  {
    cpos <- cl_id2cpos(
      corpus = "REUTERS",
      p_attribute = "word",
      id = 15,
      registry = get_tmp_registry()
    )
    expect_equal(sum(cpos), 155788L)
  }
)


test_that(
  "id_to_cpos",
  {
    cpos_old <- cl_id2cpos(
      corpus = "REUTERS",
      p_attribute = "word",
      id = 15,
      registry = get_tmp_registry()
    )
    p <- p_attr(corpus = "REUTERS", p_attribute = "word", registry = get_tmp_registry())
    cpos_new <- id_to_cpos(id = 15L, p_attr = p)
    expect_identical(cpos_old, cpos_new)
  }
)
