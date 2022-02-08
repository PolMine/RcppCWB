library(RcppCWB)

testthat::context("cl_delete_corpus")

test_that(
  "cl_delete_corpus",
  {
    expect_true(cl_load_corpus("UNGA", registry = Sys.getenv("CORPUS_REGISTRY")))
    print(Sys.getenv("CORPUS_REGISTRY"))
    expect_true(corpus_is_loaded("UNGA"))
    expect_true(cl_delete_corpus("UNGA"))
    expect_false(corpus_is_loaded("UNGA"))
    expect_false(cl_delete_corpus("UNGA"))
  }
)
