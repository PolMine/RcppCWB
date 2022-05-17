library(RcppCWB)
use_tmp_registry()
testthat::context("cl_delete_corpus")

test_that(
  "cl_delete_corpus",
  {
    expect_true(
      cl_load_corpus("UNGA", registry = get_tmp_registry())
    )
    expect_true(corpus_is_loaded("UNGA", registry = get_tmp_registry()))
    expect_true(cl_delete_corpus("UNGA", registry = get_tmp_registry()))
    expect_false(corpus_is_loaded("UNGA", registry = get_tmp_registry()))
    expect_false(cl_delete_corpus("UNGA", registry = get_tmp_registry()))
  }
)
