library(RcppCWB)
use_tmp_registry()
testthat::context("cl_cpos2str")

test_that(
  "cpos2str",
  {
    token <- cl_cpos2str(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      cpos = 0L:3L
    )
    expect_equal(token, c("Diamond", "Shamrock", "Corp", "said"))
  }
)

test_that(
  "cpos2str - check new against old",
  {
    old <- cl_cpos2str(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      cpos = 0L:3L
    )
    
    p <- p_attr(corpus = "REUTERS", p_attribute = "word", registry = get_tmp_registry())
    new <- cpos_to_str(cpos = 0L:3L, p_attr = p)
    expect_identical(old, new)
  }
)
