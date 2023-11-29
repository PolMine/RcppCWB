library(RcppCWB)
use_tmp_registry()
testthat::context("region_matrix_to_struc_matrix")

test_that(
  "region_matrix_to_struc_matrix",
  {
    use_tmp_registry(pkg = system.file(package = "GermaParl2"))
    
    if ("germaparl2mini" %in% cl_list_corpora()){
      m <- RcppCWB::region_matrix_to_struc_matrix(
        corpus = "GERMAPARL2MINI",
        s_attribute = "ne",
        registry = get_tmp_registry(),
        region_matrix = matrix(c(2770, 2785), ncol = 2, byrow = TRUE)
      )
      
      expect_identical(m[1,1], NA_integer_)
      expect_identical(m[1,2], NA_integer_)
    }
  }
)
