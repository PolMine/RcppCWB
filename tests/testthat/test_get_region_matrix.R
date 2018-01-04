library(RcppCWB)

testthat::context("get_region_matrix")

test_that(
  "get_region_matrix",
  {
    regions <- get_region_matrix(
      corpus = "REUTERS",
      s_attribute = "places",
      strucs = 0L:5L,
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(sum(regions), 6476L)
  }
)

