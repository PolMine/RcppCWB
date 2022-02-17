library(RcppCWB)
use_tmp_registry()
testthat::context("struc2cpos")

test_that(
  "struc2cpos",
  {
    cpos <- cl_struc2cpos(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = get_tmp_registry(),
      struc = 1L
    )
    expect_equal(cpos, c(92L, 535L))
  }
)

test_that(
  "struc_to_cpos",
  {
    cpos_old <- cl_struc2cpos(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = get_tmp_registry(),
      struc = 1L
    )
    s <- s_attr(corpus = "REUTERS", s_attribute = "places", registry = get_tmp_registry())
    cpos_new <- struc_to_cpos(struc = 1L, s_attr = s)
    expect_identical(cpos_old, cpos_new)
  }
)
