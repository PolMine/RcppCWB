/* 
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 * 
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */


/* included by AB to ensure that winsock2.h is included before windows.h */
#ifdef __MINGW__
#include <winsock2.h> /* AB reversed order, in original CWB code windows.h is included first */
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>

#include <sys/types.h>
#include <time.h>

#include "../cl/globals.h"
#include "../cl/macros.h"
#include "../cl/storage.h"      /* NwriteInt() & NwriteInts() */
/* #include "../cl/lexhash.h" */
/* byte order conversion functions taken from Corpus Library */
#include "../cl/endian.h"


void Rprintf(const char *, ...); /* alternative to include R_ext/Print.h */

/* ---------------------------------------------------------------------- */

#define UMASK              0644

#define UNDEF_VALUE "__UNDEF__"
#define FIELDSEPS  "\t\n"

/* max. number of s-attributes (-> could change this to implementation as a linked list) */
#define MAXRANGES 1024

/* nr of buckets of lexhashes used for checking duplicate errors (undeclared element and attribute names in XML tags) */
#define REP_CHECK_LEXHASH_SIZE 1000

/* if we have XML tags with attributes, input lines can become pretty long ... */
#define MAX_INPUT_LINE_LENGTH  16384

/* implicit knowledge about CL component files naming conventions */
#define STRUC_RNG  "%s/%s.rng"
#define STRUC_AVX  "%s/%s.avx"
#define STRUC_AVS  "%s/%s.avs"
#define POS_CORPUS "%s/%s.corpus"
#define POS_LEX    "%s/%s.lexicon"
#define POS_LEXIDX "%s/%s.lexicon.idx"



/* ---------------------------------------------------------------------- */

extern char *field_separators; FIELDSEPS;
extern char *undef_value;
int debug = 0;
int silent = 0;
extern int verbose;		/* show progress (this is _not_ the opposite of silent!) */
extern int xml_aware;		/* substitute XML entities in p-attributes & ignore <? and <! lines */
extern int skip_empty_lines;
int line = 0;			/* corpus position currently being encoded (i.e. cpos of _next_ token) */
extern int strip_blanks;		/* strip leading and trailing blanks from input and token annotations */
extern cl_string_list input_files; /* list of input file (-f option(s)) */
extern int nr_input_files;		/* number of input files (length of list after option processing) */
extern int current_input_file;	/* index of input file currently being processed */
extern char *current_input_file_name; /* filename of current input file, for error messages */
extern FILE *input_fd;		/* file handle for current input file (or pipe) */
int input_file_is_pipe = 0;	/* so we can properly close input_fd using either fclose() or pclose() */
int input_line = 0;		/* input line number (reset for each new file) for error messages */
extern char *registry_file;	/* if set, auto-generate registry file named <registry_file>, listing declared attributes */
extern char *directory;		/* corpus data directory */


/* ---------------------------------------------------------------------- */

typedef struct _Range {
  char *dir;			/* directory where this range is stored */
  char *name;			/* range name */

  int in_registry;		/* with "-R <reg_file>", this is set to 1 when the attribute is written to the registry (avoid duplicates) */

  int store_values;		/* flag indicating whether to store values (does _not_ automatically apply to children, see below) */
  int null_attribute;		/* a NULL attribute ignores all corresponding XML tags, without checking struture or annotations */
  int automatic;		/* automatic attributes are the 'children' used for recursion and element attributes below  */

  FILE *fd;			/* fd of .rng component */
  FILE *avx;			/* fd of .avx component (the attribute value index) */
  FILE *avs;			/* fd of .avs component (the attribute values) */
  int offset;			/* string offset for next string (in .avs component) */

  cl_lexhash lh;		/* lexicon hash for attribute values */

  int has_children;		/* whether attribute values of XML elements are stored in s-attribute 'children' */
  cl_lexhash el_attributes;	/* maps XML element attribute names to the appropriate s-attribute 'children' (Range *) */
  cl_string_list el_atts_list;	/* list of declared element attribute names, required by close_range() function */
  cl_lexhash el_undeclared_attributes; /* remember undeclared elment attributes, so warnings will be issued only once */

  int max_recursion;		/* maximum auto-recursion level, 0 = no recursion (maximal regions), -1 = assume flat structure */
  int recursion_level;		/* keeps track of level of embedding when auto-recursion is activated */
  int element_drop_count;	/* count how many recursive subelements where dropped because of the max_recursion limit */
  struct _Range **recursion_children;	/* (usually very short) list of s-attribute 'children' for auto-recursion; recursion_children[0] points to self! */

  int is_open;			/* whether there is an open structure at the moment */
  int start_pos;		/* if this.is_open, remember start position of current range */
  char *annot;			/* ... and annotation (if there is one) */

  int num;			/* number of current (if this.is_open) or next structure */

} Range;

Range ranges[MAXRANGES];
int range_ptr = 0;

typedef struct {
  char *name;
  cl_lexhash lh;
  int position;
  FILE *lex_fd;
  FILE *lexidx_fd;
  FILE *corpus_fd;
} WAttr;

WAttr wattrs[MAXRANGES];
int wattr_ptr = 0;

/* ---------------------------------------------------------------------- */

/* lookup hash for undeclared s-attributes and s-attributes declared with -S that 
   have annotations (which will be ignored), so warnings are issued only once */
cl_lexhash undeclared_sattrs = NULL; 

/* ---------------------------------------------------------------------- */

char *progname = NULL;

/* ---------------------------------------------------------------------- */

/* ======================================== helper functions */


/* replacement for strtok() function which doesn't skip empty fields */
char *
my_strtok(register char *s, register const char *delim) {
  register char *spanp;
  register int c, sc;
  char *tok;
  static char *last;
  

  if (s == NULL && (s = last) == NULL)
    return (NULL);
  
  c = *s++;

  if (c == 0) {		/* no non-delimiter characters */
    last = NULL;
    return (NULL);
  }
  tok = s - 1;
  
  for (;;) {
    spanp = (char *)delim;
    do {
      if ((sc = *spanp++) == c) {
	if (c == 0)
	  s = NULL;
	else
	  s[-1] = 0;
	last = s;
	return (tok);
      }
    } while (sc != 0);
    c = *s++;
  }
  /* NOTREACHED */
}

/* decode pre-defined XML entities in string s; overwrites input string s; returns s for convenience;
   (entities are &lt; &gt; &amp; &quot; &apos;) */
char *
decode_entities(char *s) {
  char *read, *write;
  if (s != NULL) {
    read = write = s;
    while (*read) {
      if (*read == '&') {
	if (strncmp(read, "&lt;", 4) == 0) {
	  *(write++) = '<';
	  read += 4;
	}
	else if (strncmp(read, "&gt;", 4) == 0) {
	  *(write++) = '>';
	  read += 4;
	}
	else if (strncmp(read, "&amp;", 5) == 0) {
	  *(write++) = '&';
	  read += 5;
	}
	else if (strncmp(read, "&quot;", 6) == 0) {
	  *(write++) = '"';
	  read += 6;
	}
	else if (strncmp(read, "&apos;", 6) == 0) {
	  *(write++) = '\'';
	  read += 6;
	}
	else {
	  *(write++) = *(read++); /* no known entity after all  */
	}
      }
      else {
	*(write++) = *(read++);	/* simply copy char */
      }
    }
    *write = '\0';		/* terminate result string */
  }
  return s;
}



