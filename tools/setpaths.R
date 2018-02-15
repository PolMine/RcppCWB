# setpaths.R - last modification: 2018-02-06
# author: Andreas Blaette (andreas.blaette@uni-due.de)
# 
#
# This is a standardized R script to perform the necessary configuration the
# path to the data directory and the info file in the registry files of CWB
# indexed corpora that are included in a R data package upon installing the
# package.
# 
# The registry files are assumed to be in the subdirectory 
# inst/extdata/cwb/registry of the data package. The indexed corpora and the 
# info file are assumed to be in the directory inst/extdata/cwb/indexed_corpora.
# 
# The script should be put in package subdirectory 'tools' of the data package.
# It is assumed to be called from the configure scripts of a R package
# (configure/configure.win).
#
# On Linux/macOS, include a shell script configure with this line:
# ${R_HOME}/bin/Rscript ./tools/setpaths.R --args "$R_PACKAGE_DIR" 
#
# On Windows, include a shell script configure.win with this line:
# ${R_HOME}/bin${R_ARCH_BIN}/Rscript.exe ./tools/setpaths.R --args "$R_PACKAGE_DIR"
# 
# The latest version of this file is part of the ctk package, available
# at www.github.com/PolMine/ctk, in the directory inst/R.



# the directory where the package will be installed is passed into the
# R script as a command line argument
args <- commandArgs(trailingOnly = TRUE)
packageDir <- args[2] 

# this is where the registry directory is before copying everything to the
# final location
registryDir <- file.path(getwd(), "inst", "extdata", "cwb", "registry")

for (corpus in list.files(registryDir)){
  registryFile <- file.path(registryDir, corpus)
  registry <- readLines(registryFile)
  
  homeDir <- file.path(packageDir, "extdata", "cwb", "indexed_corpora", corpus)
  # The CWB tools do not digest the volume declaration that will be part of the 
  # working directory path. It needs to be removed. When the corpus is used, CWB
  # tools will assume the path to point to a directory on the volume of the current
  # working directory.
  if (.Platform$OS.type == "windows") homeDir <- gsub("^[A-Za-z]?:?(.*)$", "\\1", homeDir)
  
  # It causes errors, if the path to the data directory contains whitespace (a
  # typical problem on Windows). Wrapping the path declaration with quotation marks 
  # solves the problem
  if (grepl(" ", homeDir)){
    registry[grep("^HOME", registry)] <- paste("HOME", sprintf("\"%s\"", homeDir), sep = " ")
  } else {
    registry[grep("^HOME", registry)] <- paste("HOME", homeDir , sep = " ")
  }
  
  # adjust statement of info file
  infoFileLine <- grep("^INFO", registry)
  infoFileBasename <- basename(gsub("^INFO\\s+(.*?)$", "\\1", registry[infoFileLine]))
  infoFileNew <- file.path(homeDir, infoFileBasename)
  if (grepl(" ", infoFileNew)) infoFileNew <- sprintf("\"%s\"", infoFileNew)
  registry[infoFileLine] <- paste("INFO", infoFileNew, sep = " ")

  writeLines(text = registry, con = registryFile, sep = "\n")
}
