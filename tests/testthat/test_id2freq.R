library(RcppCWB)

testthat::context("cl_id2freq")

test_that(
  "id2freq",
  {
    N <- cl_id2freq(
      corpus = "REUTERS",
      p_attribute = "word",
      id = 15,
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(N, 78L)
  }
)