/* ======================================== print time */

#include <sys/types.h>
#include <sys/time.h>

void
printtime(FILE *stream, char *msg) {
  time_t now;

  (void) time(&now);

  Rprintf("%s: %s\n", msg, ctime(&now));
}


/* ======================================== print usage message and exit */

void 
usage() {
  Rprintf("\n");
  Rprintf("Usage:  %s -f <file> [options] [flags]\n", progname);
  Rprintf("        ... | %s [options] [flags]\n\n", progname);
  Rprintf("Reads verticalised text from stdin (or an input file; -f option) and converts it\n");
  Rprintf("to the CWB binary format. Each TAB-separated column is encoded as a separate\n");
  Rprintf("p-attribute. The first p-attribute is named \"word\" (unless changed with -p),\n");
  Rprintf("additional columns must be declared with -P flags. S-attributes can be\n");
  Rprintf("declared with -S (without annotations) or -V (with annotations) flags. In\n");
  Rprintf("the input data, they must appear as opening and closing XML tags on separate\n");
  Rprintf("lines. For each encoded attribute, one or more data files are created in the\n");
  Rprintf("current directory (or any directory specified with -d). After encoding, use\n");
  Rprintf("cwb-makeall to create the required index files and frequency lists, then compress\n");
  Rprintf("them with cwb-huffcode and cwb-compress-rdx (or preferably use the cwb-make\n");
  Rprintf("program from the CWB/Perl interface).\n\n");
  Rprintf("NB: If you re-encode an existing corpus, be sure to delete all old data files,\n");
  Rprintf("in particular the index and any compressed data files, before running cwb-encode!\n");
  Rprintf("\n");
  Rprintf("Flags:\n");
  Rprintf("  -p <att>  change name of default p-attribute from \"word\" to <att>\n");
  Rprintf("  -p -      no default p-attribute (all must be declared with -P)\n");
  Rprintf("  -P <att>  declare additional p-attribute <att>\n");
  Rprintf("  -S <att>  declare s-attribute <att> without annotations\n");
  Rprintf("  -V <att>  declare s-attribute <att> with annotations\n");
  Rprintf("  -0 <att>  declare null s-attribute <att> (discards tags)\n\n");
  Rprintf("Options:\n");
  Rprintf("  -f <file> read input from <file> [default: stdin]\n");
  Rprintf("  -t <file> [same as -f <file> for backward compatibility]\n");
  Rprintf("  -d <dir>  directory for data files created by %s\n", progname);
  /* uncomment the following lines when (if...) the -C and -r flags are implemented */
/*    Rprintf("  -C <id>   (re-)encode corpus <id> (using data path from registry)\n"); */
/*    Rprintf("  -r <dir>  set registry directory (for -C flag)\n"); */
  Rprintf("  -R <rf>   create registry entry (named <rf>) listing all encoded attributes\n");
  Rprintf("  -B        strip leading/trailing blanks from (input lines & token annotations)\n");
  Rprintf("  -x        XML-aware (replace XML entities and ignore <!.. and <?..)\n");
  Rprintf("  -s        skip empty lines in input data (recommended)\n");
  Rprintf("  -U <str>  insert <str> for missing columns [default: \"%s\"]\n", undef_value);
  Rprintf("  -b <n>    number of buckets in lexicon hash tables\n");
  Rprintf("  -v        verbose mode (show progress messages while encoding)\n");
  Rprintf("  -q        quiet mode (suppresses most warnings)\n");
  Rprintf("  -D        debug mode (quiet, sorry, quite the opposite :-)\n");
  Rprintf("  -h        this help page\n\n");
  Rprintf("Part of the IMS Open Corpus Workbench v" VERSION "\n\n");
  exit(2);
}



/* ======================================== print error message and exit */

/* print input line (and input file, if applicable) on stderr, for error messages and warnings */
void
print_input_line(void) {
  if ((nr_input_files > 0) && (current_input_file_name != NULL)) {
    Rprintf("file %s, line #%d", current_input_file_name, input_line);
  }
  else {
    Rprintf("input line #%d", input_line);
  }  
}

void
error(char *format, ...) {
  va_list ap;
  va_start(ap, format);

  if (format != NULL) {
    Rprintf(format, ap);
    Rprintf("\n");
  }
  else {
    Rprintf("Internal error. Aborted.\n");
  }
  if ((input_line > 0) || (current_input_file > 0)) {
    /* show location only if we've already been reading input */
    Rprintf("[location of error: ");
    print_input_line();
    Rprintf("]\n");
  }
  return 1;
}

/* =================================================== handling s-attributes and p-attributes */

int 
find_range(char *name) {
  int i;

  for (i = 0; i < range_ptr; i++)
    if (strcmp(ranges[i].name, name) == 0)
      return i;
  return -1;
}

void
print_range_registry_line(Range *rng, FILE *fd, int print_comment) {
  Range *child;
  int i, n_atts;

  if (rng->in_registry)
    return;
  else 
    rng->in_registry = 1;		/* make a note that we've already handled the range */

  if (! rng->null_attribute) {

    if (print_comment) {
      /* print comment showing corresponding XML tags */
      Rprintf("# <%s", rng->name);
      if (rng->has_children) {	/* if there are element attributes, show them in the order of declaration */
	n_atts = cl_string_list_size(rng->el_atts_list);
	for (i = 0; i < n_atts; i++) {
	  Rprintf(" %s=\"..\"", cl_string_list_get(rng->el_atts_list, i));
	}
      }
      Rprintf("> ... </%s>\n", rng->name);
      /* print comment showing hierarchical structure (if not flat) */
      if (rng->max_recursion == 0) {
	Rprintf("# (no recursive embedding allowed)\n");
      }
      else if (rng->max_recursion > 0) {
	n_atts = rng->max_recursion;
	Rprintf("# (%d levels of embedding: <%s>", n_atts, rng->name);
	for (i = 1; i <= n_atts; i++)
	  Rprintf(", <%s>", rng->recursion_children[i]->name);
	Rprintf(").\n");
      }
    }

    /* print registry line for this s-attribute */
    if (rng->store_values)
      Rprintf("STRUCTURE %-20s # [annotations]\n", rng->name);
    else
      Rprintf("STRUCTURE %s\n", rng->name);
    
    /* print recursion children, then element attribute children */
    if (rng->max_recursion > 0) {
      n_atts = rng->max_recursion;
      for (i = 1; i <= n_atts; i++)
	print_range_registry_line(rng->recursion_children[i], fd, 0);
    }
    if (rng->has_children) {	/* element attribute children will print their recursion children as well */
      n_atts = cl_string_list_size(rng->el_atts_list);
      for (i = 0; i < n_atts; i++) {
	cl_lexhash_entry entry = cl_lexhash_find(rng->el_attributes,
						 cl_string_list_get(rng->el_atts_list, i));
	child = (Range *) entry->data.pointer;
	print_range_registry_line(child, fd, 0);
      }
    }
    
    if (print_comment)		/* print blank line after each att. declaration block headed by comment */
      Rprintf("\n");
  }
}

