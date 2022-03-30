library(RcppCWB)
use_tmp_registry()
testthat::context("cl_cpos2lbound")

test_that(
  "cpos2lbound",
  {
    cpos <- cl_cpos2lbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 5L,
      registry = get_tmp_registry()
    )
    expect_equal(cpos, 0)
  }
)

test_that(
  "cpos_to_lbound",
  {
    cpos_old <- cl_cpos2lbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 5L,
      registry = get_tmp_registry()
    )
    s <- s_attr(corpus = "REUTERS", s_attribute = "places", registry = get_tmp_registry())
    cpos_new <- cpos_to_lbound(cpos = 5L, s_attr = s)
    expect_identical(cpos_old, cpos_new)
  }
)


test_that(
  "cpos2rbound",
  {
    cpos <- cl_cpos2rbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 5L,
      registry = get_tmp_registry()
    )
    expect_equal(cpos, 91L)
  }
)

test_that(
  "cpos_to_rbound",
  {
    cpos_old <- cl_cpos2rbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 5L,
      registry = get_tmp_registry()
    )
    s <- s_attr(corpus = "REUTERS", s_attribute = "places", registry = get_tmp_registry())
    cpos_new <- cpos_to_rbound(cpos = 5L, s_attr = s)
    expect_identical(cpos_old, cpos_new)
  }
)


test_that(
  "",
  {
    attsize <- cl_attribute_size(
      corpus = "REUTERS",
      attribute = "word", attribute_type = "p",
      registry = get_tmp_registry()
    )
    
    lbound <- cl_cpos2lbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 0L:(attsize - 1L),
      registry = get_tmp_registry()
    )
    rbound <- cl_cpos2rbound(
      corpus = "REUTERS",
      s_attribute = "places",
      cpos = 0L:(attsize - 1L),
      registry = get_tmp_registry()
    )
    
    m <- matrix(c(unique(lbound), unique(rbound)), ncol = 2L, byrow = FALSE)
    m2 <- RcppCWB::get_region_matrix(
      corpus = "REUTERS",
      s_attribute = "places",
      strucs = 0L:(cl_attribute_size("REUTERS", "places", "s", get_tmp_registry()) - 1L),
      registry = get_tmp_registry()
    )
    expect_identical(m, m2)
    
  }
)
