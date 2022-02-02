library(RcppCWB)

testthat::context("cwb_encode")

test_that(
  "cwb_encode",
  {
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

test_that(
  "identity of RcppCWB and CWB encoding result",
  {
    testthat::skip_if_not()
    
    utils_zipfile <- tempfile()
    utils_dir <- file.path(tempdir(), "cwb_win-0.0.1", "utils")
    download.file("https://github.com/PolMine/cwb_win/archive/refs/tags/v0.0.1.zip", destfile = utils_zipfile)
    unzip(utils_zipfile, exdir = tempdir())
    
    utils <- Sys.glob(sprintf("%s/*.exe", utils_dir))
    names(utils) <- gsub("\\.exe$", "", basename(utils))
    
    tmp_registry <- file.path(tempdir(), "tmp_registry")
    tmp_data_dir <- file.path(tempdir(), "bt_data_dir")
    tmp_corpus_id <- "BT"
    
    cmd_encode <- sprintf(
      "%s -d %s -xsB -f %s -r %s -P pos -P lemma -S %s -S %s -S %s",
      utils[["cwb-encode"]],
      system.file(package = "RcppCWB", "extdata", "vrt"),
      tmp_data_dir,
      tmp_registry,
      "plenary_protocol:0+lp+protocol_no+date+year+birthday+version+url+filetype",                            
      "speaker:0+id+type+lp+protocol_no+date+year+ai_no+ai_id+ai_type+who+name+parliamentary_group+party+role",
      "p:0+"        
    )
    system(command = cmd_encode)
    
    unlink(tmp_registry)
    unlink(tmp_data_dir)
  }
)


