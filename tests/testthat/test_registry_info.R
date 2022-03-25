library(RcppCWB)
use_tmp_registry()
testthat::context("subcorpus")

test_that(
  "check result of corpus_data_dir()",
  {
    expect_true(
      dir.exists(
        corpus_data_dir("REUTERS", registry = get_tmp_registry())
      )
    )
  }
)

test_that(
  "check result of corpus_info_file()",
  {
    f <- corpus_info_file("REUTERS", registry = get_tmp_registry())
    expect_true(file.exists(f))
    expect_identical(length(readLines(f)), 8L)
  }
)

test_that(
  "check result of corpus_info_file()",
  {
    name <- corpus_full_name("REUTERS", registry = get_tmp_registry())
    expect_identical(name, "Reuters Sample Corpus")
  }
)


corpus_full_name("REUTERS", registry = get_tmp_registry())
