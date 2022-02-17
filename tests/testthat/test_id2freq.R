library(RcppCWB)

testthat::context("cl_id2freq")

test_that(
  "id2freq",
  {
    N <- cl_id2freq(
      corpus = "REUTERS",
      p_attribute = "word",
      id = 15,
      registry = get_tmp_registry()
    )
    expect_equal(N, 78L)
  }
)

test_that(
  "id_to_freq",
  {
    old <- cl_id2freq(
      corpus = "REUTERS",
      p_attribute = "word",
      id = 15,
      registry = get_tmp_registry()
    )
    
    p <- p_attr(corpus = "REUTERS", p_attribute = "word", registry = get_tmp_registry())
    new <- id_to_freq(id = 15L, p_attr = p)
    expect_identical(old, new)
  }
)
