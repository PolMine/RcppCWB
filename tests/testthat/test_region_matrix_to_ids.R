library(RcppCWB)

testthat::context("region_matrix_to_ids")

test_that(
  "regions_to_ids",
  {
    ids <- region_matrix_to_ids(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      matrix = matrix(c(0, 91), nrow = 1)
    )
    expect_equal(tail(ids, n = 6L), c(55L, 56L, 41L, 15L, 57L, 58L))
  }
)

test_that(
  "ranges_to_cpos()",
  {
    r <- get_region_matrix(
      corpus = "REUTERS", registry = get_tmp_registry(),
      s_attribute = "id", strucs = 0:10
    )
    c <- ranges_to_cpos(r)
    alt <- unlist(apply(r, 1, function(row) row[1]:row[2]))
    expect_identical(c, alt)
    
    r[5,2] <- 1
    expect_error(ranges_to_cpos(r))
  }
)