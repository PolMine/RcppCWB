library(data.table)
library(igraph)


subdirs <- c("cwb/cl", "cwb/cqp")
deps <- lapply(
  subdirs,
  function(subdir){
    c_dir <- file.path("~/Lab/github/RcppCWB/src", subdir)
    files <- Sys.glob(sprintf("%s/*.h", c_dir))
    includes <- lapply(
      files,
      function(f){
        includes <- grep('#include ".*?\\.h"', readLines(f), value = TRUE)
        gsub('#include\\s+"(.*?\\.h)"', "\\1", perl = TRUE, includes)
      }
    )
    names(includes) <- files
    for (i in rev(which(lapply(includes, length) == 0L))) includes[[i]] <- NULL
    includes
  }
)
#names(deps) <- subdirs

deps_unlisted <- unlist(deps, recursive = FALSE)

dt <- rbindlist(lapply(names(deps_unlisted), function(x) data.table(from = x, to = deps_unlisted[[x]])))
dt[, from := gsub("^/Users/andreasblaette/Lab/github/RcppCWB/src/cwb", "\\1", from)]
df <- data.frame(from = basename(dt[[2]]), to = basename(dt[[1]]))
df <- 

g <- igraph::graph_from_data_frame(d = df)


pdf(file = "~/Desktop/includes.pdf")
plot(
  g,
  label.font = 0.1,
  vertex.size = 0,
  edge.arrow.size = 0.4,
  vertex.label.cex = 0.5
)
dev.off()

