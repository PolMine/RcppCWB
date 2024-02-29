library(RcppCWB)
use_tmp_registry()
testthat::context("cwb_encode")


test_that(
  "identity of RcppCWB and CWB encoding result",
  {
    
    tmp_data_dir <- file.path(tempdir(), "bt")
    dir.create(tmp_data_dir)
    
    regdir <- get_tmp_registry()

    cwb_encode(
      corpus = "BT",
      registry = regdir,
      vrt_dir = system.file(package = "RcppCWB", "extdata", "vrt"),
      data_dir = tmp_data_dir,
      encoding = "utf8",
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
    
    for (p_attr in c("word", "pos", "lemma")){
      cwb_makeall(corpus = "BT", p_attribute = p_attr, registry = regdir)
      if (.Platform$OS.type != "windows"){
        cwb_huffcode(corpus = "BT", p_attribute = p_attr, registry = regdir)
        cwb_compress_rdx(corpus = "BT", p_attribute = p_attr, registry = regdir)
      }
    }
    
    expect_true(cl_load_corpus(corpus = "BT", registry = regdir))
    expect_true(tolower("BT") %in% cl_list_corpora())
    
    # In the CQP context, corpus IDs are uppered - here we knowingly provide
    # a lowercase ID that is uppered internally #64
    expect_true(cqp_load_corpus(corpus = "bt", registry = regdir))
    expect_true("BT" %in% cqp_list_corpora())
    
    for (p_attr in c("word", "pos", "lemma")){
      expect_equal(
        cl_attribute_size(
          corpus = "BT",
          attribute = p_attr,
          attribute_type = "p",
          registry = regdir
        ),
        8402
      )
    }
    
    n <- cl_attribute_size(
      corpus = "BT",
      attribute = "word",
      attribute_type = "p",
      registry = regdir
    )
    ids <- cl_cpos2id(
      "BT", p_attribute = "word", registry = regdir,
      cpos = 0L:(n-1L)
    )
    
    words <- cl_id2str("BT", p_attribute = "word", registry = regdir, id = ids)
    expect_equal(table(words == "Liebe")[["TRUE"]], 6)
    expect_equal(table(words == "SPD")[["TRUE"]], 31)

    unlink(tmp_data_dir)
  }
)
