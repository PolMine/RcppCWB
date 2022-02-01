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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
/* included by AB to ensure that winsock2.h is included before windows.h */
#ifdef __MINGW__
#include <winsock2.h> /* AB reversed order, in original CWB code windows.h is included first */
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <limits.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <glib.h>

#include "../cl/cl.h"
#include "../cl/cwb-globals.h"
#include "../cl/storage.h"      /* for NwriteInt() & NwriteInts() */
#include "../cl/endian2.h"/* for byte order conversion functions */

void Rprintf(const char *, ...); /* alternative to include R_ext/Print.h */

/* ---------------------------------------------------------------------- */

/** User privileges of new files (octal format) */
#define UMASK              0644

/** String containing the characters that can function as field separators */
#define FIELDSEPS  "\t\n"

/** Max number of attributes of a single kind (s or p). */
#define MAX_ATTRIBUTES 1024

/** nr of buckets of lexhashes used for checking duplicate errors (undeclared element and attribute names in XML tags) */
#define REP_CHECK_LEXHASH_SIZE 1000

/** Input buffer size. If we have XML tags with attributes, input lines can become pretty long
 * (but there's basically just a single buffer)
 */
#define MAX_INPUT_LINE_LENGTH  65536

/** Normal extension for CWB input text files. (must have exactly 4 characters; .gz/.bz2 may be added to this if the file is compressed.) */
#define DEFAULT_INFILE_EXTENSION ".vrt"

/* implicit knowledge about CL component files naming conventions: format strings for printf and friends that combine a directory with an attribute name. */
#define PATH_STRUC_RNG  "%s" SUBDIR_SEP_STRING "%s.rng"            /**< Path implementing CL naming convention for S-attribute RNG files */
#define PATH_STRUC_AVX  "%s" SUBDIR_SEP_STRING "%s.avx"            /**< Path implementing CL naming convention for S-attribute AVX (att-val index) files */
#define PATH_STRUC_AVS  "%s" SUBDIR_SEP_STRING "%s.avs"            /**< Path implementing CL naming convention for S-attribute AVS (attribute values) files */
#define PATH_POS_CORPUS "%s" SUBDIR_SEP_STRING "%s.corpus"         /**< Path implementing CL naming convention for P-attribute Corpus files */
#define PATH_POS_LEX    "%s" SUBDIR_SEP_STRING "%s.lexicon"        /**< Path implementing CL naming convention for P-attribute Lexicon files */
#define PATH_POS_LEXIDX "%s" SUBDIR_SEP_STRING "%s.lexicon.idx"    /**< Path implementing CL naming convention for P-attribute Lexicon-index files */


/* ---------------------------------------------------------------------- */

/* global variables representing configuration */

extern char *field_separators;     /**< string containing the characters that can function as field separators */
extern char *undef_value; /**< string used as value of P-attributes when a value is missing,
                                             ie if a tab-delimited field is empty */
extern int debugmode;                          /**< debug mode on or off? */
extern int quietly;                         /**< hide messages */
extern int verbose;                        /**< show progress (this is _not_ the opposite of silent!) */
extern int xml_aware;                      /**< substitute XML entities in p-attributes & ignore <? and <! lines */
extern int skip_empty_lines;               /**< skip empty lines when encoding? */
extern int auto_null;                      /**< auto-declare null attributes for unknown XML tags */
extern unsigned line;                      /**< corpus position currently being encoded (ie cpos of _next_ token);
                                             unsigned so it doesn't wrap after first 2^31 tokens
                                             and thus we can abort encoding when corpus size is exceeded */
extern int strip_blanks;                   /**< strip leading and trailing blanks from input and token annotations */
extern cl_string_list input_files;      /**< list of input file(s) (-f option(s)) */
extern int nr_input_files;                 /**< number of input files (length of list after option processing) */
extern int current_input_file;             /**< index of input file currently being processed */
extern char *current_input_file_name;   /**< filename of current input file, for error messages */
extern FILE *input_fh;                  /**< file handle for current input file (or pipe) (text mode!) */
extern unsigned long input_line;           /**< input line number (reset for each new file) for error messages */
extern char *registry_file;             /**< if set, auto-generate registry file named {registry_file}, listing declared attributes */
extern char *directory;                 /**< corpus data directory (no longer defaults to current directory) */
extern const char *encoding_charset_name;  /**< character set label that is inserted into the registry file */
extern CorpusCharset encoding_charset;         /**< a charset object to be generated from corpus_character_set */
extern int clean_strings;                  /**< clean up input strings by replacing invalid bytes with '?' */
extern int numbered;                       /**< alternative input mode with token lines numbered in first column */
extern int encode_token_numbers;           /**< whether token numbers in this input mode are encoded in a p-attribute */
extern char *conll_sentence_attribute;  /**< encode blank lines as sentence breaks in this attribute */

/* ---------------------------------------------------------------------- */

/* cwb-encode encodes S-attributes and P-attributes, so there is an object-type and global array representing each. */

/**
 * s_att_builder object: represents an S-attribute being encoded, and holds some
 * information about the currently-being-processed instance of that S-attribute.
 */
typedef struct s_att_builder {
  char *dir;                    /**< directory where this s-attribute is stored */
  char *name;                   /**< name of the s-attribute  */

  int in_registry;              /**< with "-R {reg_file}", this is set to 1 when the attribute is written to the registry
                                     (avoid duplicates) */

  int store_values;             /**< flag indicating whether to store values (does _not_ automatically apply to children, see below) */
  int feature_set;              /**< stored values are feature sets => validate and normalise format */
  int null_attribute;           /**< a NULL attribute ignores all corresponding XML tags, without checking structure or annotations */
  int automatic;                /**< automatic attributes are the 'children' used for recursion and element attributes below  */

  FILE *rng_fh;                 /**< fh of rng component (cpos start/end pairs for the attribute's ranges) */
  FILE *avx_fh;                 /**< fh of avx component (the attribute value index) */
  FILE *avs_fh;                 /**< fh of avs component (the attribute values) */
  int offset;                   /**< string offset for next string (in avs component) */

  cl_lexhash lh;                /**< lexicon hash for attribute values */

  int has_children;             /**< whether attribute values of XML elements are stored in s-attribute 'children' */
  cl_lexhash el_attributes;     /**< maps XML element attribute names to the appropriate s-attribute 'children' (s_att_builder *) */
  cl_string_list el_atts_list;  /**< list of declared element attribute names, required by s_att_close_range() function */
  cl_lexhash el_undeclared_attributes; /**< remembers undeclared element attributes, so warnings will be issued only once */

  int max_recursion;            /**< maximum auto-recursion level; 0 = no recursion (maximal regions), -1 = assume flat structure */
  int recursion_level;          /**< keeps track of level of embedding when auto-recursion is activated */
  int element_drop_count;       /**< count how many recursive subelements were dropped because of the max_recursion limit */
  struct s_att_builder **recursion_children;   /**< (usually very short) list of s-attribute 'children' for auto-recursion;
                                                    use as array; recursion_children[0] points to self! */

  int is_open;                  /**< boolean: whether there is an open structure region at the moment */
  int start_pos;                /**< if this->is_open, remember start position of current range */
  char *annot;                  /**< and annotation (if there is one) */

  int num;                      /**< number of current (if this->is_open) or next region */

} s_att_builder;

/** A global array for keeping track of S-attributes being encoded. */
s_att_builder s_encoder[MAX_ATTRIBUTES];
/** @see s_encoder */
extern int s_encoder_ix;

extern s_att_builder *conll_sentence_satt;  /**< optional hidden s-attribute for encoding blank lines as sentence breaks (-L option) */

/**
 * p_att_builder object: represents a P-attribute being encoded.
 */
typedef struct {
  char *name;                   /**< CWB name of the attribute */
  cl_lexhash lh;                /**< String hash object containing the lexicon for the encoded P attrbute */
  int position;                 /**< Byte index of the lexicon file in progress; contains total number of bytes
                                     written so far (== the beginning of the -next- string that is written) */
  int feature_set;              /**< Boolean: is this a feature set attribute? => validate and normalise format */
  FILE *lex_fh;                 /**< file handle of lexicon component */
  FILE *lexidx_fh;              /**< file handle of lexicon index component */
  FILE *corpus_fh;              /**< file handle of corpus component */
} p_att_builder;

/** A global array for keeping track of P-attributes being encoded. */
p_att_builder p_encoder[MAX_ATTRIBUTES];
/** @see p_encoder */
extern int p_encoder_ix;

/**
 * lookup hash for undeclared s-attributes and s-attributes declared with -S that
 * have annotations (which will be ignored), so warnings are issued only once
 */
extern cl_lexhash undeclared_sattrs;


/** name of the currently running program */
/* char *progname = NULL; */


/* ======================================== helper function */


/**
 * A replacement for the strtok() function which doesn't skip empty fields.
 *
 * @param s      The string to split.
 * @param delim  Delimiters to use in splitting.
 * @return       The next token from the string.
 */
