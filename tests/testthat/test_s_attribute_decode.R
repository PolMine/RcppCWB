library(RcppCWB)

testthat::context("s_attribute_decode")

test_that(
  "s_attribute_decode",
  {
    s_attr_df_r <- s_attribute_decode(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = use_tmp_registry(),
      method = "Rcpp"
    )
    expect_equal(s_attr_df_r[["value"]][1:4], c("usa", "usa", "canada", "usa"))
    expect_equal(nrow(s_attr_df_r), 20L)
    
    s_attr_df_rcpp <- s_attribute_decode(
      corpus = "REUTERS",
      s_attribute = "places",
      registry = use_tmp_registry(),
      method = "Rcpp"
    )
    
    expect_identical(s_attr_df_r, s_attr_df_rcpp)
  }
)
