library(RcppCWB)

testthat::context("decode_s_attribute")

test_that(
  "lexicon size",
  {
    sAttr <- decode_s_attribute(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
    )
    expect_equal(sAttr[1:4], c("usa", "usa", "canada", "usa"))
    expect_equal(length(sAttr), 20L)
  }
)