char *
encode_strtok(char *s, const char *delim)
{
  char *spanp;
  int c, sc;
  char *tok;
  static char *last;


  if (s == NULL && (s = last) == NULL)
    return NULL;

  c = *s++;

  if (c == 0)          /* no non-delimiter characters */
    return last = NULL;

  tok = s - 1;

  while (1) {
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
  return NULL;
}





/* ======================================== print time */


/**
 * Prints a message plus the current time to the specified file/stream.
 *
 * @param stream  Stream to print to.
 * @param msg     Message to incorporate into the string that is printed.
 */
void
encode_print_time(char *msg)
{
  time_t now;
  time(&now);
  if (msg)
    Rprintf("%s: %s\n", msg, ctime(&now));
  else
    Rprintf("Time: %s\n", ctime(&now));
}



/* ======================================== print error message and exit */


/**
 * Prints the input line number (and input filename, if applicable) on STDERR,
 * for error messages and warnings.
 */
void
encode_print_input_lineno(void)
{
  if (nr_input_files > 0 && current_input_file_name != NULL)
    Rprintf("file %s, line #%ld", current_input_file_name, input_line);
  else
    Rprintf("input line #%ld", input_line);
}

/**
 * Prints an error message to STDERR, automatically adding a
 * message on the location of the error in the corpus.
 *
 * Then exits the program.
 *
 * @param format  Format-specifying string of the error message.
 * @param ...     Additional arguments, printf-style.
 */
int
encode_error(char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  if (format) {
    Rprintf(format, ap);
    Rprintf("\n");
  }
  else
    Rprintf("Internal error. Aborted.\n");

  if ((input_line > 0) || (current_input_file > 0)) {
    /* show location only if we've already been reading input */
    Rprintf("[location of error: ");
    encode_print_input_lineno();
    Rprintf("]\n");
  }
  return 1;
}

/* =================================================== processing directories of input files */

/**
 * Get a list of files in a given directory.
 *
 * This function only lists files with .vrt or .vrt.(gz|bz2) extensions,
 * and only files identified  by POSIX stat() as "regular".
 *
 * (Note that .vrt is dependent on DEFAULT_INFILE_EXTENSION.)
 *
 * @see        DEFAULT_INFILE_EXTENSION
 * @param dir  Path of directory to look in.
 * @return     List of paths to files (*including* the directory name).
 *             Returned as a cl_string_list object.
 */
cl_string_list
encode_scan_directory(char *dir)
{
  DIR *dirp;
  struct dirent *dp;
  struct stat statbuf;
  int n_files = 0;
  int len_dir = strlen(dir);
  cl_string_list input_files = cl_new_string_list();


  dirp = opendir(dir);
  if (dirp == NULL) {
    perror("Can't access directory");
    encode_error("Failed to scan directory specified with -F %s -- aborted.\n", dir);
  }

  errno = 0;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    char *name = dp->d_name;
    if (name != NULL) {
      int len_name = strlen(name);
      if (   (len_name >= 5 && (0 == strcasecmp(name + len_name - 4, DEFAULT_INFILE_EXTENSION)))
          || (len_name >= 8 && (0 == strcasecmp(name + len_name - 7, DEFAULT_INFILE_EXTENSION ".gz")))
          || (len_name >= 9 && (0 == strcasecmp(name + len_name - 8, DEFAULT_INFILE_EXTENSION ".bz2"))) )
      {
        char *full_name = (char *) cl_malloc(len_dir + len_name + 2);
        sprintf(full_name, "%s%c%s", dir, SUBDIR_SEPARATOR, name);
        if (stat(full_name, &statbuf) != 0) {
          perror("Can't stat file:");
          encode_error("Failed to access input file %s -- aborted.\n", full_name);
        }
        if (S_ISREG(statbuf.st_mode)) {
          cl_string_list_append(input_files, full_name);
          n_files++;
        }
        else
          cl_free(full_name);
      }
    }
  }

  if (errno != 0) {
    perror("Error reading directory");
    encode_error("Failed to scan directory specified with -F %s -- aborted.\n", dir);
  }
  if (n_files == 0)
    Rprintf("Warning: No input files found in directory -F %s !!\n", dir);

  closedir(dirp);

  cl_string_list_qsort(input_files);
  return(input_files);
}

/* =================================================== handling s-attributes and p-attributes */

/**
 * Gets the index (in the global s_encoder array) of the encoder of a specified S-attribute.
 *
 * @see         s_encoder
 * @param name  The S-attribute to search for.
 * @return      Index (as integer). -1 if the S-attribute is not found.
 */
int
s_att_builder_find(char *name)
{
  int i;

  for (i = 0; i < s_encoder_ix; i++)
    if (cl_streq(s_encoder[i].name, name))
      return i;
  return -1;
}


/**
 * Prints registry lines for a given s-attribute, and its children,
 * if any, to the specified file handle.
 *
 * @param encoder        The s-attribute in question.
 * @param dst            Stream for the registry file to write the line to.
 * @param print_comment  Boolean: if true, a comment on the original XML tags is printed.
 */
void
s_att_print_registry_line(s_att_builder *encoder, FILE *dst, int print_comment)
{
  s_att_builder *child;
  int i, n_atts;

  if (encoder->in_registry)
    return;
  else
    encoder->in_registry = 1;               /* make a note that we've already handled the range */

  if (! encoder->null_attribute) {

    if (print_comment) {
      /* print comment showing corresponding XML tags */
      Rprintf("# <%s", encoder->name);
      if (encoder->has_children) {  /* if there are element attributes, show them in the order of declaration */
        n_atts = cl_string_list_size(encoder->el_atts_list);
        for (i = 0; i < n_atts; i++)
          Rprintf(" %s=\"..\"", cl_string_list_get(encoder->el_atts_list, i));
      }
      Rprintf("> ... </%s>\n", encoder->name);
      /* print comment showing hierarchical structure (if not flat) */
      if (encoder->max_recursion == 0)
        Rprintf("# (no recursive embedding allowed)\n");
      else if (encoder->max_recursion > 0) {
        n_atts = encoder->max_recursion;
        Rprintf("# (%d levels of embedding: <%s>", n_atts, encoder->name);
        for (i = 1; i <= n_atts; i++)
          Rprintf(", <%s>", encoder->recursion_children[i]->name);
        Rprintf(").\n");
      }
    }

    /* print registry line for this s-attribute */
    Rprintf(encoder->store_values ? "STRUCTURE %-20s # [annotations]\n" : "STRUCTURE %s\n", encoder->name);

    /* print recursion children, then element attribute children */
    if (encoder->max_recursion > 0) {
      n_atts = encoder->max_recursion;
      for (i = 1; i <= n_atts; i++)
        s_att_print_registry_line(encoder->recursion_children[i], dst, 0);
    }

    /* element attribute children will print their recursion children as well */
    if (encoder->has_children) {
      n_atts = cl_string_list_size(encoder->el_atts_list);
      for (i = 0; i < n_atts; i++) {
        cl_lexhash_entry entry = cl_lexhash_find(encoder->el_attributes, cl_string_list_get(encoder->el_atts_list, i));
        child = (s_att_builder *) entry->data.pointer;
        s_att_print_registry_line(child, dst, 0);
      }
    }

    /* print blank line after each att. declaration block headed by comment */
    if (print_comment)
      Rprintf("\n");
  }
}


/**
 * Creates a s_att_builder object to store a specified s-attribute
 * (and, if appropriate, does the same for children-attributes).
 *
 * The new s_att_builder object is placed in a global variable, but a pointer
 * is also returned. So you can ignore the return value or not, as
 * you prefer.
 *
 * This is the function where the command-line formalism for defining
 * s-attributes is defined.
 *
 * @see                   s_encoder
 *
 * @param name            The string from the user specifying the name of
 *                        this attribute, recursion and any "attributes"
 *                        of this XML element - e.g. "text:0+id"
 * @param directory       The directory where the CWB data files will go.
 * @param store_values    boolean: indicates whether this s-attribute was
 *                        specified with -V (true) or -S (false) when the
 *                        program was invoked.
 * @param null_attribute  boolean: this is a null attribute, i.e. an XML
 *                        element to be ignored.
 * @return                Pointer to the new s_att_builder object
 *                        (which is a member of the global array).
 */
