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


test_that(
  "check result of corpus_p_attributes()",
  {
    p <- corpus_p_attributes(corpus = "REUTERS", registry = get_tmp_registry())
    expect_identical(p, "word")
  }
)


test_that(
  "check result of corpus_s_attributes()",
  {
    s <- corpus_s_attributes(corpus = "REUTERS", registry = get_tmp_registry())
    expect_identical(s, c("id", "topics_cat", "places", "language"))
  }
)

test_that(
  "check result of corpus_properties()",
  {
    props <- corpus_properties(corpus = "REUTERS", registry = get_tmp_registry())
    expect_identical(props, c("language", "charset"))
  }
)


test_that(
  "check result of corpus_property()",
  {
    lang <- corpus_property(
      corpus = "REUTERS",
      registry = get_tmp_registry(),
      property = "language"
    )
    expect_identical(lang, "en")
    
    charset <- corpus_property(
      corpus = "REUTERS",
      registry = get_tmp_registry(),
      property = "charset"
    )
    expect_identical(charset, "latin1")
    
    na <- corpus_property(
      "REUTERS",
      registry = get_tmp_registry(),
      property = "foo"
    )
    expect_identical(na, NA_character_)
  }
)


test_that(
  "check result of corpus_registry_dir()",
  {
    expect_identical(
      corpus_registry_dir("REUTERS"), get_tmp_registry()
    )
  }
)
