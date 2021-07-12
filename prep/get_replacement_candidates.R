diffreport <- readLines("~/Lab/github/RcppCWB/prep/gitdiff_cqp.txt")

diffs <- split(
  diffreport,
  cut(1:length(x), breaks = grep("^diff\\s--git", x), include.lowest = TRUE, right = FALSE)
)
names(diffs) <- unname(sapply(diffs, function(x) gsub("^diff\\s--git\\s+a/(.*?)\\s.*?$", "\\1", x[1])))

y <- lapply(
  diffs,
  function(diff){
    candidates <- grep("^+", diff)
    for (i in rev(candidates)){
      if (!grepl("^-", diff[i + 1L])) candidates[-which(candidates == i)]
    }
    lapply(candidates, function(i) c(diff[i], diff[i + 1L]))
  }
)