s_att_builder *
s_att_declare(char *name, char *directory, int store_values, int null_attribute)
{
  char buf[CL_MAX_LINE_LENGTH];
  s_att_builder *sbuilder;
  char *p, *rec, *ea_start, *ea;
  cl_lexhash_entry entry;
  int i, is_feature_set;
  char *flag_SV = (store_values) ? "-V" : "-S";

  if (debugmode)
    Rprintf("ATT: %s %s\n", flag_SV, name);

  if (s_encoder_ix >= MAX_ATTRIBUTES)
    encode_error("Too many s-attributes declared (last was <%s>).", name);

  if (directory == NULL)
    encode_error("Error: you must specify a directory for CWB data files with the -d option");

  sbuilder = &s_encoder[s_encoder_ix];  /* fill next entry in s_encoder[] */
  s_encoder_ix++;                  /* must increment range index now, in case we have children */

  cl_strcpy(buf, name);
  /* check if recursion and/or element attributes are declared */
  if ((rec = strchr(buf, ':')) != NULL) {    /* recursion declaration ":<n>"  */
    *(rec++) = '\0';
    if (strchr(buf, '+'))       /* make sure recursion is declared _before_ element attributes */
      encode_error("Usage error: recursion depth must be declared before element attributes in %s %s !", flag_SV, name);
  }
  p = (rec != NULL) ? rec : buf; /* start looking for element attribute declarations from here */
  if (NULL != (ea_start = strchr(p, '+')) )  /* element att. declaration "+<ea>" */
    *(ea_start++) = '\0';

  /* by default - not a feature set. Then test. */
  is_feature_set = 0;
  if (buf[strlen(buf)-1] == '/') {
    is_feature_set = 1;
    buf[strlen(buf)-1] = '\0';
    if (!store_values)
      encode_error("Usage error: feature set marker '/' is meaningless with -S flag in %s %s !", flag_SV, name);
    if (ea_start != NULL)
      encode_error("Usage error: values of s-attribute %s cannot be feature sets if element attributes are declared (%s %s).",
                   buf, flag_SV, name);
  }

  /* now buf points to <name> rec points to <n> and ea_start to <ea> of the first element att.;
     all strings are NUL-terminated (ea_start has the form "<ea1>+<ea2>+...+<ea_n>" */

  sbuilder->name = cl_strdup(buf);   /* name of the s-attribute */
  sbuilder->dir = cl_strdup(directory);
  sbuilder->in_registry = 0;
  sbuilder->store_values = store_values;
  sbuilder->feature_set = is_feature_set;
  sbuilder->max_recursion = (rec) ? atoi(rec) : -1; /* set recursion depth: -1 = flat structure */
  sbuilder->recursion_level = 0;
  sbuilder->automatic = 0;

  sbuilder->null_attribute = 0;
  if (null_attribute) {
    sbuilder->null_attribute = 1;
    if (rec != NULL || ea_start != NULL)
      Rprintf("Warning: recursion and element attribute specifiers are ignored for null attributes (-0 %s).'n", name);
    return sbuilder;
    /* stop initialisation here; other functions shouldn't do anything with this att */
  }

  if (ea_start != NULL)
    ea_start = cl_strdup(ea_start); /* now buf can be re-used for pathnames below */

  /* open data files for this s-attribute (children will be added later) */

  /* create .rng component */
  sprintf(buf, PATH_STRUC_RNG, directory, sbuilder->name);
  if ((sbuilder->rng_fh = fopen(buf, "wb")) == NULL) {
    perror(buf);
    encode_error("Can't write .rng file for s-attribute <%s>.", name);
  }
  if (sbuilder->store_values) {
    /* create .avx and .avs components and initialise lexicon hash */
    sprintf(buf, PATH_STRUC_AVS, sbuilder->dir, sbuilder->name);
    if ((sbuilder->avs_fh = fopen(buf, "wb")) == NULL) {
      perror(buf);
      encode_error("Can't write .avs file for s-attribute <%s>.", name);
    }

    sprintf(buf, PATH_STRUC_AVX, sbuilder->dir, sbuilder->name);
    if ((sbuilder->avx_fh = fopen(buf, "wb")) == NULL) {
      perror(buf);
      encode_error("Can't write .avx file for s-attribute <%s>.", name);
    }

    sbuilder->lh = cl_new_lexhash(10000);    /* typically, will only have moderate number of entries -> save memory */
  }
  else {
    sbuilder->avs_fh = NULL;
    sbuilder->avx_fh = NULL;
    sbuilder->lh = NULL;
  }
  sbuilder->offset = 0;
  sbuilder->is_open = 0;
  sbuilder->start_pos = 0;
  sbuilder->annot = NULL;
  sbuilder->num = 0;

  /* now that the range is initialised, declare its 'children' if necessary */
  if (sbuilder->max_recursion >= 0) {
    sbuilder->recursion_children = (s_att_builder **) cl_calloc(sbuilder->max_recursion + 1, sizeof(s_att_builder *));
    sbuilder->recursion_children[0] = sbuilder; /* zeroeth recursion level is stored in the att. itself */
    for (i = 1; i <= sbuilder->max_recursion; i++) {
      /* recursion children have 'flat' structure, because recursion is handled explicitly */
      sprintf(buf, "%s%d%s", sbuilder->name, i, is_feature_set ? "/" : "");
      sbuilder->recursion_children[i] = s_att_declare(buf, sbuilder->dir, sbuilder->store_values, /*null*/ 0);
      sbuilder->recursion_children[i]->automatic = 1; /* mark as automatically handled attribute */
    }
    sbuilder->recursion_level = 0;
    sbuilder->element_drop_count = 0;
  }

  /* element attributes children can handle recursion on their own */
  if (ea_start == NULL) {
    sbuilder->has_children = 0;
  }
  else {
    s_att_builder *att_ptr;

    sbuilder->has_children = 1;
    sbuilder->el_attributes = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);
    sbuilder->el_atts_list = cl_new_string_list();
    sbuilder->el_undeclared_attributes = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);
    ea = ea_start;
    while (ea != NULL) {
      if ((p = strchr(ea, '+')) != NULL)
        *p = '\0';              /* ea now points to NUL-terminated "<ea_i>" */

      if (sbuilder->max_recursion >= 0)
        sprintf(buf, "%s_%s:%d", sbuilder->name, ea, sbuilder->max_recursion);
      else
        sprintf(buf, "%s_%s", sbuilder->name, ea);
      /* potential feature set marker (/) is passed on to the respective child attribute and handled there */

      if (ea[strlen(ea)-1] == '/')
        ea[strlen(ea)-1] = '\0';  /* remove feature set marker from element attribute name (used for lookup in encoding) */

      if (cl_lexhash_id(sbuilder->el_attributes, ea) >= 0)
        encode_error("Element attribute <%s %s=...> declared twice!", sbuilder->name, ea);
      entry = cl_lexhash_add(sbuilder->el_attributes, ea);
      att_ptr = s_att_declare(buf, sbuilder->dir, 1, /*null*/ 0); /* element att. children always store value, of course */
      att_ptr->automatic = 1;   /* mark as automatically handled attribute */
      entry->data.pointer = att_ptr;
      cl_string_list_append(sbuilder->el_atts_list, cl_strdup(ea)); /* make copy of name (for code cleanness) */

      if (p != NULL)
        ea = p + 1 ;
      else
        ea = NULL;              /* end of element att declarations */
    }
    cl_free(ea_start);          /* don't forget to free copy of element att declaration */
  }

  return sbuilder;
}

/**
 * Closes a currently open instance (aka region, range) of an S-attribute.
 *
 * @param encoder  Pointer to the S-attribute builder whose range should close.
 * @param end_pos  The corpus position at which this instance closes.
 */
