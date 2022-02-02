library(RcppCWB)

testthat::context("cwb_encode")

# test_that(
#   "cwb_encode",
#   {
#     data_dir <- file.path(tempdir(), "tmp_data_dir")
#     dir.create(data_dir)
# 
#     cwb_encode(
#       corpus = "BTMIN",
#       registry = Sys.getenv("CORPUS_REGISTRY"),
#       vrt_dir = system.file(package = "RcppCWB", "extdata", "vrt"),
#       data_dir = data_dir,
#       p_attributes = c("word", "pos", "lemma"),
#       s_attributes = list(
#         plenary_protocol = c(
#           "lp", "protocol_no", "date", "year", "birthday", "version",
#           "url", "filetype"
#         ),
#         speaker = c(
#           "id", "type", "lp", "protocol_no", "date", "year", "ai_no", "ai_id",
#           "ai_type", "who", "name", "parliamentary_group", "party", "role"
#          ),
#         p = character()
#       )
#     )
#     
#     cqp_reset_registry(registry = Sys.getenv("CORPUS_REGISTRY"))
#     
#     ids <- cl_cpos2id(corpus = "BTMIN", p_attribute = "word", cpos = 0L:17L)
#     str <- cl_id2str(corpus = "BTMIN", p_attribute = "word", id = ids)
#     
#     expect_equal(str[c(1,3:5)], c("Herr", ",", "ich", "schlage"))
# 
#     unlink(data_dir)
#     unlink(file.path(Sys.getenv("CORPUS_REGISTRY"), "btmin"))
#   }
# )

test_that(
  "identity of RcppCWB and CWB encoding result",
  {
    # testthat::skip_if_not(.Platform$OS.type == "windows")
    
    tmp_registry <- file.path(tempdir(), "tmp_registry")
    dir.create(tmp_registry)
    Sys.setenv(CORPUS_REGISTRY = tmp_registry)
    cqp_reset_registry(tmp_registry)
    
    tmp_registry_file <- fs::path(file.path(tmp_registry, "bt"))
    tmp_data_dir <- file.path(tempdir(), "bt")
    dir.create(tmp_data_dir)
    
    
    # utils_zipfile <- tempfile()
    # utils_dir <- file.path(tempdir(), "cwb_win-0.0.1", "utils")
    # download.file("https://github.com/PolMine/cwb_win/archive/refs/tags/v0.0.1.zip", destfile = utils_zipfile)
    # unzip(utils_zipfile, exdir = tempdir())
    # file.rename(
    #   from = file.path(utils_dir, "libintl-9.dll"),
    #   to = file.path(utils_dir, "libintl-8.dll")
    # )
    # utils <- Sys.glob(sprintf("%s/*.exe", utils_dir))
    # names(utils) <- gsub("\\.exe$", "", basename(utils))
    # 
    #     args <- sprintf(
    #       '%s -d "%s" -c utf8 -xsB -v -D -F "%s" -R "%s" -P pos -S plenary_protocol:0+lp+protocol_no+date+year+birthday+version+url+filetype -S speaker:0+id+type+lp+protocol_no+date+year+ai_no+ai_id+ai_type+who+name+parliamentary_group+party+role -S p:0+',
    #       fs::path(utils[["cwb-encode"]]),
    #       fs::path(tmp_data_dir),
    #       system.file(package = "RcppCWB", "extdata", "vrt"),
    #       fs::path(tmp_registry_file)
    #     )
    # foo <- system(shQuote(args))
    # 
    # for (p_attr in c("word", "pos", "lemma")){
    #   cmd_makeall <- sprintf(
    #     "%s -r %s -P word BT",
    #     fs::path(utils[["cwb-makeall"]]), fs::path(tmp_registry)
    #   )
    #   system(command = cmd_makeall)
    #   
    #   cmd_huffcode <- sprintf(
    #     "%s -r %s -P word BT",
    #     fs::path(utils[["cwb-huffcode"]]), fs::path(tmp_registry)
    #   )
    #   system(command = cmd_huffcode)
    #   
    #   cmd_compress_rdx <- sprintf(
    #     "%s -r %s -P word BT",
    #     fs::path(utils[["cwb-compress-rdx"]]), fs::path(tmp_registry)
    #   )
    #   system(command = cmd_compress_rdx)
    # }
    
    cwb_encode(
      corpus = "BT",
      registry = tmp_registry,
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
      cwb_makeall(corpus = "BT", p_attribute = p_attr, registry = tmp_registry)
      cwb_huffcode(corpus = "BT", p_attribute = p_attr, registry = tmp_registry)
      cwb_compress_rdx(corpus = "BT", p_attribute = p_attr, registry = tmp_registry)
    }
    
    for (p_attr in c("word", "pos", "lemma")){
      expect_equal(
        cl_attribute_size(corpus = "BT", attribute = p_attr, attribute_type = "p", registry = tmp_registry),
        8402
      )
    }
    
    n <- cl_attribute_size(corpus = "BT", attribute = "word", attribute_type = "p", registry = tmp_registry)
    ids <- cl_cpos2id(
      "BT", p_attribute = "word", registry = tmp_registry,
      cpos = 0L:(n-1L)
    )
    
    words <- cl_id2str("BT", p_attribute = "word", registry = tmp_registry, id = ids)
    expect_equal(table(words == "Liebe")[["TRUE"]], 6)
    expect_equal(table(words == "SPD")[["TRUE"]], 31)
    Encoding(words) <- "UTF-8"
    wordfile <- tempfile()
    # cat(words, file = wordfile)
    # expect_equal(unname(tools::md5sum(wordfile)), "c3983b7c7f142692f0d177ffc3079536")

    unlink(tmp_registry)
    unlink(tmp_data_dir)
  }
)
