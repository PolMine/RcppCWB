library(RcppCWB)
use_tmp_registry()
testthat::context("cpos2struc")

test_that(
  "cpos2struc",
  {
    N <- cl_cpos2struc(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 100L,
      registry = get_tmp_registry()
    )
    expect_equal(N, 1L)
  }
)

test_that(
  "cpos2struc - new like old",
  {
    strucs_old <- cl_cpos2struc(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 1:100L,
      registry = get_tmp_registry()
    )
    
    s <- s_attr(corpus = "REUTERS", s_attribute = "places", registry = get_tmp_registry())
    strucs_new <- cpos_to_struc(1:100L, s_attr = s)
  }
)
