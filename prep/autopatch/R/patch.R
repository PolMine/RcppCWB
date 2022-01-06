patch_functions <- list(
  
  delete_line_before = function(code, action){
    which_position <- if (length(action) > 1L) action[[2]] else 1L
    times <- if (length(action) == 3L) action[[3]] else 1L
    
    message(
      sprintf("regex: %s | match: %d | lines: %d ... ", action[[1]], which_position, times),
      appendLF = FALSE
    )
    
    position <- grep(pattern = action[[1]], code)[which_position]
    
    if (!is.na(position)){
      code <- code[-(position - 1L:times)]
      message("OK")
    } else {
      message("FAIL - no match")
    }
    code
  },
  
  insert_before = function(code, action){
    which_position <- if (length(action) > 2L) action[[3]] else 1
    
    message(
      sprintf(
        "regex: %s | match: %d | insertion: %s ... ", action[[1]], which_position, paste(action[[2]], collapse = "///")
      ),
      appendLF = FALSE
    )
    
    position <- grep(pattern = action[[1]], code)[which_position]
    if (!is.na(position)){
      code <- c(
        code[1L:(position - 1L)],
        action[[2]],
        code[position:length(code)]
      )
      message("OK")
    } else {
      message("FAIL - no matches for regex")
    }
    code
  },
  
  insert_after = function(code, action){
    which_position <- if (length(action) > 2L) action[[3]] else 1L
    
    message(
      sprintf("regex: %s | match: %d | insertion: %s ... ", action[[1]], which_position, paste(action[[2]], collapse = "///")),
      appendLF = FALSE
    )
    
    position <- grep(pattern = action[[1]], code)[which_position]
    if (!is.na(position)){
      code <- c(
        code[1L:position],
        action[[2]],
        code[(position + 1L):length(code)]
      )
      message("OK")
    } else {
      message("FAIL - no match")
    }
    code
  },
  
  replace = function(code, action){
    message(
      sprintf("regex: %s | match: %d | replacement: %s ... ", action[[1]], action[[3]], paste(action[[2]], collapse = "///")),
      appendLF = FALSE
    )
    position <- grep(pattern = action[[1]], code)[ action[[3]] ]
    if (!is.na(position)){
      code[position] <- gsub(action[1], action[2], code[position])
      message("OK")
    } else {
      message("FAIL - no match")
    }
    code
  },
  
  remove_lines = function(code, action){
    message(
      sprintf("regex: %s | match: %d ... ", action[[1]], action[[2]]),
      appendLF = FALSE
    )
    
    position <- grep(pattern = action[[1]], code)[ action[[2]] ]
    if (!is.na(position)){
      code <- code[-position]
      message("OK")
    } else {
      message("FAIL - no match")
    }
    code
  },
  
  extern = function(code, action){
    for (ext in action){
      matches <- which(startsWith(code, ext))
      if (length(matches) > 0L){
        for (position in matches){
          code[position] <- paste("extern", code[position], sep = " ")
        }
        message(sprintf("var to extern: %s | n_matches: %d", ext, length(matches)))
      } else {
        message("FAIL - no matches for var: ", ext)
      }
    }
    code
  }
  
)


patch_file <- function(file, actions){
  if (!file.exists(file)){
    warning(sprintf("FAIL - file %s does not exist", file))
    return(FALSE)
  } else {
    code <- readLines(file)
    for (i in 1L:length(actions)){
      message("Action to perform: ", names(actions)[i])
      code <- patch_functions[[ names(actions)[i] ]](code = code, action = actions[[i]])
    }
    writeLines(code, file)
    return(TRUE)
  }
}