Range *
declare_range(char *name, char *directory, int store_values, int null_attribute) {
  char buf[MAX_LINE_LENGTH];
  Range *rng;
  char *p, *rec, *ea_start, *ea;
  cl_lexhash_entry entry;
  int i;

  if (debug)
    Rprintf("ATT: -%s %s\n", (store_values) ? "V":"S", name);

  if (range_ptr >= MAXRANGES) {
    error("Too many s-attributes declared (last was <%s>).", name);
  }

  rng = &ranges[range_ptr];	/* fill next entry in ranges[] */
  range_ptr++;			/* must increment range pointer now, in case we have children */

  strcpy(buf, name);
  /* check if recursion and/or element attributes are declared */
  if ((rec = strchr(buf, ':')) != NULL) {    /* recursion declaration ":<n>"  */
    *(rec++) = '\0';
    if (strchr(buf, '+'))	/* make sure recursion is declared _before_ element attributes */
      error("Usage error: recursion must be declared first in -S %s !", name);
  }
  p = (rec != NULL) ? rec : buf; /* start looking for element attribute declarations from here */
  if ((ea_start = strchr(p, '+')) != NULL) { /* element att. declaration "+<ea>" */
    *(ea_start++) = '\0';
  }
  /* now buf points to <name> rec points to <n> and ea_start to <ea> of the first element att.;
     all strings are NUL-terminated (ea_start has the form "<ea1>+<ea2>+...+<ea_n>" */

  rng->name = cl_strdup(buf);	/* name of the s-attribute */
  rng->dir = cl_strdup(directory);
  rng->in_registry = 0;
  rng->store_values = store_values;
  rng->max_recursion = (rec) ? atoi(rec) : -1; /* set recursion depth: -1 = flat structure */
  rng->recursion_level = 0;
  rng->automatic = 0;

  if (null_attribute) {
    rng->null_attribute = 1;
    return rng;			/* stop initialisation here; other functions shouldn't do anything with this att */
  }
  else {
    rng->null_attribute = 0;
  }

  if (ea_start != NULL)
    ea_start = cl_strdup(ea_start); /* now buf can be re-used for pathnames below */

  /* open data files for this s-attribute (children will be added later) */
  /* create .rng component */
  sprintf(buf, STRUC_RNG, directory, rng->name);
  if ((rng->fd = fopen(buf, "w")) == NULL) {
    perror(buf);
    error("Can't write .rng file for s-attribute <%s>.", name);
  }
  if (rng->store_values) {
    /* create .avx and .avs components and initialise lexicon hash */
    sprintf(buf, STRUC_AVS, rng->dir, rng->name);
    if ((rng->avs = fopen(buf, "w")) == NULL) {
      perror(buf);
      error("Can't write .avs file for s-attribute <%s>.", name);
    }

    sprintf(buf, STRUC_AVX, rng->dir, rng->name);
    if ((rng->avx = fopen(buf, "w")) == NULL) {
      perror(buf);
      error("Can't write .avx file for s-attribute <%s>.", name);
    }

    rng->lh = cl_new_lexhash(10000);	/* typically, will only have moderate number of entries -> save memory */
  }
  else {
    rng->avs = NULL;
    rng->avx = NULL;
    rng->lh = NULL;
  }
  rng->offset = 0;
  rng->is_open = 0;
  rng->start_pos = 0;
  rng->annot = NULL;
  rng->num = 0;

  /* now that the range is intialised, declare its 'children' if necessary */
  /* recursion children */
  if (rng->max_recursion >= 0) {
    rng->recursion_children = (Range **) cl_calloc(rng->max_recursion + 1, sizeof(Range *));
    rng->recursion_children[0] = rng; /* zeroeth recursion level is stored in the att. itself */
    for (i = 1; i <= rng->max_recursion; i++) {
      sprintf(buf, "%s%d", rng->name, i); /* recursion children have 'flat' structure, because recursion is handled explicitly */
      rng->recursion_children[i] = declare_range(buf, rng->dir, rng->store_values, /*null*/ 0);
      rng->recursion_children[i]->automatic = 1; /* mark as automatically handled attribute */
    }
    rng->recursion_level = 0;
    rng->element_drop_count = 0;
  }
  /* element attributes children can handle recursion on their own */
  if (ea_start != NULL) {
    Range *att_ptr;

    rng->has_children = 1;
    rng->el_attributes = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);
    rng->el_atts_list = cl_new_string_list();
    rng->el_undeclared_attributes = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);
    ea = ea_start;
    while (ea != NULL) {
      if ((p = strchr(ea, '+')) != NULL)
	*p = '\0';		/* ea now points to NUL-terminated "<ea_i>" */

      if (rng->max_recursion >= 0) 
	sprintf(buf, "%s_%s:%d", rng->name, ea, rng->max_recursion);
      else
	sprintf(buf, "%s_%s", rng->name, ea);

      if (cl_lexhash_id(rng->el_attributes, ea) >= 0) 
	error("Element attribute <%s %s=...> declared twice!", rng->name, ea);
      entry = cl_lexhash_add(rng->el_attributes, ea);
      att_ptr = declare_range(buf, rng->dir, 1, /*null*/ 0); /* element att. children always store value, of course */
      att_ptr->automatic = 1;	/* mark as automatically handled attribute */
      entry->data.pointer = att_ptr;
      cl_string_list_append(rng->el_atts_list, cl_strdup(ea)); /* make copy of name (for code cleanness) */

      if (p != NULL)
	ea = p + 1 ;
      else
	ea = NULL;		/* end of element att declarations */
    }
    cl_free(ea_start);		/* don't forget to free copy of element att declaration */
  }
  else {
    rng->has_children = 0;
  }

  return rng;
}

