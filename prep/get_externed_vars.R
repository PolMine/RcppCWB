diffreport <- readLines("~/Lab/github/RcppCWB/prep/gitdiff_cqp.txt")

diffs <- split(
  diffreport,
  cut(1:length(x), breaks = grep("^diff\\s--git", x), include.lowest = TRUE, right = FALSE)
)
names(diffs) <- unname(sapply(diffs, function(x) gsub("^diff\\s--git\\s+a/(.*?)\\s.*?$", "\\1", x[1])))

get_to_extern <- function(x){
  externs <- gsub("^\\+\\s*", "", grep("^\\+\\s*extern\\s", x, value = TRUE), perl = TRUE)
  extern_candidates <- gsub("^-", "", grep("^-", x, value = TRUE))
  
  unname(sapply(
    externs,
    function(new_extern){
      extern_min <- gsub("extern\\s+", "", new_extern)
      if (extern_min %in% extern_candidates){
        y <- extern_candidates[which(extern_min == extern_candidates)]
        gsub("^(.*?);.*?$", "\\1;", y)
      } else {
        character()
      }
    }
  ))
}

foo <- lapply(diffs, get_to_extern)
for (i in rev(which(sapply(foo, length) == 0))) foo[[i]] <- NULL
