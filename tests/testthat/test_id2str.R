library(RcppCWB)

testthat::context("cwb_id2str")

test_that(
  "id2str",
  {
    token <- cwb_id2str(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry"),
      id = 0L:5L
    )
    expect_equal(token, c("Diamond", "Shamrock", "Corp", "said", "that", "effective"))
  }
)
