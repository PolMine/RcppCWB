diffreport <- readLines("~/Lab/github/RcppCWB/prep/gitdiff_cqp.txt")

diffs <- split(
  diffreport,
  cut(1:length(diffreport), breaks = grep("^diff\\s--git", diffreport), include.lowest = TRUE, right = FALSE)
)
names(diffs) <- unname(sapply(diffs, function(x) gsub("^diff\\s--git\\s+a/(.*?)\\s.*?$", "\\1", x[1])))

y <- lapply(
  1L:length(diffs),
  function(d){
    diff <- diffs[[d]]
    candidates <- grep("^\\-", diff)
    for (i in rev(candidates)){
      if (!grepl("^\\+", diff[i + 1L])) candidates <- candidates[-which(candidates == i)]
    }
    lapply(candidates, function(i) sprintf('"%s" = list("^(\\\\s*)%s", "\\\\1%s", 1),', names(diffs)[d], gsub("^\\-\\s*", "", diff[i]), gsub("^\\+\\s*", "", diff[i + 1L] )))
  }
)

cat(paste(unlist(y), collapse = "\n"))