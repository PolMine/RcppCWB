.adjustRegistry <- function(registryFile, dataDir){
  if (file.exists(registryFile)){
    registry <- readLines(registryFile)
    home_position <- grep("^HOME.*?$", registry)
    home <- gsub( "^HOME\\s+(.*?)\\s*$", "\\1", registry[home_position], perl = TRUE)
    if (home != dataDir){
      registry[home_position] <- paste("HOME", dataDir, sep = " ")
      info_position <- grep("^INFO.*?$", registry)
      registry[info_position] <- paste("INFO", file.path(dataDir, "info.md"), sep = " ")
      writeLines(registry, con = registryFile, sep = "\n")
    }
  }
}
