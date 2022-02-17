library(RcppCWB)
use_tmp_registry()
testthat::context("attribute_size")

test_that(
  "attribute size s-attribute places",
  {
    N <- cl_attribute_size(
      corpus = "REUTERS",
      attribute = "places",
      attribute_type = "s",
      registry = get_tmp_registry()
    )
    expect_equal(N, 20)
  }
)

test_that(
  "attribute size p-attribute word",
  {
    N <- cl_attribute_size(
      corpus = "REUTERS",
      attribute = "word",
      attribute_type = "p",
      registry = get_tmp_registry()
    )
    expect_equal(N, 4050)
  }
)

test_that(
  "new workflow for size of p-attribute",
  {
    conventional <- cl_attribute_size(
      corpus = "REUTERS",
      attribute = "word",
      attribute_type = "p",
      registry = get_tmp_registry()
    )
    attr <- p_attr("REUTERS", p_attribute = "word", registry = get_tmp_registry())
    new <- p_attr_size(attr)
    expect_identical(conventional, new)
  }
)

test_that(
  "new workflow for size of s-attribute",
  {
    conventional <- cl_attribute_size(
      corpus = "REUTERS",
      attribute = "id",
      attribute_type = "s",
      registry = get_tmp_registry()
    )
    attr <- s_attr("REUTERS", s_attribute = "id", registry = get_tmp_registry())
    new <- s_attr_size(attr)
    expect_identical(conventional, new)
  }
)