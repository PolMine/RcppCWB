registry_dir <- system.file(package = "RcppCWB", "extdata", "cwb", "registry")
no_tokens <- cl_attribute_size(
   "REUTERS", attribute = "word", attribute_type = "p",
   registry = registry_dir
   )
ids <- cl_cpos2id(
 "REUTERS", p_attribute = "word",
   registry = registry_dir, cpos = 0:(no_tokens - 1)
 )
tokens <- cl_id2str(
 "REUTERS", p_attribute = "word",
   registry = registry_dir, id = ids
)
writeLines(tokens, "~/Lab/github/RcppCWB/inst/extdata/examples/reuters.txt")
