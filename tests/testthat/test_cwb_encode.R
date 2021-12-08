library(RcppCWB)

testthat::context("cwb_encode")

test_that(
  "cwb_encode",
  {
    skip_on_os("windows")
    data_dir <- file.path(tempdir(), "tmp_data_dir")
    dir.create(data_dir)

    cwb_encode(
      corpus = "BTMIN",
      registry = Sys.getenv("CORPUS_REGISTRY"),
      vrt_dir = system.file(package = "RcppCWB", "extdata", "vrt"),
      data_dir = data_dir,
      p_attributes = c("word", "pos", "lemma"),
      s_attributes = list(
        plenary_protocol = c(
          "lp", "protocol_no", "date", "year", "birthday", "version",
          "url", "filetype"
        ),
        speaker = c(
          "id", "type", "lp", "protocol_no", "date", "year", "ai_no", "ai_id",
          "ai_type", "who", "name", "parliamentary_group", "party", "role"
         ),
        p = character()
      )
    )
    
    cqp_reset_registry(registry = Sys.getenv("CORPUS_REGISTRY"))
    
    ids <- cl_cpos2id(corpus = "BTMIN", p_attribute = "word", cpos = 0L:17L)
    str <- cl_id2str(corpus = "BTMIN", p_attribute = "word", id = ids)
    
    expect_equal(str[c(1,3:5)], c("Herr", ",", "ich", "schlage"))

    unlink(data_dir)
    unlink(file.path(Sys.getenv("CORPUS_REGISTRY"), "btmin"))
  }
)
