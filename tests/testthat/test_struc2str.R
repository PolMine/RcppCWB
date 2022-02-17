library(RcppCWB)
use_tmp_registry()

testthat::context("struc2str")

test_that(
  "struc2str",
  {
    sAttr <- cl_struc2str(
      corpus = "REUTERS",
      s_attribute = "places",
      struc = 2L,
      registry = get_tmp_registry()
    )
    expect_equal(sAttr, "canada")
  }
)

test_that(
  "struc_to_str",
  {
    value_old <- cl_struc2str(
      corpus = "REUTERS",
      s_attribute = "places",
      struc = 2L,
      registry = get_tmp_registry()
    )
    s <- s_attr(corpus = "REUTERS", s_attribute = "places", registry = get_tmp_registry())
    value_new <- struc_to_str(struc = 2L, s_attr = s)
    expect_identical(value_old, value_new)
  }
)