void
close_range(Range *rng, int end_pos) {
  cl_lexhash_entry entry;
  int close_this_range = 0;	/* whether we actually have to close this range (may be skipped or delegated in recursion mode) */
  int i, n_children;

  if (rng->null_attribute)	/* do nothing for NULL attributes */
    return;

  if (rng->max_recursion >= 0) {                  /* recursive XML structure */
    rng->recursion_level--;	/* decrement level of nesting */
    if (rng->recursion_level < 0) {
      /* extra close tag (ignored) */
      if (!silent) {
	Rprintf("Surplus </%s> tag ignored (", rng->name);
	print_input_line();
	Rprintf(").\n");
      }
      rng->recursion_level = 0;
    }
    else if (rng->recursion_level > rng->max_recursion) {
      /* deeply nested ranges are ignored */
    }
    else if (rng->recursion_level > 0) { 
      /* delegated to appropriate recursion child */
      close_range(rng->recursion_children[rng->recursion_level], end_pos);
    }
    else {			/* rng->recursion_level == 0, i.e. actually applies to rng */
      close_this_range = 1;
    }
  }
  else {			                  /* flat structure (traditional mode) */
    if (rng->is_open) {
      close_this_range = 1;	/* ok */
    }
    else {
      /* extra close tag (ignored) */
      if (!silent) {
	Rprintf("Close tag </%s> without matching open tag ignored (", rng->name);
	print_input_line();
	Rprintf(").\n");
      }
    }
  }

  /* now close the range and write data to disk if we really have to */
  if (close_this_range) {
    if (end_pos >= rng->start_pos) {
      NwriteInt(rng->start_pos, rng->fd); /* write (start, end) to .rng component */
      NwriteInt(end_pos, rng->fd);
      if (rng->store_values) {
	/* check if annot is already in hash */
	if (rng->annot == NULL) /* shouldn't happen, just to be on the safe side ... */
	  rng->annot = cl_strdup("");
	if ((entry = cl_lexhash_find(rng->lh, rng->annot)) != NULL) {
	  /* annotation is already in hash (and hence, stored in .avs component */
	  int offset = entry->data.integer;
	  /* write (range_num, offset) to .avx component */
	  NwriteInt(rng->num, rng->avx); 
	  NwriteInt(offset, rng->avx);
	}
	else {
	  /* write annotation string to .avs component (at offset rng->offset) */
	  Rprintf(rng->avs, "%s%c", rng->annot, '\0'); 
	  /* write (range_num, current offset) to .avx component */
	  NwriteInt(rng->num, rng->avx);  /* this was intended for 'sparse' annotations, which I don't like (so they're no longer there) */
	  NwriteInt(rng->offset, rng->avx);
	  /* insert annotation string into lexicon hash (with offset_ptr as data ptr) */
	  entry = cl_lexhash_add(rng->lh, rng->annot);
	  entry->data.integer = rng->offset;
	  /* update offset (string length + null byte) */
	  rng->offset += strlen(rng->annot) + 1;
	}
	rng->num++;
	cl_free(rng->annot);
      }
      rng->is_open = 0;
    }
    else {			
      rng->is_open = 0;      /* silently ignore empty region */
      cl_free(rng->annot);
    }
  }

  /* if range has element attribute children, send corresponding close_range() event to all children in the list 
     (recursion and nesting errors will be handled by the children themselves) */
  if (rng->has_children) {
    n_children = cl_string_list_size(rng->el_atts_list);
    for (i = 0; i < n_children; i++) {
      entry = cl_lexhash_find(rng->el_attributes, 
			      cl_string_list_get(rng->el_atts_list, i));
      if (entry == NULL) {
	error("Internal error in <%s>: rng->el_attributes inconsistent with rng->el_atts_list!", rng->name);
      }
      close_range((Range *) entry->data.pointer, end_pos);
    }
  }

  return;
}

