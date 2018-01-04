library(RcppCWB)

testthat::context("get_cbow_matrix")

test_that(
  "get_cbow_matrix",
  {
    M <- get_cbow_matrix(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry"),
      matrix = matrix(c(0, 91), nrow = 1),
      window = 5
    )
    expect_equal(dim(M), c(92L, 11L))
  }
)