void
s_att_close_range(s_att_builder *encoder, int end_pos)
{
  cl_lexhash_entry entry;
  int close_this_range = 0;     /* whether we actually have to close this range (may be skipped or delegated in recursion mode) */
  int i, n_children, annot_len;

  if (debugmode)
    Rprintf("Close range of <%s> at cpos %d, line %ld\n", encoder->name, end_pos, input_line);

  if (encoder->null_attribute)      /* do nothing for NULL attributes */
    return;

  if (encoder->max_recursion >= 0) {                  /* recursive XML structure */
    encoder->recursion_level--;     /* decrement level of nesting */

    if (encoder->recursion_level < 0) {
      /* extra close tag (ignored) */
      encoder->recursion_level = 0;
      if (!quietly) {
        Rprintf("Close tag </%s> without matching open tag ignored (", encoder->name);
        encode_print_input_lineno();
        Rprintf(").\n");
      }
    }
    else if (encoder->recursion_level > encoder->max_recursion) {
      /* deeply nested ranges are ignored silently and only listed at the end (cf. s_att_open_range() below) */
      /*
      if (!quietly) {
        Rprintf("Close tag </%s> too deeply nested, ignored (", encoder->name);
        encode_print_input_lineno();
        Rprintf(").\n");
      }
      */
    }
    else if (encoder->recursion_level > 0)
      /* delegated to appropriate recursion child */
      s_att_close_range(encoder->recursion_children[encoder->recursion_level], end_pos);

    else
      /* encoder->recursion_level == 0, i.e. the close tag actually applies to the present s-attribute (and not a ...1, ...2 suffix etc.) */
      close_this_range = 1;
  }
  else {                        /* flat structure (traditional mode) */
    if (encoder->is_open)
      close_this_range = 1;     /* ok */
    else {
      /* extra close tag (ignored) */
      if (!quietly) {
        Rprintf("Close tag </%s> without matching open tag ignored (", encoder->name);
        encode_print_input_lineno();
        Rprintf(").\n");
      }
    }
  }

  /* now close the range and write data to disk if we really have to */
  if (close_this_range) {
    if (end_pos >= encoder->start_pos) {

      /* write (start, end) to .rng component */
      NwriteInt(encoder->start_pos, encoder->rng_fh);
      NwriteInt(end_pos, encoder->rng_fh);

      if (encoder->store_values) {
        /* shouldn't happen, but just to be on the safe side ... */
        if (encoder->annot == NULL)
          encoder->annot = cl_strdup("");

        /* check annotation length & truncate if necessary */
        annot_len = strlen(encoder->annot);
        if (annot_len >= CL_MAX_LINE_LENGTH) {
          char *target;
          if (!quietly) {
            Rprintf("Value of <%s> region exceeds maximum string length (%d > %d chars), truncated (", encoder->name, annot_len, CL_MAX_LINE_LENGTH-1);
            encode_print_input_lineno();
            Rprintf(").\n");
          }
          encoder->annot[CL_MAX_LINE_LENGTH-2] = '$'; /* truncation marker, as e.g. in Emacs */
          encoder->annot[CL_MAX_LINE_LENGTH-1] = '\0';
          /* truncation may break UTF-8 strings */
          if (utf8 == encoding_charset && !g_utf8_validate((const gchar *)encoder->annot, -1, (const gchar **)&target))
            *target = '$', *(target+1) = '\0';
        }

        /* check if annot is already in hash */
        if (!(entry = cl_lexhash_find(encoder->lh, encoder->annot)))  {
          /*
           * present annotation was not found in the hash - so it is a new value.
           * so insert annotation string into lexicon hash (with the avs offset as data.integer)
           */
          entry = cl_lexhash_add(encoder->lh, encoder->annot);
          entry->data.integer = encoder->offset;

          /* write annotation string to .avs component (at offset encoder->offset) */
          fprintf(encoder->avs_fh, "%s%c", encoder->annot, '\0');

          /* update offset, ready for the next annotation string; next str begins at (string length + null byte) */
          encoder->offset += annot_len + 1;
          /* just in case: check for integer overflow */
          if (encoder->offset < 0)
            encode_error("Too many annotation values for <%s> regions (lexicon size > %d bytes)", encoder->name, INT_MAX);
        }
        /* so at this point, either way, the annotation is in the hash (and hence, on disk in the .avs component)
           and we have its avs-offset in the *entry* variable's integer member.  */

        /* write (range_number, offset) to .avx component */
        NwriteInt(encoder->num, encoder->avx_fh);  /* this was intended for 'sparse' annotations, which I don't like (so they're no longer there) */
        NwriteInt(entry->data.integer, encoder->avx_fh);

        /* throw away the now-written annotation, and incremement the number, ready for the next range. */
        encoder->num++;
        cl_free(encoder->annot);
      }
      /* endif store_values */

      encoder->is_open = 0;
    }  /* endif end_pos >= start_pos */

    else {
      encoder->is_open = 0;      /* silently ignore empty region */
      cl_free(encoder->annot);
    }
  }

  /* if this att has element attribute children, send corresponding close_range() event to all children in the list
     (recursion and nesting errors will be handled by the children themselves) */
  if (encoder->has_children) {
    n_children = cl_string_list_size(encoder->el_atts_list);
    for (i = 0; i < n_children; i++) {
      entry = cl_lexhash_find(encoder->el_attributes, cl_string_list_get(encoder->el_atts_list, i));
      if (entry == NULL)
        encode_error("Internal error in <%s>: encoder->el_attributes inconsistent with encoder->el_atts_list!", encoder->name);
      s_att_close_range((s_att_builder *) entry->data.pointer, end_pos);
    }
  }
}


/**
 * Opens an instance of the given S-attribute.
 *
 * If encoder has element attribute children, range_open() will mess around
 * with the string annotation (otherwise not).
 *
 * @param encoder        The S-attribute to open.
 * @param start_pos  The corpus position at which this instance begins.
 * @param annot      The annotation string (the XML element's att-val pairs).
 */
