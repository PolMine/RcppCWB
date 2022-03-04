library(RcppCWB)
use_tmp_registry()
testthat::context("subcorpus")

test_that(
  "create subcorpus from R",
  {
    skip_on_ci()
    skip_on_cran()
    
    oil_context <- cqp_query("REUTERS", subcorpus = "OIL", query = '[]{3}"oil" []{3}')
    m <- subcorpus_get_ranges(oil_context)
    reuters <- cl_find_corpus("REUTERS", registry = get_tmp_registry())
    p <- matrix_to_subcorpus(subcorpus = "OIL2", corpus = reuters, region_matrix = m)
    expect_true("OIL2" %in% cqp_list_subcorpora("REUTERS"))
    
    x <- cqp_query("REUTERS:OIL2", query = '"crude";', subcorpus = "CRUDEOIL")
    expect_true("CRUDEOIL" %in% cqp_list_subcorpora("REUTERS"))
    
    expect_identical(
      cqp_dump_subcorpus("REUTERS", "CRUDEOIL"),
      subcorpus_get_ranges(x)
    )
    
    cqp_drop_subcorpus("REUTERS:OIL")
    cqp_drop_subcorpus("REUTERS:OIL2")
    cqp_drop_subcorpus("REUTERS:CRUDEOIL")
    
    expect_false("OIL" %in% cqp_list_subcorpora("REUTERS"))
    expect_false("OIL2" %in% cqp_list_subcorpora("REUTERS"))
    expect_false("CRUDEOIL" %in% cqp_list_subcorpora("REUTERS"))
  }
)
