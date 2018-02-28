library(RcppCWB)

testthat::context("attribute_size")

test_that(
  "attribute size s-attribute places",
  {
    N <- cl_attribute_size(
      corpus = "REUTERS",
      attribute = "places",
      attribute_type = "s",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
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
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(N, 4050)
  }
)
