global_replacements <- list(
  c("(vf|f|v)printf\\s*\\(\\s*(stderr|stream|stdout|outfd|fd|File|rd->stream|redir->stream),\\s*", "Rprintf("),
  c("YY(F|D)PRINTF\\s*(\\({1,2})\\s*(stderr|yyoutput),\\s*" , "YY\\1PRINTF \\2"),
  c("fprintf\\s*\\(", "Rprintf("),
  c("(\\s+)printf\\(", "\\1Rprintf("),
  c("#(\\s*)define\\sYYFPRINTF\\sfprintf", "#\\1define YYFPRINTF Rprintf"),
  c('^\\s*#include\\s+"endian\\.h"\\s*$', '#include "endian2.h"') # only files in cl, maybe limit this
)
