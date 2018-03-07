#' @title Drive Letter Handling for Windows.
#' @description Auxiliary functions for handling drive letters on Windows systems.
#' @param path a path declaration
#' @param drive_letter a drive letter
#' @name drive letters
#' @rdname path
NULL

#' @export path_get_drive_letter
#' @rdname path
#' @examples 
#' path_get_drive_letter("D:/PATH/TO/REGISTRY")
#' path_get_drive_letter("d:/PATH/TO/REGISTRY")
path_get_drive_letter <- function(path) gsub("^([A-Za-z]?:?).*$", "\\1", path)

#' @export path_remove_drive_letter
#' @rdname path
#' @examples
#' path_remove_drive_letter("D:/PATH/TO/REGISTRY")
#' path_remove_drive_letter("d:/PATH/TO/REGISTRY")
path_remove_drive_letter <- function(path) gsub("^[A-Za-z]?:?(.*)$", "\\1", path)

#' @export path_add_drive_letter
#' @rdname path
path_add_drive_letter <- function(path, drive_letter) file.path(drive_letter, path)