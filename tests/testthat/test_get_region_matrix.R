library(RcppCWB)
use_tmp_registry()
testthat::context("get_region_matrix")

test_that(
  "get_region_matrix",
  {
    regions <- get_region_matrix(
      corpus = "REUTERS",
      s_attribute = "places",
      strucs = 0L:5L,
      registry = get_tmp_registry()
    )
    expect_equal(sum(regions), 6476L)
  }
)

test_that(
  "NA for negative values",
  {
    regions <- get_region_matrix(
      corpus = "REUTERS",
      registry = RcppCWB::corpus_registry_dir("REUTERS")[[1]],
      s_attribute = "id",
      strucs = c(-1, 0:2)
    )
    
    expect_identical(regions[1,1], NA_integer_)
    expect_identical(regions[1,2], NA_integer_)
    
  }
)