void
s_att_open_range(s_att_builder *encoder, int start_pos, char *annot)
{
  cl_lexhash_entry entry;
  int open_this_range = 0;      /* whether we actually have to open this range (may be skipped or delegated in recursion mode) */
  int i, mark, point, n_children;
  char *el_att_name, *el_att_value;
  char quote_char;              /* quote char used for element attribute value ('"' or '\'') */

  if (debugmode)
    Rprintf("Open range of <%s> at cpos %d, line %ld\n", encoder->name, start_pos, input_line);

  if (encoder->null_attribute)      /* do nothing for NULL attributes */
    return;

  if (encoder->max_recursion >= 0) {
    /* recursive XML structure */
    if (encoder->recursion_level > encoder->max_recursion)
      /* deeply nested ranges are ignored; count how many we've lost */
      /* NB: an option to raise a warning here could be useful to make it easier to locate such format errors in input files */
      encoder->element_drop_count++;

    else if (encoder->recursion_level > 0)
      /* delegate to appropriate recursion child (with same annotation) */
      s_att_open_range(encoder->recursion_children[encoder->recursion_level], start_pos, (encoder->store_values) ? annot : NULL);
      /* recursion children don't parse the annotation string, so annot will remain untouched;
         since recursion children always have the same -S or -V behaviour as the parent, we only
         pass the annotation string for -V attributes in order to avoid spurious warnings */

    else                       /* encoder->recursion_level == 0, i.e. the "open" actually applies to the present s-attribute */
      open_this_range = 1;

    encoder->recursion_level++;     /* increment level of nesting */
  }
  else {
    /* flat structure (traditional mode) */
    if (encoder->is_open)
      /* if we assume flat structure, implicitly close a range that is already open */
      s_att_close_range(encoder, start_pos - 1);
    open_this_range = 1;        /* with flat structure, a start tag always opens a new range */
  }

  if (open_this_range) {
    encoder->is_open = 1;
    encoder->start_pos = line;

    if (annot == NULL)          /* shouldn't happen, but just to be on the safe side ... */
      annot = "";

    if (encoder->store_values) {
      encoder->annot = cl_strdup(annot); /* remember annotation for s_att_close_range(); must strdup because it's pointer into linebuf[] */
      /* don't warn about empty annotations, because that's explicitly allowed! */
      if (strip_blanks) {               /* annotation string may have trailing blanks */
        i = strlen(encoder->annot) - 1;
        while (i >= 0 && (encoder->annot[i] == ' ' || encoder->annot[i] == '\t'))
          encoder->annot[i--] = '\0';
      }
      if (encoder->feature_set) {
        char *token = cl_make_set(encoder->annot, /*split*/ 0);
        if (token == NULL) {
          if (! quietly) {
            Rprintf("Warning: '%s' is not a valid feature set for s-attribute %s, replaced by empty set | (",
                            encoder->annot, encoder->name);
            encode_print_input_lineno();
            Rprintf(")\n");
          }
          token = cl_strdup("|"); /* encoder->annot will be free()d later, so it must be an allocated string */
        }
        cl_free(encoder->annot);
        encoder->annot = token;
      }
    }
    else {
      /* warn about non-empty annotation string in -S attribute (unless annotation string is parsed), but only once */
      if ((!encoder->has_children) && (*annot != '\0')) {
        if (!cl_lexhash_freq(undeclared_sattrs, encoder->name)) {
          if (!quietly) {
            Rprintf("Annotations of s-attribute <%s> not stored (", encoder->name);
            encode_print_input_lineno();
            Rprintf(", warning issued only once).\n");
          }
          cl_lexhash_add(undeclared_sattrs, encoder->name); /* we can re-use the lookup hash for undeclared s-attributes :o) */
        }
      }
    }
  }

  /* if encoder has element attribute children, try to parse the annotation string into
     XML attribute="value" or attribute=id pairs (destructively modifying the original)
     NB: there must not be any leading whitespace in annot
     NB: we don't bother about recursion here; the child attributes will take care of that themselves */
  if (encoder->has_children) {
    /* we have to make sure that regions are opened for all declared element attributes, and that
       no element attribute occurs more than once in order to ensure proper nesting; */
    n_children = cl_string_list_size(encoder->el_atts_list); /* use the integer data field of the el_attributes hash */
    for (i = 0; i < n_children; i++) {
      entry = cl_lexhash_find(encoder->el_attributes, cl_string_list_get(encoder->el_atts_list, i));
      entry->data.integer = 0;  /* initialise to 0, i.e. "not handled" */
    }

    mark = 0;                   /* mark and point are offsets into annot[] */
    while (annot[mark] != '\0') {
      point = mark;

      /* identify XML element attribute name (slightly relaxed attribute naming conventions) */
      while (cl_xml_is_name_char(annot[point]))
        point++;
      while ((annot[point] == ' ') || (annot[point] == '\t')) {
        annot[point] = '\0';    /* skip optional whitespace before '=' separator, and remove it from el.att. name */
        point++;
      }

      /* now annot[point] should be the separator '=' char */
      if (annot[point] != '=') {
        if (!quietly) {
          Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (``='' not found) (", encoder->name);
          encode_print_input_lineno();
          Rprintf(").\n");
        }
        break;                  /* stop processing attributes */
      }
      annot[point] = '\0';      /* terminate el. attribute name in el_att_name = (annot+mark) */
      el_att_name = annot + mark;
      mark = point + 1;
      while ((annot[mark] == ' ') || (annot[mark] == '\t'))
        mark++; /* skip optional whitespace after '=' separator */

      /* now get the attribute value (either "value" or 'value' or id) */
      quote_char = annot[mark];
      if ((quote_char == '"') || (quote_char == '\'')) {    /* attribute="value" or attribute='value' format */
        mark++;                 /* assume it's well-formed XML and just look for next occurrence of quote_char */
        point = mark;
        while ((annot[point] != quote_char) && (annot[point] != '\0'))
          point++;
        if (annot[point] == '\0') { /* syntax error: missing end quote */
          if (!quietly) {
            Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (value missing end quote) (", encoder->name);
            encode_print_input_lineno();
            Rprintf(").\n");
          }
          break;                /* stop processing attributes */
        }
        el_att_value = annot + mark;
        annot[point] = '\0';    /* terminate attribute value, and advance mark */
        mark = point + 1;
      }
      else {                    /* attribute=id format (accepts same id's as el.att. name) */
        point = mark;
        while (cl_xml_is_name_char(annot[point]))
          point++;
        el_att_value = annot + mark;
        if (annot[point] == '\0') { /* end of annot[] reached, don't advance mark beyond NUL byte */
          mark = point;
        }
        else {                  /* terminate attribute value, and advance mark */
          annot[point] = '\0';
          mark = point + 1;
        }
        if (strlen(el_att_value) == 0) { /* syntax error: attribute=id with empty value (not allowed) */
          if (!quietly) {
            Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (attribute=id with empty value (not allowed)) (", encoder->name);
            encode_print_input_lineno();
            Rprintf(").\n");
          }
          break;                /* stop processing attributes */
        }
      }

      /* syntax check: el_att_name must be non-empty (values "" and '' are allowed) */
      if (strlen(el_att_name) == 0) {
        if (!quietly) {
          Rprintf("Attributes of open tag <%s ...> ignored because of syntax error (empty attribute name)) (", encoder->name);
          encode_print_input_lineno();
          Rprintf(").\n");
        }
        break;          /* stop processing attributes */
      }

      /* now delegate the attribute/value pair to the appropriate child attribute */
      entry = cl_lexhash_find(encoder->el_attributes, el_att_name);
      if (entry == NULL) {      /* undeclared element attribute (ignored) */
        if (!cl_lexhash_freq(encoder->el_undeclared_attributes, el_att_name)) {
          if (!quietly) {
            Rprintf("Undeclared element attribute <%s %s=...> ignored (", encoder->name, el_att_name);
            encode_print_input_lineno();
            Rprintf(", warning issued only once).\n");
          }
          cl_lexhash_add(encoder->el_undeclared_attributes, el_att_name);
        }
      }
      else {                    /* declared element attribute -> decode XML entities in value and delegate to child */
        if (entry->data.integer) {
          /* attribute already handled, i.e. it must have occurred twice in start tag -> issue warning */
          if (!quietly) {
            Rprintf("Duplicate attribute value <%s %s=... %s=...> ignored (", encoder->name, el_att_name, el_att_name);
            encode_print_input_lineno();
            Rprintf(").\n");
          }
        }
        else {
          entry->data.integer = 1; /* mark el. att. as handled */
          cl_xml_entity_decode(el_att_value);
          s_att_open_range((s_att_builder *) entry->data.pointer, start_pos, el_att_value);
        }
      }

      while ((annot[mark] == ' ') || (annot[mark] == '\t'))
        mark++;                 /* skip whitespace before next attribute="value" pair */
    }

    /* phew. that was a bit of work;
       and we still have to make sure that missing element attributes are encoded as empty strings  */
    for (i = 0; i < n_children; i++) {
      entry = cl_lexhash_find(encoder->el_attributes, cl_string_list_get(encoder->el_atts_list, i));
      if (entry->data.integer == 0) {
        s_att_open_range((s_att_builder *) entry->data.pointer, start_pos, "");
      }
    }

  } /* end if range has attribute children */
}

/**
 * Finds a p-attribute (in the global p_encoder array).
 *
 * Returns the index (in p_encoder) of the p-attribute with the given name.
 *
 * @see         p_encoder
 * @param name  The P-attribute to search for.
 * @return      Index (as integer), or -1 if not found.
 */
int
p_att_builder_find(char *name)
{
  int i;

  for (i = 0; i < p_encoder_ix; i++)
    if (cl_streq(p_encoder[i].name, name))
      return i;
  return -1;
}


/**
 * Sets up a new p-attribute builder, including opening corpus, lex and index file handles.
 *
 * All files are opened for write in the globally-specified data directory.
 *
 * @see               directory
 * @param name        Identifier string of the p-attribute
 * @param directory   The directory where the CWB data files will go.
 * @param nr_buckets  Number of buckets in the lexhash of the new p-attribute (value passed to cl_new_lexhash() )
 * @return            Always 1.
 */
int
p_att_declare(char *name, char *directory, int nr_buckets)
{
  char corname[CL_MAX_LINE_LENGTH];
  char lexname[CL_MAX_LINE_LENGTH];
  char idxname[CL_MAX_LINE_LENGTH];

  if (name == NULL)
    name = CWB_DEFAULT_ATT_NAME;

  if (directory == NULL)
    encode_error("Error: you must specify a directory for CWB data files with the -d option");
  /* This should be checked in option parsing, NOT here. (Specifically, in the -S and -P options.) */

  /* copy the name supplied as an argument (first removing feature set flag / if needful */
  p_encoder[p_encoder_ix].name = cl_strdup(name);
  if (name[strlen(name)-1] == '/') {
    p_encoder[p_encoder_ix].name[strlen(name)-1] = '\0';
    p_encoder[p_encoder_ix].feature_set = 1;
  }
  else
    p_encoder[p_encoder_ix].feature_set = 0;

  p_encoder[p_encoder_ix].lh = cl_new_lexhash(nr_buckets);
  p_encoder[p_encoder_ix].position = 0;

  /* We now create paths for each of the three files that this encoder generates.
   * The paths aren't stored in the p_attr - only the file handles from opening them. */
  sprintf(corname, PATH_POS_CORPUS, directory, p_encoder[p_encoder_ix].name);
  sprintf(lexname, PATH_POS_LEX,    directory, p_encoder[p_encoder_ix].name);
  sprintf(idxname, PATH_POS_LEXIDX, directory, p_encoder[p_encoder_ix].name);

  /* Note: corpus_fh is a binary file, lex_fh is kinda mixed(*), and lexidx_fh is a binary file.
   *
   * (*)since lexicon items are delimited by '\0' not by '\n', and 0 is not always allowed as
   * a byte in a text file. But, since '\n' is never written, the text/binary distinction doesn't matter much.
   * TODO: check whether putc() with a zero byte works on Windows?
   */
  if (!(p_encoder[p_encoder_ix].corpus_fh = fopen(corname, "wb"))) {
    perror(corname);
    encode_error("Can't write .corpus file for %s attribute.", name);
  }

  if (!(p_encoder[p_encoder_ix].lex_fh = fopen(lexname, "w"))) {
    perror(lexname);
    encode_error("Can't write .lexicon file for %s attribute.", name);
  }

  if (!(p_encoder[p_encoder_ix].lexidx_fh = fopen(idxname, "wb"))) {
    perror(idxname);
    encode_error("Can't write .lexicon.idx file for %s attribute.", name);
  }

  p_encoder_ix++;

  return 1;
}

/**
 * Closes all three file handles for each of the wattr objects
 * in cwb-encode's global array.
 */
void
p_att_builder_close_all(void)
{
  int i;

  for (i = 0; i < p_encoder_ix; i++) {
    if (EOF == fclose(p_encoder[i].lex_fh)) {
      perror("fclose() failed");
      encode_error("Error writing .lexicon file for %s attribute", p_encoder[i].name);
    }
    if (EOF == fclose(p_encoder[i].lexidx_fh)) {
      perror("fclose() failed");
      encode_error("Error writing .lexicon.idx file for %s attribute", p_encoder[i].name);
    }
    if (EOF == fclose(p_encoder[i].corpus_fh)) {
      perror("fclose() failed");
      encode_error("Error writing .corpus file for %s attribute", p_encoder[i].name);
    }
  }
}






/**
 * Processes a token data line.
 *
 * That is, it processes a line that is *not* an XML line.
 *
 * Note that this is destructive - the argument character
 * string will be changed *in situ* via an strtok-like mechanim.
 *
 * @param str  A string containing the line to process.
 */
void
encode_add_p_attr_line(char *str)
{
  /* fc = field counter (current column number, zero indexed)
   * id = container for lexicon ID int.
   * length = temp holder for a strlen return. */
  int fc, id, length;
  /* field = the current column (string).
   * token = token we will store (same as field, except in case of feature sets).
   * Both are pointers to suitable chunks of the parameter string. */
  char *field, *token;

  cl_lexhash_entry entry;

  /* the following tokenization code messes around with the contents of the str parameter,
   * which (in the usage in this program) means changing linebuf[] in main()! */
  for (field = encode_strtok(str, field_separators), fc = 0;
       fc < p_encoder_ix;
       field = encode_strtok(NULL, field_separators), fc++) {
    /* LOOP across each column in the line... */

    if (field != NULL && strip_blanks) {
      /* need to strip both leading & trailing blanks from field values */
      length = strlen(field);
      while ((length > 0) && (field[length-1] == ' '))
        field[--length] = '\0';
      while (*field == ' ')
        field++;
    }
    if ((field != NULL) && (field[0] == '\0'))
      field = NULL;  /* field == NULL -> missing field; field == "" -> empty field; both inserted as __UNDEF__ */

    if ((field != NULL) && xml_aware)
      cl_xml_entity_decode(field);

    if (field == NULL)          /* mustn't do this before cl_xml_entity_decode(), because undef_value is a constant */
      field = undef_value;

    if (p_encoder[fc].feature_set) {
      token = cl_make_set(field, /*split*/ 0);
      if (token == NULL) {
        if (! quietly) {
          Rprintf("Warning: '%s' is not a valid feature set for -P %s/, replaced by empty set | (", field, p_encoder[fc].name);
          encode_print_input_lineno();
          Rprintf(")\n");
        }
        token = cl_strdup("|");
        /* so we always have to cl_free() token for feature set attributes,
         * because either cl_make_set or cl_strdup was used */
      }
    }
    else
      token = field;

    /* check annotation length & truncate if necessary (assumes it's ok to modify token[] destructively) */
    length = strlen(token);
    if (length >= CL_MAX_LINE_LENGTH) {
      char *target;
      if (!quietly) {
        Rprintf("Value of p-attribute '%s' exceeds maximum string length (%d > %d chars), truncated (",
                p_encoder[fc].name, length, CL_MAX_LINE_LENGTH-1);
        encode_print_input_lineno();
        Rprintf(").\n");
      }
      token[CL_MAX_LINE_LENGTH-2] = '$'; /* truncation marker, as e.g. in Emacs */
      token[CL_MAX_LINE_LENGTH-1] = '\0';

      /* truncation may break UTF-8 strings. Note this code is repeated for s-encoders, above.  */
      if (utf8 == encoding_charset && !g_utf8_validate((const gchar *)token, -1, (const gchar **)&target))
        *target = '$', *(target+1) = '\0';
    }

    id = cl_lexhash_id(p_encoder[fc].lh, token);
    if (id < 0) {
      /* new entry -> write LEXIDX & LEXICON files */
      NwriteInt(p_encoder[fc].position, p_encoder[fc].lexidx_fh);
      p_encoder[fc].position += strlen(token) + 1;
      if (p_encoder[fc].position < 0)
        encode_error("Maximum size of .lexicon file exceeded for %s attribute (> %d bytes)", p_encoder[fc].name, INT_MAX);
      if (EOF == fputs(token, p_encoder[fc].lex_fh)) {
        perror("fputs() write error");
        encode_error("Error writing .lexicon file for %s attribute.", p_encoder[fc].name);
      }
      if (EOF == putc('\0', p_encoder[fc].lex_fh)) {
        perror("putc() write error");
        encode_error("Error writing .lexicon file for %s attribute.", p_encoder[fc].name);
      }
      entry = cl_lexhash_add(p_encoder[fc].lh, token);
      id = entry->id;
    }

    if (p_encoder[fc].feature_set)
      cl_free(token); /* string has been allocated by cl_make_set(). See above.  */

    NwriteInt(id, p_encoder[fc].corpus_fh);
  } /* end for loop (for each column in input data...) */
}

/**
 * Reads one input line into the specified buffer
 * (either from stdin, or from one or more input files).
 *
 * The input files are not passed to the function,
 * but are taken from the program global variables.
 *
 * This function returns False when the last input file
 * has been completely read, and automatically closes files.
 *
 * If the line that is read is not valid according to the
 * character set specified for the corpus, then an error
 * will be printed and the program shut down.
 *
 * @param buffer   Where to load the line to. Assumed to be
 *                 MAX_INPUT_LINE_LENGTH long.
 * @param bufsize  Not currently used, but should be
 *                 MAX_INPUT_LINE_LENGTH in case of future use!
 *
 * @return         boolean: true for all OK, false for a problem.
 */
int
encode_get_input_line(char *buffer, int bufsize)
{
  int ok;

  if (nr_input_files == 0) {
    /* read one line of text from stdin */
    ok = (NULL != fgets(buffer, MAX_INPUT_LINE_LENGTH, stdin));
  }
  else {
    /* input_fh is set to NULL in global initialisation. */
    if (!input_fh) {
      if (current_input_file >= nr_input_files)
        return 0;

      current_input_file_name = cl_string_list_get(input_files, current_input_file);

      input_fh = cl_open_stream(current_input_file_name, CL_STREAM_READ, CL_STREAM_MAGIC);
      if (input_fh == NULL) {
        cl_error(current_input_file_name);
        encode_error("Can't open input file %s!", current_input_file_name);
      }

      input_line = 0;
    } /* endif no input file is open */

    /* read one line of text from current input file */
    ok = (NULL != fgets(buffer, MAX_INPUT_LINE_LENGTH, input_fh));

    if (ok) {
      /* on first line of file, skip UTF8 byte-order-mark if present */
      if (input_line == 0 && encoding_charset == utf8)
        if (buffer[0] == (char)0xEF && buffer[1] == (char)0xBB && buffer[2] == (char)0xBF)
          cl_strcpy(buffer, (buffer+3));
    }
    else {
      /* assume we're at end of file -> close current input file, and try reading from next one */
      ok = (0 == cl_close_stream(input_fh));
      if (!ok) {
        Rprintf("ERROR reading from file %s (ignored).\n", current_input_file_name);
        cl_error(current_input_file_name);
      }

      /* use recursive tail call to open the next input file and read from it */
      input_fh = NULL;
      current_input_file++;
      return encode_get_input_line(buffer, bufsize);
    }
  } /* end of block to follow if we're not reading from stdin. */

  /* check encoding and standardise Unicode character composition */
  if (!cl_string_validate_encoding(buffer, encoding_charset, clean_strings))
    encode_error("Encoding error: an invalid byte or byte sequence for charset \"%s\" was encountered.\n", encoding_charset_name);

  /* normalize UTF8 to precomposed form, but don't bother with the redundant function call otherwise */
  if (encoding_charset == utf8)
    cl_string_canonical(buffer, utf8, REQUIRE_NFC, MAX_INPUT_LINE_LENGTH);

  /* finally, get rid of C0 controls iff the user asked us to clean up strings */
  if (clean_strings)
    cl_string_zap_controls(buffer, encoding_charset, '?', 0, 0);
  /* note we DIDN'T zap tab and newline, because this string has yet to be column-split */

  return ok;
}

/**
 * Writes a registry file for the corpus that has been encoded.
 * Part of cwb-encode; not a library function.
 *
 * @param registry_file  String containing the path of the file to write.
 */
void
encode_generate_registry_file(char *registry_file)
{
  FILE *registry_fh;
  char *registry_id;          /* use last part of registry filename (i.e. string following last '/' character) */
  char *corpus_name = NULL;   /* name of the corpus == uppercase version of registry_id */
  char *info_file = NULL;     /* name of INFO file: <dir>/.info or <dir>/corpus-info.txt under win; see cl/globals.h */
  char *path = NULL;
  int i;

  if (debugmode)
    Rprintf("Writing registry file %s ...\n", registry_file);

  if (!(registry_fh = fopen(registry_file, "w"))) {
    perror(registry_file);
    encode_error("Can't create registry entry in file %s!", registry_file);
  }

  i = strlen(registry_file) - 1;
  while (i > 0 && registry_file[i-1] != SUBDIR_SEPARATOR)
    i--;
  registry_id = registry_file + i;

  if (!cl_id_validate(registry_id))
      encode_error("%s is not a valid corpus ID! Can't create registry entry.", registry_id);
  /* enforce the "lowercase characters only" rule */
  cl_id_tolower(registry_id);

  i = strlen(directory) - 1;
  while (i > 0 && directory[i] == SUBDIR_SEPARATOR)
    directory[i--] = '\0';    /* remove trailing '/' from home directory */

  /* copy registry_id and convert it to uppercase */
  corpus_name = cl_strdup(registry_id);
  cl_id_toupper(corpus_name);

  info_file = (char *)cl_malloc(strlen(directory) + 1 + strlen(CWB_INFOFILE_DEFAULT_NAME) + 4); /* extra bytes as safety margin */
  sprintf(info_file, "%s%c%s", directory, SUBDIR_SEPARATOR, CWB_INFOFILE_DEFAULT_NAME);

  /* write header part for registry file */
  fprintf(registry_fh, "##\n## registry entry for corpus %s\n##\n\n", corpus_name);
  fprintf(registry_fh, "# long descriptive name for the corpus\n"
                       "NAME \"\"\n"
                       "# corpus ID (must be lowercase in registry!)\n"
                       "ID   %s\n", registry_id);
  fprintf(registry_fh, "# path to binary data files\n");
  path = cl_path_registry_quote(directory);
  fprintf(registry_fh, "HOME %s\n", path);
  cl_free(path);
  fprintf(registry_fh, "# optional info file (displayed by \"info;\" command in CQP)\n");
  path = cl_path_registry_quote(info_file);
  fprintf(registry_fh, "INFO %s\n\n", path);
  cl_free(path);
  fprintf(registry_fh, "# corpus properties provide additional information about the corpus:\n");
  /* lines marked with ##:: are NOT commented out, this is part of the normal registry format! */
  fprintf(registry_fh, "##:: charset  = \"%s\" # character encoding of corpus data\n", encoding_charset_name);
  fprintf(registry_fh, "##:: language = \"??\"     # insert ISO code for language (de, en, fr, ...)\n"
                       "\n\n");

  /* insert p-attributes into registry file */
  fprintf(registry_fh, "##\n## p-attributes (token annotations)\n##\n\n");
  for (i = 0; i < p_encoder_ix; i++)
    fprintf(registry_fh, "ATTRIBUTE %s\n", p_encoder[i].name);

  fprintf(registry_fh, "\n\n");

  /* insert s-attributes into registry file */
  fprintf(registry_fh, "##\n## s-attributes (structural markup)\n##\n\n");
  for (i = 0; i < s_encoder_ix; i++)
    s_att_print_registry_line(&s_encoder[i], registry_fh, 1);

  fprintf(registry_fh, "\n"
                       "# Yours sincerely, the Encode tool.\n");

  fclose(registry_fh);

  cl_free(corpus_name);
  cl_free(info_file);
}


/**
 * Check whether input line looks like a token with annotations,
 * i.e. whether it has the correct number of k > 1 TAB-separated fields.
 *
 * Used to suppress error messages about malformed XML tags if they
 * are likely to be tokens starting with '<'.
 *
 * @param line     Input line (read-only, must be NUL-terminated).
 *
 * @return         boolean: true if line looks like at token
 */
int
looks_like_a_token(const char *line) {
  int n_fields = 1;
  for (; *line; line++)
    if (*line == '\t')
      n_fields++;
  /* don't print warning if it looks like a valid token line with annotations */
  return ((n_fields > 1) && (n_fields == p_encoder_ix));
}
int cwb_encode_worker(cl_string_list input_files){
  int i, i_sep, j, k, s_att_ix, handled, might_be_token, encode_line;

  char linebuf[MAX_INPUT_LINE_LENGTH];
  char *buf;                    /* 'virtual' buffer; may be advanced to skip leading blanks */
  char separator;

  int input_length;             /* length of input line */

  /* initialise global variables */
  cl_startup();
  /* progname = "cwb-encode"; */
/* input_files = cl_new_string_list(); */

  /* parse command-line options */

  /* encode_parse_options(argc, argv); */
  nr_input_files = cl_string_list_size(input_files);

  /* initialisation debug messages */
  if (debugmode) {
    cl_set_debug_level(1);
    if (nr_input_files > 0) {
      Rprintf("List of input files:\n");
      for (i = 0; i < nr_input_files; i++)
        Rprintf(" - %s\n", cl_string_list_get(input_files, i));
    }
    else
      Rprintf("Reading from standard input.\n");
    encode_print_time("Start");
  }

  /* initialise loop variables ... */
  encoding_charset = cl_charset_from_name(encoding_charset_name);
  line = 0;
  input_line = 0;

  /* lookup hash for (undeclared) structural attributes (inserted as tokens into corpus) */
  undeclared_sattrs = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);

  /* MAIN LOOP: read one line of input and process it */
  while ( encode_get_input_line(linebuf, MAX_INPUT_LINE_LENGTH) ) {
    if (verbose && (line % 15000 == 0)) {
      Rprintf("%" COMMA_SEP_THOUSANDS_CONVSPEC "9dk tokens processed\r", line >> 10);
      fflush(stdout);
    }

    input_line++;
    input_length = strlen(linebuf);
    if (input_length >= (MAX_INPUT_LINE_LENGTH - 1))  /* buffer filled -> line may have been longer */
      encode_error("Input line too long (max: %d characters/bytes).", MAX_INPUT_LINE_LENGTH - 2);

    /* remove trailing line break (LF or CR-LF) */
    cl_string_chomp(linebuf);

    buf = linebuf;
    if (strip_blanks)
      /* strip leading blanks (trailing blanks will be erased during further processing) */
      while (*buf == ' ')
        buf++;

    /* This block runs UNLESS either
     *   (a) skip_empty_lines (-s) is active and this an empty line;
     *   (b) numbered (-n) is active and this line is a comment starting with #
     *   (c) xml_aware (-x) is active and this line is an XML comment or declaration, i.e. <? or <!
     * To put it another way "if (this is a line that should be encoded)" ...  */
    encode_line = 1;
    if (buf[0] == '\0') { /* empty line */
      if (skip_empty_lines)
        encode_line = 0;
      if (conll_sentence_satt && (conll_sentence_satt->is_open))
        s_att_close_range(conll_sentence_satt, line - 1); /* blank line indicates sentence boundary with -L option */
    }
    if (numbered && (buf[0] == '#'))
      encode_line = 0;
    if (xml_aware && (buf[0] == '<') && ((buf[1] == '?') || (buf[1] == '!')))
      encode_line = 0;

    if (encode_line) {
      handled = 0;

      if (buf[0] == '<') {
        /* XML tag (may be declared or undeclared s-attribute, start or end tag) */
        k = (buf[1] == '/' ? 2 : 1);
        might_be_token = !numbered && looks_like_a_token(buf); /* check whether the line might be a token with annotations (except in -n mode) */

        /* identify XML element name (according to slightly relaxed attribute naming conventions!) */
        i = k;
        while (cl_xml_is_name_char(buf[i]))
          i++;
        /* first non-valid XML element name character must be whitespace or '>' or '/' (for empty XML element) */
        if ( ! (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '>' || buf[i] == '/') )
          i = k;                /* no valid element name found */

        if (i > k) {
          /* looks like a valid XML tag */
          separator = buf[i];   /* terminate string containing element name, but remember original char */
          i_sep = i;            /* and remember where it needs to go */
          buf[i] = '\0';        /* so that we can reconstruct the line if we have to insert it literally after all */

          if ((s_att_ix = s_att_builder_find(&buf[k])) >= 0) {
            /* good, it's a declared s-attribute and can be handled */
            handled = 1;

            if (s_encoder[s_att_ix].automatic) {
              if (!cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
                Rprintf("explicit XML tag <%s%s> for implicit s-attribute ignored (", (k == 1) ? "" : "/", &buf[k]);
                encode_print_input_lineno();
                Rprintf(", warning issued only once).\n");
                cl_lexhash_add(undeclared_sattrs, &buf[k]); /* can reuse lexhash for undeclared attributes here */
              }
            }
            else {
              if (k == 1) {     /* XML start tag or empty tag */
                i++;            /* identify annotation string, i.e. tag attributes (if there are any) */
                while (buf[i] == ' ' || buf[i] == '\t') /* skip whitespace between element name and first attribute */
                  i++;
                if (separator == '>') {
                  /* tag without annotations: check that there is no extraneous material on the line */
                  if (buf[i] != '\0') {
                    Rprintf("Warning: extra material after XML tag ignored (");
                    encode_print_input_lineno();
                    Rprintf(").\n");
                    buf[i] = '\0';
                  }
                }
                else {
                  j = i + strlen(buf+i); /* find '>' character marking end of tag (must be last character on line) */
                  while ((j > i) && (buf[j] == ' ' || buf[j] == '\t' || buf[j] == '\0'))
                    j--; /* set j to last non-blank character on line, which should be '>' */
                  if (buf[j] != '>') {
                    Rprintf("Malformed XML tag: missing > terminator at end of line (");
                    encode_print_input_lineno();
                    Rprintf(", annotations will be ignored).\n");
                    buf[i] = '\0'; /* so the annotation string passed to range_open() below is empty */
                  }
                  else {
                    if (buf[j-1] == '/') {
                      j--; /* empty tag: remove "/" from annotation string and handle as an open tag */
                      /* Note that this implicitly closes the previous instance of the empty tag:
                       *  - this means that we can work with empty elements by looking just at the "open-point" of each range;
                       *  - it also means that empty tags with metadata at the start of each text will automatically extend over the full text.
                       * However, the approach sketched here only works with "flat" s-attributes declared without recursion (even without :0). */
                    }
                    buf[j] = '\0';
                  }
                }
                /* start tag: open range */
                s_att_open_range(&s_encoder[s_att_ix], line, buf+i);
              }
              else {            /* XML end tag */
                if (separator != '>') {
                  Rprintf("Warning: no annotations allowed on XML close tag </%s ...> (", &buf[k]);
                  encode_print_input_lineno();
                  Rprintf(", ignored).\n");
                }
                s_att_close_range(&s_encoder[s_att_ix], line - 1); /* end tag belongs to previous line! */
              }
            }
          }
          else {
            /* no appropriate s-attribute declared -> insert tag literally (unless auto-null option -9 was specified or -n input format is used) */
            if (might_be_token) {
              /* but if it looks like a token line with annotations, always insert as token line without warning */
              buf[i_sep] = separator; /* restore original line */
            }
            else {
              if (auto_null || numbered) {
                if (!quietly && !cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
                  Rprintf("undeclared s-attribute <%s> will be ignored (implicit '-0 %s', ", &buf[k], &buf[k]);
                  encode_print_input_lineno();
                  Rprintf(").\n");
                  cl_lexhash_add(undeclared_sattrs, &buf[k]);
                }
                /* Actually declaring the implicit -0 attributes creates two problems:
                 *  (1) we might run out of s-attribute slots, so encoding a huge corpus might abort close to the end (and we didn't even check!);
                 *  (2) it would overwrite the hidden s-attribute created by the -L option.
                 * For these reasons, the line below has been removed, making auto-null a little slower and minimally less reliable. */
                /* s_att_declare(&buf[k], directory, 0, 1); -- auto-declare as null attribute */
                handled = 1; /* mark line as handled so it doesn't get inserted as a token */
              }
              else {
                /* print warning once (unless in -q mode), then fall through to token line with p-attributes */
                if (!quietly) {
                  if (!cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
                    Rprintf("s-attribute <%s> not declared, inserted literally (", &buf[k]);
                    encode_print_input_lineno();
                    Rprintf(", warning issued only once).\n");
                    cl_lexhash_add(undeclared_sattrs, &buf[k]);
                  }
                }
                buf[i_sep] = separator; /* restore original line, which will be interpreted as token line */
              }
            }
          }
        }
        /* malformed XML tag (no element name found) */
        else if (!might_be_token) {
          if (numbered) {
            encode_error("Malformed tag %s, encoding aborted", buf);
          }
          else if (!quietly) {
            Rprintf("Malformed tag %s, inserted literally (", buf);
            encode_print_input_lineno();
            Rprintf(").\n");
          }
        }
      } /* endif line begins with < */

      /* if we haven't handled the line so far, it must be a token with (optional) annotations */
      if (!handled) {
        if (numbered) {
          /* in numbered mode, all token lines must start with a number in the first TAB-delimited column;
           * check that it is present and skip it before encoding the line */
          int ok_numbered = 0;
          i = strspn(buf, "0123456789");
          if (i > 0) {
            if (buf[i] == '-' || buf[i] == '.') {
              /* CoNLL multiword (3-4) or trace (5.1) token --> discard */
              i++;
              while(buf[i] >= '0' && buf[i] <= '9')
                i++;
              if (buf[i] == '\t') {
                handled = 1; /* mark line to be discarded */
                ok_numbered = 1;
              }
            }
            else if (buf[i] == '\t') {
              if (!encode_token_numbers)
                buf = buf + i + 1; /* skip line number and TAB character (unless -N option has been specified) */
              ok_numbered = 1;
            }
          }
          if (!ok_numbered)
            encode_error("Invalid input line [%s], encoding aborted", buf); /* all other lines are fatal errors */
        }

        if (!handled) {
          if (conll_sentence_satt && !conll_sentence_satt->is_open)
            s_att_open_range(conll_sentence_satt, line, NULL); /* -L option: start sentence region before a token is encoded */
          encode_add_p_attr_line(buf);
          line++;                 /* line is now the corpus position of the next token that will be encoded */
          if (line >= CL_MAX_CORPUS_SIZE) {
            /* largest admissible corpus size should be 2^31 - 1 tokens, with maximal cpos = 2^31 - 2 */
            Rprintf("WARNING: Maximal corpus size has been exceeded.\n");
            Rprintf("         Input truncated to the first %d tokens (", CL_MAX_CORPUS_SIZE);
            encode_print_input_lineno();
            Rprintf(").\n");
            break;
          }
        }
      }
    } /* endif (this is a line that should be encoded) */
  } /* endwhile (main loop for each line) */

  if (verbose) {
    Rprintf("%50s\r", "");       /* clear progress line */
    Rprintf("Total size: %" COMMA_SEP_THOUSANDS_CONVSPEC "d tokens (%.1fM)\n", line, ((float) line) / 1048576);
  }

  /* unhide the implicit s-attribute from -L so it's cleaned up and included in the registry file */
  if (conll_sentence_satt) {
    s_encoder_ix++;
    if (conll_sentence_satt->is_open)
      s_att_close_range(conll_sentence_satt, line - 1); /* close last sentence so we don't get a warning below */
  }

  /* close open regions at end of input; then close file handles for s-attributes */
  for (i = 0; i < s_encoder_ix; i++) {
    s_att_builder *encoder = &s_encoder[i];

    if (! encoder->null_attribute) { /* don't attempt to close NULL attribute */

      /* This is fairly tricky: When multiple end tags are missing for an attribute declared with recursion (even ":0"),
         we have to call s_att_close_range() repeatedly to ensure that the open region at the top level is really closed
         (which happens when encoder->recursion_level == 1). At the same time, s_att_close_range() will also close the corresponding
         ranges of any implicitly defined attributes (used to resolve recursive embedding and element attributes).
         Therefore, the following code calls s_att_close_range() repeatedly until the current range is actually closed.
         It also relies on the ordering of the s_encoder[] array, where top level attributes always precede their children,
         so they should be closed automatically before cleanup reaches them. If the ordering were different, children might
         be closed directly at first, and the following attempt to close them automatically from within the s_att_close_range()
         function would produce highly confusing error messages. To be on the safe side (for some definition of safe :-),
         we _never_ close ranges for implicit attributes, and issue a warning if they're still open when cleanup reaches them.
      */
      if (encoder->automatic) {
        /* implicitly generated s-attributes should have been closed automatically */
        if (!quietly && encoder->is_open)
          Rprintf("Warning: implicit s-attribute <%s> open at end of input (should not have happened).\n", encoder->name);
      }
      else {
        if (encoder->is_open) {
          if (encoder->recursion_level > 1)
            Rprintf("Warning: %d missing </%s> tags inserted at end of input.\n", encoder->recursion_level, encoder->name);
          else
            Rprintf("Warning: missing </%s> tag inserted at end of input.\n", encoder->name);

          /* close open region; this will automatically close children from recursion and element attributes;
             if multiple end tags are missing, we have to call s_att_close_range() repeatedly until we reach the top level */
          while (encoder->is_open) /* should _not_ create an infinite loop, I hope */
            s_att_close_range(encoder, line - 1);
        }

        if (!quietly && encoder->max_recursion >= 0 && encoder->element_drop_count > 0)
          Rprintf("%7d <%s> regions dropped because of deep nesting.\n", encoder->element_drop_count, encoder->name);
      }

      /* close file handles for s-attributes */
      if (EOF == fclose(encoder->rng_fh)) {
        perror("fclose() failed");
        encode_error("Error writing .rng file for s-attribute <%s>", encoder->name);
      }
      if (encoder->store_values) {
        if (EOF == fclose(encoder->avs_fh)) {
          perror("fclose() failed");
          encode_error("Error writing .avs file for s-attribute <%s>", encoder->name);
        }
        if (EOF == fclose(encoder->avx_fh)) {
          perror("fclose() failed");
          encode_error("Error writing .avx file for s-attribute <%s>", encoder->name);
        }
      }
    }
  } /* endfor: closing each open region and s-attribute filehandle for each s_att_builder */

  /* close file handles for positional attributes */
  p_att_builder_close_all();

  /* if registry_file has something in it, write appropriate registry entry to file named <registry_file> */
  if (registry_file)
    encode_generate_registry_file(registry_file);

  if (debugmode)
    encode_print_time("Done");

  return 0;
}

