library(RcppCWB)

testthat::context("struc2str")

test_that(
  "struc2str",
  {
    sAttr <- cl_struc2str(
      corpus = "REUTERS",
      s_attribute = "places",
      struc = 2L,
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(sAttr, "canada")
  }
)
