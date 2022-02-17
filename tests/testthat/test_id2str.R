library(RcppCWB)
use_tmp_registry()
testthat::context("cl_id2str")

test_that(
  "id2str",
  {
    token <- cl_id2str(
      corpus = "REUTERS",
      p_attribute = "word",
      registry = get_tmp_registry(),
      id = 0L:5L
    )
    expect_equal(token, c("Diamond", "Shamrock", "Corp", "said", "that", "effective"))
  }
)
