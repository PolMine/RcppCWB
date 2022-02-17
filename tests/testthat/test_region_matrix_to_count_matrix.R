library(RcppCWB)
use_tmp_registry()
testthat::context("region_matrix_to_count_matrix")

test_that(
  "regions_to_count_matrix",
  {
    M <- region_matrix_to_count_matrix(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      matrix = matrix(c(0, 91), nrow = 1)
    )
    expect_equal(M[16,2], 5L)
  }
)
