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

test_that(
  "s_attr_regions",
  {
    m1 <- s_attr_regions("REUTERS", "id")
    m2 <- get_region_matrix(
      corpus = "REUTERS",
      s_attribute = "id",
      strucs = 0L:(cl_attribute_size("REUTERS", attribute = "id", attribute_type = "s") - 1L)
    )
    expect_identical(m1, m2)
  }
)