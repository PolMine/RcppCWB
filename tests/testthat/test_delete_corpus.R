library(RcppCWB)

testthat::context("cl_delete_corpus")

test_that(
  "cl_delete_corpus",
  {
    cl_attribute_size(
      corpus = "UNGA",
      attribute = "word",
      attribute_type = "p",
      registry = Sys.getenv("CORPUS_REGISTRY")
    )
    expect_true(corpus_is_loaded("UNGA"))
    del_1 <- cl_delete_corpus("UNGA")
    expect_identical(del_1, 1L)
    expect_false(corpus_is_loaded("UNGA"))
    del_2 <- cl_delete_corpus("UNGA")
    expect_identical(del_2, 0L)
  }
)
