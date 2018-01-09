library(RcppCWB)

testthat::context("cwb_id2freq")

test_that(
  "id2freq",
  {
    N <- cwb_id2freq(
      corpus = "REUTERS",
      p_attribute = "word",
      id = 15,
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(N, 78L)
  }
)