/* if rng has element attribute children, open_range() will mess around with the string annot (otherwise not) */
void
open_range(Range *rng, int start_pos, char *annot) {
  cl_lexhash_entry entry;
  int open_this_range = 0;	/* whether we actually have to open this range (may be skipped or delegated in recursion mode) */
  int i, mark, point, n_children;
  char *el_att_name, *el_att_value;
  char quote_char;		/* quote char used for element attribute value ('"' or '\'') */

  if (rng->null_attribute)	/* do nothing for NULL attributes */
    return;

  if (rng->max_recursion >= 0) {                  /* recursive XML structure */
    if (rng->recursion_level > rng->max_recursion) {
      /* deeply nested ranges are ignored; count how many we've lost */
      rng->element_drop_count++;
    }
    else if (rng->recursion_level > 0) {
      /* delegate to appropriate recursion child (with same annotation) */
      open_range(rng->recursion_children[rng->recursion_level], start_pos, 
		 (rng->store_values) ? annot : NULL);
      /* recursion children don't parse the annotation string, so annot will remain untouched;
	 since recursion children always have the same -S or -V behaviour as the parent, we only
         pass the annotation string for -V attributes in order to avoid spurious warnings */
    }
    else {			/* rng->recursion_level == 0, i.e. actually applies to rng */
      open_this_range = 1;
    }
    rng->recursion_level++;	/* increment level of nesting */
  }
  else {			                  /* flat structure (traditional mode) */
    if (rng->is_open) {
      /* if we assume flat structure, implicitly close a range that is already open */
      close_range(rng, start_pos - 1);
    }
    open_this_range = 1;	/* with flat structure, a start tag always opens a new range */
  }

  if (open_this_range) {
    rng->is_open = 1;
    rng->start_pos = line;

    if (annot == NULL)		/* shouldn't happen, but just to be on the safe side ... */
      annot = "";

    if (rng->store_values) {
      rng->annot = cl_strdup(annot); /* remember annotation for close_range(); must strdup because it's pointer into linebuf[] */
      /* don't warn about empty annotations, because that's explicitly allowed! */
      if (strip_blanks) {		/* annotation string may have trailing blanks */
	i = strlen(rng->annot) - 1;
	while ((i >= 0) && ((rng->annot[i] == ' ') || (rng->annot[i] == '\t')))
	  rng->annot[i--] = '\0';
      }
    }
    else {
      /* warn about non-empty annotation string in -S attribute (unless annotation string is parsed), but only once */
      if ((!rng->has_children) && (*annot != '\0')) { 
	if (!cl_lexhash_freq(undeclared_sattrs, rng->name)) {
	  if (!silent) {
	    Rprintf("Annotations of s-attribute <%s> not stored (", rng->name);
	    print_input_line();
	    Rprintf(", warning issued only once).\n");
	  }
	  cl_lexhash_add(undeclared_sattrs, rng->name); /* we can re-use the lookup hash for undeclared s-attributes :o) */
	}
      }
    }
  }
  
  /* if rng has element attribute children, try to parse the annotation string into
     XML attribute="value" or attribute=id pairs (destructively modyfing the original) 
     NB: there must not be any leading whitespace in annot
     NB: we don't bother about recursion here; the child attributes will take care of that themselves */
  if (rng->has_children) {
    /* we have to make sure that regions are opened for all declared element attributes, and that
       no element attribute occurs more than once in order to ensure proper nesting; */
    n_children = cl_string_list_size(rng->el_atts_list); /* use the integer data field of the el_attributes hash */
    for (i = 0; i < n_children; i++) {
      entry = cl_lexhash_find(rng->el_attributes, 
			      cl_string_list_get(rng->el_atts_list, i));
      entry->data.integer = 0;	/* initialise to 0, i.e. "not handled" */
    }

    mark = 0;			/* mark and point are offsets into annot[] */
    while (annot[mark] != '\0') {
      point = mark;

      /* identify XML element attribute name (slightly relaxed attribute naming conventions) */
      while ((annot[point] >= 'A' && annot[point] <= 'Z') ||
	     (annot[point] >= 'a' && annot[point] <= 'z') ||
	     (annot[point] >= '0' && annot[point] <= '9') ||
	     (annot[point] >= (char)0xa0 && annot[point] <= (char)0xff) || /* iso-latin 'extended' characters */
	     (annot[point] == '-') ||
	     (annot[point] == '_')) 
	{
	  point++;
	}
      while ((annot[point] == ' ') || (annot[point] == '\t')) {
	annot[point] = '\0';	/* skip optional whitespace before '=' separator, and remove it from el.att. name */
	point++;
      }

      /* now annot[point] should be the separator '=' char */
      if (annot[point] != '=') {
	if (!silent) {
	  Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (", rng->name);
	  print_input_line();
	  Rprintf(").\n");
	}
	break;			/* stop processing attributes */
      }
      annot[point] = '\0';	/* terminate el. attribute name in el_att_name = (annot+mark) */
      el_att_name = annot + mark;
      mark = point + 1;
      while ((annot[mark] == ' ') || (annot[mark] == '\t'))
	mark++;	/* skip optional whitespace after '=' separator */

      /* now get the attribute value (either "value" or 'value' or id) */
      quote_char = annot[mark];
      if ((quote_char == '"') || (quote_char == '\'')) {    /* attribute="value" or attribute='value' format */
	mark++;			/* assume it's well-formed XML and just look for next occurrence of quote_char */
	point = mark;
	while ((annot[point] != quote_char) && (annot[point] != '\0'))
	  point++;
	if (annot[point] == '\0') { /* syntax error: missing end quote */
	  if (!silent) {
	    Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (", rng->name);
	    print_input_line();
	    Rprintf(").\n");
	  }
	  break;		/* stop processing attributes */
	}
	el_att_value = annot + mark;
	annot[point] = '\0';	/* terminate attribute value, and advance mark */
	mark = point + 1;
      }
      else {			                            /* attribute=id format (accepts same id's as el.att. name) */
	point = mark;
	while ((annot[point] >= 'A' && annot[point] <= 'Z') ||
	       (annot[point] >= 'a' && annot[point] <= 'z') ||
	       (annot[point] >= '0' && annot[point] <= '9') ||
	       (annot[point] >= (char)0xa0 && annot[point] <= (char)0xff) || /* iso-latin 'extended' characters */
	       (annot[point] == '-') ||
	       (annot[point] == '_')) 
	  {
	    point++;
	  }
	el_att_value = annot + mark;
	if (annot[point] == '\0') { /* end of annot[] reached, don't advance mark beyond NUL byte */
	  mark = point;
	}
	else {			/* terminate attribute value, and advance mark */
	  annot[point] = '\0';
	  mark = point + 1;
	}
	if (strlen(el_att_value) == 0) { /* syntax error: attribute=id with empty value (not allowed) */
	  if (!silent) {
	    Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (", rng->name);
	    print_input_line();
	    Rprintf(").\n");
	  }
	  break;		/* stop processing attributes */
	}
      }

      /* syntax check: el_att_name must be non-empty (values "" and '' are allowed) */
      if (strlen(el_att_name) == 0) {
	if (!silent) {
	  Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (", rng->name);
	  print_input_line();
	  Rprintf(").\n");
	}
	break;		/* stop processing attributes */
      }

      /* now delegate the attribute/value pair to the appropriate child attribute */
      entry = cl_lexhash_find(rng->el_attributes, el_att_name);
      if (entry == NULL) {	/* undeclared element attribute (ignored) */
	if (!cl_lexhash_freq(rng->el_undeclared_attributes, el_att_name)) {
	  if (!silent) {
	    Rprintf("Undeclared element attribute <%s %s=...> ignored (",
		    rng->name, el_att_name);
	    print_input_line();
	    Rprintf(", warning issued only once).\n");
	  }
	  cl_lexhash_add(rng->el_undeclared_attributes, el_att_name);
	}
      }
      else {			/* declared element attribute -> decode XML entities in value and delegate to child */
	if (entry->data.integer) {
	  /* attribute already handled, i.e. it must have occurred twice in start tag -> issue warning */
	  if (!silent) {
	    Rprintf("Duplicate attribute value <%s %s=... %s=...> ignored (",
		    rng->name, el_att_name, el_att_name);
	    print_input_line();
	    Rprintf(").\n");
	  }
	}
	else {
	  entry->data.integer = 1; /* mark el. att. as handled */
	  decode_entities(el_att_value);
	  open_range((Range *) entry->data.pointer, start_pos, el_att_value);
	}
      }

      while ((annot[mark] == ' ') || (annot[mark] == '\t'))
	mark++;			/* skip whitespace before next attribute="value" pair */
    }

    /* phew. that was a bit of work; 
       and we still have to make sure that missing element attributes are encoded as empty strings  */
    for (i = 0; i < n_children; i++) {
      entry = cl_lexhash_find(rng->el_attributes, 
			      cl_string_list_get(rng->el_atts_list, i));
      if (entry->data.integer == 0) {
	open_range((Range *) entry->data.pointer, start_pos, "");
      }
    }

  }

  return;
}

int 
find_wattr(char *name) {
  int i;

  for (i = 0; i < wattr_ptr; i++)
    if (strcmp(wattrs[i].name, name) == 0)
      return i;
  return -1;
}

int 
declare_wattr(char *name, char *directory, int nr_buckets) {
  char corname[MAX_LINE_LENGTH];
  char lexname[MAX_LINE_LENGTH];
  char idxname[MAX_LINE_LENGTH];

  if (name == NULL)
    name = "word";

  wattrs[wattr_ptr].name = cl_strdup(name);

  wattrs[wattr_ptr].lh = cl_new_lexhash(nr_buckets);
  
  wattrs[wattr_ptr].position = 0;

  if (directory == NULL)
    directory = ".";

  sprintf(corname, POS_CORPUS, directory, name);
  sprintf(lexname, POS_LEX,    directory, name);
  sprintf(idxname, POS_LEXIDX, directory, name);

  if ((wattrs[wattr_ptr].corpus_fd = fopen(corname, "w")) == NULL) {
    perror(corname);
    error("Can't write .corpus file for %s attribute.", name);
  }

  if ((wattrs[wattr_ptr].lex_fd = fopen(lexname, "w")) == NULL) {
    perror(lexname);
    error("Can't write .lexicon file for %s attribute.", name);
  }

  if ((wattrs[wattr_ptr].lexidx_fd = fopen(idxname, "w")) == NULL) {
    perror(idxname);
    error("Can't write .lexicon.idx file for %s attribute.", name);
  }

  wattr_ptr++;

  return 1;
}

/* ======================================== parse options and set global vars */

void parse_options(int argc, char **argv) {
  int c;
  extern char *optarg;
  extern int optind;

  char *prefix = "word";

  int number_of_buckets = 0;	/* -> use CL default unless changed with -b <n> */
  int first_attr_declared = 0;	/* whether we have already declared the default 'word' attribute (useful for "-p -") */

  while((c = getopt(argc, argv, "p:P:S:V:0:f:t:d:r:C:R:U:Bsb:xvqhD")) != EOF)
    switch(c) {

      /* -B: strip leading and trailing blanks from tokens and annotations */
    case 'B':
      strip_blanks++;
      break;

      /* -v: show progress messages */
    case 'v':
      verbose++;
      break;

      /* -q: suppress warnings (quiet mode) */
    case 'q':
      silent++;
      break;

      /* -x: translate XML entities and ignore declarations & comments */
    case 'x':
      xml_aware++;
      break;

      /* -p <att>: change name of first p-attribute ("-p -": skip first attribute) */
    case 'p':
      if (first_attr_declared)
	error("Usage error: -p option used after -P <att>, or used twice.");
      prefix = optarg;
      if (strcmp(prefix, "-") != 0) {
	declare_wattr(prefix, directory, number_of_buckets);
      }
      first_attr_declared = 1;	/* even if we haven't _really_ declared it because it's "-" */
      break;

      /* -d <dir>: create files in this directory */
    case 'd':
      directory = optarg;
      break;

      /* -r <dir>: change registry directory (for -C option) */
    case 'r':
      error("Sorry, the -r and -C flags haven't been implemented yet.");
      break;

      /* -C <id>: (re-)encode corpus <id>; must be declared in default registry */
    case 'C':
      error("Sorry, the -C flag hasn't been implemented yet.");
      break;

      /* -R <rf>: create registry file named <rf> */
    case 'R':
      if (registry_file != NULL)
	error("Usage error: -R option used twice.");
      else
	registry_file = optarg;
      break;

      /* -t: text input file */
    case 't':
    case 'f':
      cl_string_list_append(input_files, optarg);
      break;
      
      /* -s: skip empty lines */
    case 's': 
      skip_empty_lines++;
      break;

      /* -b <n>: number of buckets */
    case 'b':
      number_of_buckets = atoi(optarg);
      break;

      /* -D: debug mode */
    case 'D':
      debug++;
      break;
      
      /* -S: declare s-attribute without annotations */
    case 'S':
      if (range_ptr < MAXRANGES) {
	if (find_range(optarg) == -1)
	  declare_range(optarg, directory, 0, /*null*/ 0);
	else 
	  error("Usage error: s-attribute <%s> declared twice!", optarg);
      }
      else 
	error("Too many s-attributes (max. %d).", MAXRANGES);
      break;

      /* -V: declare s-attribute with annotations */
    case 'V':
      if (range_ptr < MAXRANGES) {
	if (find_range(optarg) == -1) {
	  declare_range(optarg, directory, /*annot*/ 1, /*null*/ 0); 
	}
	else 
	  error("Usage error: s-attribute <%s> declared twice!", optarg);
      }
      else
	error("Too many s-attributes (max. %d).", MAXRANGES);
      break;

      /* -0: declare NULL s-attribute */
    case '0':
      if (range_ptr < MAXRANGES) {
	if (find_range(optarg) == -1) {
	  declare_range(optarg, directory, /*annot*/ 0, /*null*/ 1); 
	}
	else 
	  error("Usage error: s-attribute <%s> declared twice!", optarg);
      }
      else
	error("Too many s-attributes (max. %d).", MAXRANGES);
      break;

      /* -P: declare additional p-attribute */
    case 'P':

      if (!first_attr_declared)	{ /* no word attribute declared yet */
	declare_wattr(prefix, directory, number_of_buckets);
	first_attr_declared = 1;
      }

      if (wattr_ptr < MAXRANGES) {
	if (find_wattr(optarg) == -1)
	  declare_wattr(optarg, directory, number_of_buckets);
	else 
	  error("Usage error: %s attribute declared twice!", optarg);
      }
      else
	error("Too many p-attributes (max. %d).", MAXRANGES);
      break;

      /* -U: default value for missing columns */
    case 'U':
      undef_value = optarg;
      break;

      /* default or -h: help page */
    case 'h':
    default:
      usage();
      break;
    }
  
  /* if no attributes have been declared, declare the standard attribute */
  if (!first_attr_declared)	/* no word attribute declared yet */
    declare_wattr(prefix, directory, number_of_buckets);


  /* now, check the default and obligatory values */
  if (optind < argc) {
    
    Rprintf("%s:\n  Warning: additional arguments in command ignored:",
	    progname);

    while (optind < argc) {
      Rprintf(" %s", argv[optind]);
      optind++;
    }
    Rprintf("\n  (perhaps you forgot -P, -p, -S, or -V before an attribute name?)\n");
    
  }

}


/* process token data line */
void 
addline(char *str) {
  int fc, id;
  char *field;
  cl_lexhash_entry entry;

  /* the following tokenization code messes around with the containts of linebuf[] in main()! */
  for (field = my_strtok(str, field_separators), fc = 0; 
       fc < wattr_ptr; 
       field = my_strtok(NULL, field_separators), fc++) {
    
    if ((field != NULL) && strip_blanks) { /* need to strip both leading & trailing blanks from field values */
      int len = strlen(field);
      while ((len > 0) && (field[len-1] == ' ')) {
	len--;
	field[len] = '\0';
      }
      while (*field == ' ') 
	field++;
    }
    if ((field != NULL) && (field[0] == '\0'))	/* field == NULL -> missing field; field == "" -> empty field; both inserted as __UNDEF__ */
      field = NULL;

    if ((field != NULL) && xml_aware) 
      decode_entities(field);

    if (field == NULL)		/* mustn't do that before decode_entities(), because undef_value is a constant */
      field = undef_value;

    id = cl_lexhash_id(wattrs[fc].lh, field);
    if (id < 0) {
      /* new entry -> write LEXIDX & LEXICON files */
      NwriteInt(wattrs[fc].position, wattrs[fc].lexidx_fd);
      wattrs[fc].position += strlen(field) + 1;
      if (EOF == fputs(field, wattrs[fc].lex_fd)) {
	perror("fputs() write error");
	error("Error writing .lexicon file for %s attribute.", wattrs[fc].name); 
      }
      if (EOF == putc('\0', wattrs[fc].lex_fd)) {
	perror("putc() write error");
	error("Error writing .lexicon file for %s attribute.", wattrs[fc].name); 
      }
      entry = cl_lexhash_add(wattrs[fc].lh, field);
      id = entry->id;
    }

    NwriteInt(id, wattrs[fc].corpus_fd);
  }
}
 
/* read one input line into buffer (either from stdin, or from one or more input files) 
   returns False when the last input file has been completely read, and automatically closes files */
int
get_input_line(char *buffer, int bufsize) {
  int ok, len;
  char command[MAX_LINE_LENGTH];

  if (nr_input_files == 0) {
    /* read one line of text from stdin */
    ok = (NULL != fgets(buffer, MAX_INPUT_LINE_LENGTH, stdin));
    return ok;
  }
  else {
    if (! input_fd) {
      if (current_input_file >= nr_input_files)
	return 0;
      
      current_input_file_name = cl_string_list_get(input_files, current_input_file);
      len = strlen(current_input_file_name);
      /* if input file has the extension .gz, try opening it with "gzip -cd" */
      if ((len > 3) && (strncasecmp(current_input_file_name + len - 3, ".gz", 3) == 0)) {
	input_file_is_pipe = 1;
	sprintf(command, "gzip -cd %s", current_input_file_name);
	if ((input_fd = popen(command, "r")) == NULL) {
	  perror(command);
	  error("Can't decompress input file %s!", current_input_file_name);
	}
      }
      /* otherwise, open it as a plain text file */
      else {
	input_file_is_pipe = 0;
	if ((input_fd = fopen(current_input_file_name, "r")) == NULL) {
	  perror(current_input_file_name);
	  error("Can't open input file %s!", current_input_file_name);
	}
      }
      
      input_line = 0;
    }

    /* read one line of text from current input file */
    ok = (NULL != fgets(buffer, MAX_INPUT_LINE_LENGTH, input_fd));

    if (! ok) {
      /* assume we're at end of file -> close current input file, and try reading from next one */
      if (input_file_is_pipe) 
	ok = (0 == pclose(input_fd));
      else
	ok = (0 == fclose(input_fd));
      if (! ok) {
	Rprintf("ERROR reading from file %s (ignored).\n", current_input_file_name);
	perror(current_input_file_name);
      }

      input_fd = NULL;		/* use recursive call to open the next input file and read from it */
      current_input_file++;
      ok = get_input_line(buffer, bufsize);
    }

    return ok;
  }
}



/*
 *  main program
 */

     
int 
main(int argc, char **argv) {
  int i, j, k, rng, handled;

  char linebuf[MAX_INPUT_LINE_LENGTH];
  char *buf;			/* 'virtual' buffer; may be advanced to skip leading blanks */
  char separator;
  
  int input_length;		/* length of input line */

  progname = argv[0];		/* initialise global variables */
  input_files = cl_new_string_list();

  parse_options(argc, argv);	/* parse command-line options */
  nr_input_files = cl_string_list_size(input_files);

  if (debug) {
    cl_set_debug_level(1);
    printtime(stderr, "Start");
  }

  line = 0;
  input_line = 0;

  /* lookup hash for (undeclared) structural attributes (inserted as tokens into corpus) */
  undeclared_sattrs = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);

  /* MAIN LOOP: read one line of input and process it */
  while ( get_input_line(linebuf, MAX_INPUT_LINE_LENGTH) ) {  
    if (verbose && (line % 15000 == 0)) {
      Rprintf("%'9dk tokens processed\r", line >> 10);
      fflush(stdout);
    }

    input_line++;
    input_length = strlen(linebuf);
    if (input_length >= (MAX_INPUT_LINE_LENGTH - 1)) { /* buffer filled -> line may have been longer */
      error("Input line too long (max: %d characters).", MAX_INPUT_LINE_LENGTH - 2);
    }

    if (linebuf[input_length-1] == '\n') /* chomp $buf; :o) */
      linebuf[input_length-1] = '\0';

    buf = linebuf;
    if (strip_blanks) {
      while (*buf == ' ')	/* strip leading blanks (trailing blanks will be erased during further processing) */
	buf++;
    }

    if ( (! (skip_empty_lines && (buf[0] == '\0')) ) &&                              /* skip empty lines with -s option */
	 (! (xml_aware && (buf[0] == '<') && ((buf[1] == '?') || (buf[1] == '!'))) ) /* skip XML declarations with -x option */
      ) {
      /* skip empty lines with -s option (for an empty line, first character will usually be newline) */
      handled = 0;

      if (buf[0] == '<') {
	/* XML tag (may be declared or undeclared s-attribute, start or end tag) */
	k = (buf[1] == '/' ? 2 : 1);
	
	/* identify XML element name (according to slightly relaxed attribute naming conventions!) */
	i = k;			
	while ((buf[i] >= 'A' && buf[i] <= 'Z') ||
	       (buf[i] >= 'a' && buf[i] <= 'z') ||
	       (buf[i] >= '0' && buf[i] <= '9') ||
	       (buf[i] >= (char)0xa0 && buf[i] <= (char)0xff) || /* iso-latin 'extended' characters */
	       (buf[i] == '-') ||
	       (buf[i] == '_')) 
	  {
	    i++;
	  }
	/* first non-valid XML element name character must be whitespace or '>' or '/' (for empty XML element) */
	if (! ((buf[i] == ' ') || (buf[i] == '\t') || (buf[i] == '>') || (buf[i] == '/')) ) 
	  i = k;		/* no valid element name found */

	if (i > k) {
	  /* looks like a valid XML tag */
	  separator = buf[i];	/* terminate string containing element name, but remember original char */
	  buf[i] = '\0';	/* so that we can reconstruct the line if we have to insert it literally after all */
	  
	  if ((rng = find_range(&buf[k])) >= 0) {
	    /* good, it's a declared s-attribute and can be handled */
	    handled = 1;

	    if (ranges[rng].automatic) {
	      if (!cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
		Rprintf("explicit XML tag <%s%s> for implicit s-attribute ignored (", 
			(k == 1) ? "" : "/", &buf[k]);
		print_input_line();
		Rprintf(", warning issued only once).\n");
		cl_lexhash_add(undeclared_sattrs, &buf[k]); /* can reuse lexhash for undeclared attributes here */
	      }
	    }
	    else {
	      if (k == 1) {	/* XML start tag */
		i++;		/* identify annotation string, i.e. tag attributes (if there are any) */
		while ((buf[i] == ' ') || (buf[i] == '\t')) /* skip whitespace between element name and first attribute */
		  i++;
		j = i + strlen(buf+i); /* find last '>' character on line */
		while ((j > i) && (buf[j] != '>'))
		  j--;
		buf[j] = '\0';	/* terminate annotation string (if no '>' was found, we have j==i and the annotation string is empty */
		open_range(&ranges[rng], line, buf+i);
	      }
	      else {		/* XML end tag */
		close_range(&ranges[rng], line - 1); /* end tag belongs to previous line! */
	      }
	    }

	  }
	  else {
	    /* no appropriate s-attribute declared -> insert tag literally */
	    if (!silent) {
	      if (!cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
		Rprintf(
			"s-attribute <%s> not declared, inserted literally (", &buf[k]);
		print_input_line();
		Rprintf(", warning issued only once).\n");
		cl_lexhash_add(undeclared_sattrs, &buf[k]);
	      }
	    }
	    buf[i] = separator;	/* restore original line, which will be interpreted as token line */
	  }
	}
	/* malformed XML tag (no element name found) */
	else if (!silent) {
	  Rprintf("Malformed tag %s, inserted literally (", buf);
	  print_input_line();
	  Rprintf(").\n");
	}
      }
      
      /* if we haven't handled the line so far, it must be data for the positional attributes */
      if (!handled) {
	addline(buf);
	line++;			/* line is the corpus position of the next token that will be encoded */
      }
    }
  }
  if (verbose) {
    Rprintf("%50s\r", "");	/* clear progress line */
    Rprintf("Total size: %'d tokens (%.1fM)\n", line, ((float) line) / 1048576);
  }

  /* close open regions at end of input; then close file handles for s-attributes */
  for (i = 0; i < range_ptr; i++) {
    Range *rng = &ranges[i];

    if (! rng->null_attribute) { /* don't attempt to close NULL attribute */
    
      /* This is fairly tricky: When multiple end tags are missing for an attribute declared with recursion (even ":0"),
         we have to call close_range() repeatedly to ensure that the open region at the top level is really closed 
         (which happens when rng->recursion_level == 1). At the same time, close_range() will also close the corresponding
         ranges of any implicitly defined attributes (used to resolve recursive embedding and element attributes).
         Therefore, the following code calls close_range() repeatedly until the current range is actually closed.
         It also relies on the ordering of the ranges[] array, where top level attributes always precede their children,
         so they should be closed automatically before cleanup reaches them. If the ordering were different, children might
         be closed directly at first, and the following attempt to close them automatically from within the close_range() 
	 function would produce highly confusing error messages. To be on the safe side (for some definition of safe :-),
	 we _never_ close ranges for implicit attributes, and issue a warning if they're still open when cleanup reaches them. 
      */
      if (rng->automatic) {	/* implicitly generated s-attributes should have been closed automatically */
	if (!silent && rng->is_open) {
	  Rprintf("Warning: implicit s-attribute <%s> open at end of input (should not have happened).\n",
		  rng->name);
	}
      }
      else {
	if (rng->is_open) {
	  if (rng->recursion_level > 1) 
	    Rprintf("Warning: %d missing </%s> tags inserted at end of input.\n", 
		    rng->recursion_level, rng->name);
	  else
	    Rprintf("Warning: missing </%s> tag inserted at end of input.\n", 
		    rng->name);
	  
	  /* close open region; this will automatically close children from recursion and element attributes;
	     if multiple end tags are missing, we have to call close_range() repeatedly until we reach the top level */
	  while (rng->is_open) { /* should _not_ create an infinite loop, I hope */
	    close_range(rng, line - 1);
	  }
	}

	if (!silent && (rng->max_recursion >= 0) && (rng->element_drop_count > 0)) {
	  Rprintf("%7d <%s> regions dropped because of deep nesting.\n",
		  rng->element_drop_count, rng->name);
	}
      }


      if (EOF == fclose(rng->fd)) {
	perror("fclose() failed");
	error("Error writing .rng file for s-attribute <%s>", rng->name);
      }
      if (rng->store_values) {
	if (EOF == fclose(rng->avs)) {
	  perror("fclose() failed");
	  error("Error writing .avs file for s-attribute <%s>", rng->name);
	}
	if (EOF == fclose(rng->avx)) {
	  perror("fclose() failed");
	  error("Error writing .avx file for s-attribute <%s>", rng->name);
	}
      }

    }
  }

  /* close file handles for positional attributes */
  for (i = 0; i < wattr_ptr; i++) {

    if (EOF == fclose(wattrs[i].lex_fd)) {
      perror("fclose() failed");
      error("Error writing .lexicon file for %s attribute", wattrs[i].name);
    }
    if (EOF == fclose(wattrs[i].lexidx_fd)) {
      perror("fclose() failed");
      error("Error writing .lexicon.idx file for %s attribute", wattrs[i].name);
    }
    if (EOF == fclose(wattrs[i].corpus_fd)) {
      perror("fclose() failed");
      error("Error writing .corpus file for %s attribute", wattrs[i].name);
    }

  }

  /* if registry_file != NULL, write appropriate registry entry to file named <registry_file> */
  if (registry_file != NULL) {
    FILE *registry_fd;
    char *registry_id;		/* use last part of registry filename (i.e. string following last '/' character) */
    char *corpus_name = NULL;	/* name of the corpus == uppercase version of registry_id */

    if (debug)
      Rprintf("Writing registry file %s ...\n", registry_file);

    if ((registry_fd = fopen(registry_file, "w")) == NULL) {
      perror(registry_file);
      error("Can't create registry entry in file %s!", registry_file);
    }

    i = strlen(registry_file) - 1;
    while ((i > 0) && (registry_file[i-1] != '/'))
      i--;
    registry_id = registry_file + i;

    i = strlen(directory) - 1;
    while ((i > 0) && (directory[i] == '/'))
      directory[i--] = '\0';	/* remove trailing '/' from home directory */

    corpus_name = cl_strdup(registry_id);	/* copy registry_id and convert it to uppercase */
    i = strlen(corpus_name) - 1;
    while (i >= 0) {
      corpus_name[i] = toupper((unsigned char) corpus_name[i]);	/* this _might_ handle non-ascii characters, if we're lucky */
      i--;
    }

    /* write header part for registry file */
    Rprintf(registry_fd, "##\n## registry entry for corpus %s\n##\n\n", corpus_name);
    Rprintf(registry_fd, "# long descriptive name for the corpus\n");
    Rprintf(registry_fd, "NAME \"\"\n");
    Rprintf(registry_fd, "# corpus ID (must be lowercase in registry!)\n");
    Rprintf(registry_fd, "ID   %s\n", registry_id);
    Rprintf(registry_fd, "# path to binary data files\n");
    Rprintf(registry_fd, "HOME %s\n", directory);
    Rprintf(registry_fd, "# optional info file (displayed by \"info;\" command in CQP)\n");
    Rprintf(registry_fd, "INFO %s/.info\n\n", directory);
    Rprintf(registry_fd, "# corpus properties provide additional information about the corpus:\n");
    Rprintf(registry_fd, "##:: charset  = \"latin1\" # change if your corpus uses different charset\n");
    Rprintf(registry_fd, "##:: language = \"??\"     # insert ISO code for language (de, en, fr, ...)\n");
    Rprintf(registry_fd, "\n\n");

    /* insert p-attributes into registry file */
    Rprintf(registry_fd, "##\n## p-attributes (token annotations)\n##\n\n");
    for (i = 0; i < wattr_ptr; i++) {
      Rprintf(registry_fd, "ATTRIBUTE %s\n", wattrs[i].name);
    }
    Rprintf(registry_fd, "\n\n");

    /* insert s-attributes into registry file */
    Rprintf(registry_fd, "##\n## s-attributes (structural markup)\n##\n\n");
    for (i = 0; i < range_ptr; i++) {
      print_range_registry_line(&ranges[i], registry_fd, 1);
    }
    Rprintf(registry_fd, "\n");
    Rprintf(registry_fd, "# Yours sincerely, the Encode tool.\n");

    fclose(registry_fd);

    cl_free(corpus_name);
  }

  if (debug)
    printtime(stderr, "Done");

  exit(0);
}
