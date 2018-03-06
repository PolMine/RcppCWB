library(RcppCWB)

testthat::context("s_attribute_decode")

test_that(
  "s_attribute_decode",
  {
    sAttrDF <- s_attribute_decode(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry"),
      method = "Rcpp"
    )
    expect_equal(sAttrDF[["values"]][1:4], c("usa", "usa", "canada", "usa"))
    expect_equal(dim(sAttrDF), 20L)
  }
)
