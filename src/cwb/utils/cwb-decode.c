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

#include <ctype.h>

#include "../cl/cl.h"
#include "../cl/cwb-globals.h"
#include "../cl/corpus.h"
#include "../cl/attributes.h"

char *progname = NULL;
char *registry_directory = NULL;
char *corpus_name = NULL;
Corpus *corpus = NULL;

/* ---------------------------------------- */

/** Maximum number of attributes that can be printed. */
#define MAX_ATTRS 2048

/**
 * Chain of s-attributes that represent attribute-value pairs of an XML region
 */
typedef struct AVList {
  char *name;                   /**< name of the XML attribute to be displayed */
  Attribute *handle;            /**< CWB handle of the attribute */
  struct AVList *next;          /**< pointer to next element of chain */
} *AVList;

/**
 * Represents an attribute to be printed together with some additional information
 */
typedef struct {
  char *name;                   /**< display name (may be different from CWB attribute name for certain s-attributes) */
  Attribute *handle;            /**< CWB handle of the attribute */
  int type;                     /**< attribute type, same as handle->type */
  AVList av_list;               /**< for XML attribute only: linked list of attribute-value pairs to be displayed in start tag */
} AttSpec;

AttSpec print_list[MAX_ATTRS];    /**< array of attributes selected by user for printing */
int print_list_index = 0;            /**< Number of atts added to print_list (so far), i.e. index of the first free element */

/**
 * Represents a single s-attribute region and its annotation.
 *
 * Before a token is printed, all regions of s-attributes from print_list[]
 * which contain that token are copied to s_att_regions[],
 * bubble-sorted (to enforce proper nesting while retaining
 * the specified order as far as possible), and printed from s_att_regions[] .
 */
typedef struct {
  const char *name;             /**< display name of the s-attribute */
  int start;                    /**< start and end of region (for sorting) */
  int end;
  const char *annot;            /**< NULL if there is no annotation; otherwise the content of the annotation */
  AVList av_list;               /**< optional linked list of attribute-value pairs to be displayed in a start tag */
} SAttRegion;
SAttRegion s_att_regions[MAX_ATTRS];
int sar_sort_index[MAX_ATTRS];  /**< index used for bubble-sorting list of regions */
int N_regions = 0;                  /**< number of regions currently in list (may change for each token printed) */

/* ---------------------------------------- */

/* the following are used only in matchlist mode: */

/** Maximum number of attributes whose "surrounding values" can be printed in matchlist mode. */
#define MAX_PRINT_VALUES 1024

Attribute *printValues[MAX_PRINT_VALUES];   /**< List of s-attributes whose values are to be printed */
int printValuesIndex = 0;                   /**< Number of atts added to printValues (so far);
                                             *   used with less-than, = top limit for scrolling that array */

/* ---------------------------------------- */

int first_token;            /**< cpos of token to begin output at */
int last_token;             /**< cpos of token to end output at (inclusive; ie this one gets printed!) */
int maxlast;                /**< maximum ending cpos + 1 (deduced from size of p-attribute);  */
int printnum = 0;           /**< whether or not token numbers are to be printed (-n option) */
Attribute *conll_delimiter = NULL; /**< if not NULL, print empty line after each region of this s-attribute */

/**
 * Supported output formats:
 *
 *   EncodeMode     -C, -Cx ... the main and best-supported output format
 *   XMLMode        -X      ... XML output
 *   ConclineMode   -H      ... "horizontal" output, intended for kwic-style concordance lines
 *   LispMode       -L      ... Lisp-style notation
 *   StandardMode           ... the default output format
 */
typedef enum _output_modes {
  StandardMode, LispMode, EncodeMode, ConclineMode, XMLMode
} OutputMode;

OutputMode mode = StandardMode;  /**< global variable for overall output mode */
int xml_compatible = 0;          /**< xml-style, for (cwb-encode -x ...); EncodeMode only, selected by -Cx */


/**
 * Cleans up memory prior to an error-prompted exit. This should be completely superfluous because
 * cwb-decode doesn't acquire any external resources (and memory will be released much more efficiently
 * by program termination).
 *
 * @param error_code  Value to be returned by the program when it exits.
 */
void
cleanup_and_exit(int error_code)
{
  int i;
  AVList p, p2;

  for (i = 0; i < print_list_index; i++) {
    cl_free(print_list[i].name);
    cl_delete_attribute(print_list[i].handle);
    p = print_list[i].av_list;
    while (p) {
      p2 = p;
      p = p->next;
      cl_free(p2->name);
      cl_delete_attribute(p2->handle);
      cl_free(p2);
    }
  }

  if (corpus != NULL)
    cl_delete_corpus(corpus);
  exit(error_code);
}

/**
 * Prints a usage message and exits the program.
 *
 * @param exit_code  Value to be returned by the program when it exits.
 */
void
decode_usage(int exit_code)
{
  fprintf(stderr, "\n");
  fprintf(stderr, "Usage:  %s [options] <corpus> [declarations]\n\n", progname);
  fprintf(stderr, "Decodes CWB corpus as plain text (or in various other text formats).\n");
  fprintf(stderr, "In normal mode, the entire corpus (or a segment specified with the\n");
  fprintf(stderr, "-s and -e options) is printed on stdout. In matchlist mode (-p or -f),\n");
  fprintf(stderr, "(pairs of) corpus positions are read from stdin (or a file specified\n");
  fprintf(stderr, "with -f), and the corresponding tokens or ranges are displayed. The\n");
  fprintf(stderr, "[declarations] determine which attributes to display (-ALL for all attributes).\n\n");
  fprintf(stderr, "See list of options for available output modes.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -L        Lisp output mode\n");
  fprintf(stderr, "  -H        concordance line ('horizontal') output mode\n");
  fprintf(stderr, "  -C        compact output mode (suitable for cwb-encode)\n");
  fprintf(stderr, "  -Cx       XML-compatible compact output (for \"cwb-encode -x ...\")\n");
  fprintf(stderr, "  -X        XML output mode\n");
  fprintf(stderr, "  -n        show corpus position ('numbers') in first column\n");
  fprintf(stderr, "  -s <n>    first token to print (at corpus position <n>)\n");
  fprintf(stderr, "  -e <n>    last token to print (at corpus position <n>)\n");
  fprintf(stderr, "  -r <dir>  set registry directory\n");
  fprintf(stderr, "  -p        matchlist mode (input from stdin)\n");
  fprintf(stderr, "  -f <file> matchlist mode (input from <file>)\n");
  fprintf(stderr, "  -Sp, -Sf  subcorpus mode (output all XML tags, but only selected tokens)\n");
  fprintf(stderr, "  -b <att>  print blank line at the end of each region of s-attribute <att>\n");
  fprintf(stderr, "  -h        this help page\n\n");
  fprintf(stderr, "Attribute declarations:\n");
  fprintf(stderr, "  -P <att>  print p-attribute <att>\n");
  fprintf(stderr, "  -S <att>  print s-attribute <att> (including annotated values if available)\n");
  fprintf(stderr, "            [can use same recursion and tag attribute specifiers as in cwb-encode]\n");
  fprintf(stderr, "  -V <att>  show s-attribute annotation for each range in matchlist mode\n");
  fprintf(stderr, "  -A <att>  print alignment attribute <att>\n");
  fprintf(stderr, "  -ALL      print all p-attributes and s-attributes\n");
  fprintf(stderr, "  -c <att>  expand ranges to full <att> region (matchlist mode)\n\n");
  fprintf(stderr, "Part of the IMS Open Corpus Workbench v" CWB_VERSION "\n\n");

  cleanup_and_exit(exit_code);
}

/**
 * Check whether a string represents a number.
 *
 * @param s  The string to check.
 * @return   Boolean: true iff s contains only digits.
 */
int
is_num(char *s)
{
  int i;

  for (i = 0; s[i]; i++)
    if (!isdigit((unsigned char)s[i]))
      return 0;

  return 1;
}


/**
 * Escapes a string according to the currently active global mode.
 *
 * In XMLMode, this function converts the string to an encoded XML string;
 * all 'critical' characters are replaced by entity references,
 * and C0 control characters are replaced with blanks. (This also happens
 * in other modes - i.e. compact - if the global xml_compatible variable is true.)
 *
 * In LispMode, it converts the string to a Lisp string with the required escapes (probably!)
 *
 * In any other mode, it does nothing, and just returns the argument pointer.
 *
 * It is safe to use this function without checking for a NULL argument,
 * as NULLs will just be returned as NULLs.
 *
 * Warning: returns pointer to static internal buffer of fixed size;
 * in particular, don't use it twice in a single argument list!
 *
 * @see      EncodeMode
 * @param s  String to encode.
 * @return   Pointer to encoded string in static internal buffer; or,
 *           the argument s iff the mode is not one that requires any
 *           encoding. If the argument is NULL, NULL is returned.
 */
const char *
decode_string_escape(const char *s)
{
  int i, t = 0;
  static char coded_s[CL_MAX_LINE_LENGTH];

  if (s == NULL)
    return NULL;

  if (mode == XMLMode || xml_compatible) {
    for (i = 0; s[i]; i++) {
      if (s[i] == '"') {
        sprintf(coded_s+t, "&quot;");
        t += strlen(coded_s+t);
      }
      else if (s[i] == '\'') {
        sprintf(coded_s+t, "&apos;");
        t += strlen(coded_s+t);
      }
      else if (s[i] == '<') {
        sprintf(coded_s+t, "&lt;");
        t += strlen(coded_s+t);
      }
      else if (s[i] == '>') {
        sprintf(coded_s+t, "&gt;");
        t += strlen(coded_s+t);
      }
      else if (s[i] == '&') {
        sprintf(coded_s+t, "&amp;");
        t += strlen(coded_s+t);
      }
      else if ((s[i] > 0) && (s[i] < 32)) {
        /* C0 controls are invalid -> substitute blanks */
        coded_s[t++] = ' ';
      }
      else {
        coded_s[t++] = s[i];
      }
    }
    /* terminate converted string and return it */
    coded_s[t] = '\0';
    return coded_s;
  }
  else if (mode == LispMode) {
    for (i = 0; s[i]; i++) {
      if ((s[i] == '"') || (s[i] == '\\')) {
        coded_s[t++] = '\\';
        coded_s[t++] = '\\';
        coded_s[t++] = '\\';
      }
      coded_s[t++] = s[i];
    }
    coded_s[t] = '\0';
    /* terminate converted string and return it */
    return coded_s;
  }
  else
    /* all other modes : do nothing */
    return s;
}

/**
 * Prints an XML declaration, using character set specification
 * obtained from the global corpus variable.
 */
void
decode_print_xml_declaration(void)
{
  CorpusCharset charset = unknown_charset;
  if (corpus)
    charset = cl_corpus_charset(corpus);

  printf("<?xml version=\"1.0\" encoding=\"");
  switch (charset) {
  case latin1:
    printf("ISO-8859-1");
    break;
  case latin2:
    printf("ISO-8859-2");
    break;
  case latin3:
    printf("ISO-8859-3");
    break;
  case latin4:
    printf("ISO-8859-4");
    break;
  case cyrillic:
    printf("ISO-8859-5");
    break;
  case arabic:
    printf("ISO-8859-6");
    break;
  case greek:
    printf("ISO-8859-7");
    break;
  case hebrew:
    printf("ISO-8859-8");
    break;
  case latin5:
    printf("ISO-8859-9");
    break;
  case latin6:
    printf("ISO-8859-10");
    break;
  case latin7:
    printf("ISO-8859-13");
    break;
  case latin8:
    printf("ISO-8859-14");
    break;
  case latin9:
    printf("ISO-8859-15");
    break;
  case utf8:
    printf("UTF-8");
    break;
  case unknown_charset:
  default:
    printf("ISO-8859-1");       /* at least the parser isn't going to break down that way. probably. */
    break;
  }
  printf("\" standalone=\"yes\" ?>\n");
}


/**
 * Sorts s_att_regions[MAX_ATTRS] in ascending 'nested' order,
 * using sar_sort_index[] (which is automatically initialised).
 *
 * Since only regions which begin or end at the current token are
 * considered, such an ordering is always possible;
 * without knowing the current token, we sort by end position descending,
 * then by start position ascending, which gives us:
 *
 *  - first the regions corresponding to start tags, beginning with the 'largest' region
 *
 *  - then the regions corresponding to end tags, again beginning with the 'largest' region
 *
 * The function uses bubble sort in order to retain the existing order of identical regions.
 *
 */
void
decode_sort_s_att_regions(void)
{
  int i, modified;

  for (i = 0; i < N_regions; i++)   /* initialise sort index */
    sar_sort_index[i] = i;

  /* repeat 'bubble' loop until no more modifications are made */
  modified = 1;

  while (modified) {
    modified = 0;
    for (i = 0; i < (N_regions-1); i++) {
      SAttRegion *a = &(s_att_regions[sar_sort_index[i]]); /* compare *a and *b */
      SAttRegion *b = &(s_att_regions[sar_sort_index[i+1]]);

      if ( a->end < b->end || (a->end == b->end && a->start > b->start) ) {
        int temp = sar_sort_index[i];
        /* swap sar_sort_index[i] and sar_sort_index[i+1] */
        sar_sort_index[i] = sar_sort_index[i+1];
        sar_sort_index[i+1] = temp;
        modified = 1;
        /* modified ordering, so we need another loop iteration */
      }
    }
  }
}

/**
 * Determines whether or not a given Attribute is in an array of attribute specifications.
 *
 * @param handle         CWB handle of the attribute to look for.
 * @param att_list       Pointer to the first member of the array (i.e. array name).
 * @param att_list_size  Upper bound of the array (the last member the function checks is attlist[attlist_size-1]).
 * @return               Boolean.
 */
int
decode_attribute_is_in_list(Attribute *handle, AttSpec *att_list, int att_list_size)
{
  int i;
  for (i = 0; i < att_list_size; i++)
    if (att_list[i].handle == handle)
      return 1;
  return 0;
}

/**
 * Obtains CWB handle for the specified attribute. Aborts the program in case of any error,
 * so the return value does not need to be checked.
 *
 * @param name   CWB name of the attribute.
 * @param type   Attribute type (ATT_POS, ATT_STRUC, ATT_ALIGN).
 * @return       CWB handle of the attribute (Attribute *).
 */
Attribute *
get_attribute_handle(const char *name, int type)
{
  Attribute *handle;
  const char *att_type = "";

  if (ATT_POS == type)
    att_type = "positional";
  else if (ATT_STRUC == type)
    att_type = "structural";
  else if (ATT_ALIGN == type)
    att_type = "alignment";
  else {
    if (ATT_DYN == type)
      fprintf(stderr, "Error: Dynamic attributes (%s.%s) are not supported by %s (aborted).\n", corpus_name, name, progname);
    else
      fprintf(stderr, "Internal error: Unknown attribute type code %d (aborted).\n", type);
    cleanup_and_exit(1);
  }

  if (! (handle = cl_new_attribute(corpus, name, type)) ) {
    fprintf(stderr, "Error: Can't access %s attribute %s.%s (aborted).\n", att_type, corpus_name, name);
    cleanup_and_exit(1);
  }

  return handle;
}


/**
 * Adds a specified attribute (by name and type) to the global print_list array.
 * Aborts the program if that array is already full or another error occurs.
 *
 * @param name          CWB name of the attribute.
 * @param type          Attribute type (ATT_POS, ATT_STRUC, ATT_ALIGN).
 * @param display_name  Print attribute under a different name (NULL: use 'name').
 * @param avspec        Optional '+'-separated specifier of XML attribute-value pairs (ATT_STRUC only).
 * @param recursion     Recursion level, appended to all attribute names (esp. those created by avspec).
 * @return              Boolean.
 */
int
decode_add_attribute(const char *name, int type, const char *display_name, const char *avspec, int recursion)
{
  Attribute *handle, *av_handle;
  AVList point = NULL, new;
  static char temp[CL_MAX_LINE_LENGTH];
  char *av, *item;
  int size, n;

  if (!display_name)
    display_name = name;
  if (print_list_index >= MAX_ATTRS) {
    fprintf(stderr, "Too many attributes (maximum is %d, aborted).\n", MAX_ATTRS);
    cleanup_and_exit(2);
    return 0;
  }
  if (recursion > 0)
    sprintf(temp, "%s%d", name, recursion);
  else
    cl_strcpy(temp, name);
  handle = get_attribute_handle(temp, type); /* aborts with error message if not found */
  if (decode_attribute_is_in_list(handle, print_list, print_list_index)) {
    fprintf(stderr, "Warning: Attribute %s.%s added twice to print list (ignored).\n", corpus_name, temp);
    return 0;
  }
  print_list[print_list_index].name = cl_strdup(display_name);
  print_list[print_list_index].type = type;
  print_list[print_list_index].handle = handle;
  print_list[print_list_index].av_list = NULL;

  if (avspec) {
    if (type != ATT_STRUC) {
      fprintf(stderr, "Internal error: Attribute-value items only allowed for structural attributes, not for %s (aborted).\n", name);
      cleanup_and_exit(2);
    }
    if (cl_struc_values(handle))
      fprintf(stderr, "Warning: Annotated values of s-attribute %s will be ignored by -S %s+%s\n", temp, name, avspec);
    size = cl_max_struc(handle); /* plausibility check: all attributes in AV list must have same size as main s-attribute */
    /* add linked list of attribute-value items selected by avspec */
    av = cl_strdup(avspec);
    for (item = strtok(av, "+"); item; item = strtok(NULL, "+")) {
      if (recursion > 0)
        sprintf(temp, "%s_%s%d", name, item, recursion);
      else
        sprintf(temp, "%s_%s", name, item);
      av_handle = get_attribute_handle(temp, ATT_STRUC);
      if (!cl_struc_values(av_handle)) {
        fprintf(stderr, "Error: S-attribute %s selected by -S %s+%s has no annotated values (aborted).\n",
                temp, name, item);
        cleanup_and_exit(2);
      }
      n = cl_max_struc(av_handle);
      if (n != size) {
        fprintf(stderr, "Error: S-attribute %s selected by -S %s+%s not compatible with main s-attribute (%d != %d regions, aborted).\n",
                temp, name, item, n, size);
        cleanup_and_exit(2);
      }
      new = cl_malloc(sizeof(struct AVList));
      if (point == NULL)
        print_list[print_list_index].av_list = new;
      else
        point->next = new;
      point = new;
      new->name = cl_strdup(item);
      new->handle = av_handle;
      new->next = NULL;
    }
    cl_free(av);
  }

  print_list_index++;
  return 1;
}

/**
 * Add structural attribute, with support for an extended specification as shown below.
 * If it includes a recursion marker, further s-attributes will be declared.
 * Aborts the program with a suitable message if any error is encountered.
 *
 * An extended attribute specification has a form similar to that used by cwb-encode,
 * with the addition of an alternative display_name that can optionally be specified.
 *
 *   [display_name=]name[:N][+av1+av2+...]
 *
 * @param att_spec  Attribute specification of the form described above.
 * @return          Boolean.
 */
int
decode_add_s_attribute(const char *att_spec)
{
  char *point, *mark;
  char *spec = cl_strdup(att_spec);
  char *display_name = NULL, *recursion_spec = NULL, *av_spec = NULL;
  char *name = spec;
  int recursion = 0, ok = 1, i;

  /* optional display name */
  if ((point = strchr(spec, '=')) != NULL) {
    *(point++) = '\0';
    display_name = spec;
    name = point;
  }

  /* optional recursion specifier */
  if ((point = strchr(name, ':')) != NULL) {
    *(point++) = '\0';
    if (strchr(name, '+')) {
      /* make sure recursion is declared _before_ attribute-value pairs */
      fprintf(stderr, "Error: recursion depth must be declared before attribute-value items in -S %s (aborted).\n", att_spec);
      cleanup_and_exit(2);
    }
    recursion_spec = mark = point;
  }
  else
    mark = name;

  /* optional attribute-value pairs in start tags */
  if ((point = strchr(mark, '+')) != NULL) {
    *(point++) = 0; /* terminates name or recursion_spec */
    av_spec = point;
  }

  if (recursion_spec) {
    if (strspn(recursion_spec, "0123456789") != strlen(recursion_spec)) {
      fprintf(stderr, "Error: invalid recursion specifier :%s in -S %s (aborted).\n", recursion_spec, att_spec);
      cleanup_and_exit(2);
    }
    recursion = atoi(recursion_spec);
  }

  if (!display_name)
    display_name = name;

  /* declare s-attribute (and recursion attributes if recursion > 0) */
  for (i = 0; i <= recursion; i++)
    ok &= decode_add_attribute(name, ATT_STRUC, display_name, av_spec, i);

  cl_free(spec);
  return ok;
}

/**
 * Check the context of the global printValues array, to check that no s-attribute in
 * it is declared in the main print_list_index as well.
 *
 * If an attribute is found to be declared in both, a warning is printed.
 */
void
decode_verify_print_value_list(void)
{
  int i;
  for (i = 0; i < printValuesIndex; i++)
    if (decode_attribute_is_in_list(printValues[i], print_list, print_list_index))
      fprintf(stderr, "Warning: s-attribute %s.%s used with both -S and -V.\n", corpus_name, printValues[i]->any.name);
}

/**
 * Prints a starting tag for each s-attribute surrounding the position (for -V flags)
 */
void
decode_print_surrounding_s_att_values(int position)
{
  int i;
  char *tagname;

  for (i = 0; i < printValuesIndex; i++) {
    if (printValues[i]) {
      const char *sval;
      int snum = cl_cpos2struc(printValues[i], position);

      /* don't print tag if start position is not in region */
      if (snum >= 0) {
        /* if it is a p- or a- attribute, snum is a CL error (less than 0) */
        sval = decode_string_escape(cl_struc2str(printValues[i], snum));
        tagname = printValues[i]->any.name;

        switch (mode) {
        case ConclineMode:
          printf("<%s %s>: ", tagname, sval);
          break;

        case LispMode:
          printf("(VALUE %s \"%s\")\n", tagname, sval);
          break;

        case XMLMode:
          printf("<element name=\"%s\" value=\"%s\"/>\n", tagname, sval);
          break;

        case EncodeMode:
          /* pretends to be a comment, but has to be stripped before feeding output to encode */
          printf("# %s=%s\n", tagname, sval);
          break;

        case StandardMode:
        default:
          printf("<%s %s>\n", tagname, sval);
          break;
        }
      }
    }
  }
}

/**
 * Expand range [start] .. [end] to encompass full regions of s-attribute [context].
 *
 * Function arguments are overwritten with the new values.
 */
void
decode_expand_context(int *start, int *end, Attribute *context)
{
  int dummy;
  int start_as_passed = *start, end_as_passed = *end;

  assert(end_as_passed >= start_as_passed);

  if (!cl_cpos2struc2cpos(context, start_as_passed, start, &dummy))
    *start = start_as_passed;
  if (!cl_cpos2struc2cpos(context, end_as_passed, &dummy, end))
    *end = end_as_passed;
}

/**
 * Prints out the requested attributes for a sequence of tokens
 * (or a single token if end_position == -1, which also indicates that we're not in matchlist mode).
 *
 * If the -c flag was used (and, thus, the context parameter is not NULL),
 * then the sequence is extended to the entire s-attribute region (intended for matchlist mode).
 *
 * If the additional skip_token parameter is true, only XML tags before/after each token are printed,
 * but not the tokens themselves (for subcorpus mode).
 */
void
decode_print_token_sequence(int start_position, int end_position, Attribute *context, int skip_token)
{
  int alg, aligned_start, aligned_end, aligned_start2, aligned_end2, rng_start, rng_end, snum;
  int start_context, end_context;
  int i, w;
  int beg_of_line, has_values;
  AVList av;

  /* pointer used for values of p-attributes */
  const char *wrd;

  start_context = start_position;
  end_context = (end_position >= 0) ? end_position : start_position;

  if (context != NULL) {
    /* expand the start_context end_context numbers to the start
     * and end points of the containing region of the context s-attribute */
    decode_expand_context(&start_context, &end_context, context);

    /* indicate that we're showing context */
    switch (mode) {
    case LispMode:
      printf("(TARGET %d\n", start_position);
      if (end_position >= 0)
        printf("(INTERVAL %d %d)\n", start_position, end_position);
      break;
    case EncodeMode:
    case ConclineMode:
      /* nothing here */
      break;
    case XMLMode:
      printf("<context start=\"%d\" end=\"%d\"/>\n", start_context, end_context);
      break;
    case StandardMode:
    default:
      if (end_position >= 0)
        printf("INTERVAL %d %d\n", start_position, end_position);
      else
        printf("TARGET %d\n", start_position);
      break;
    }
  }

  /* some extra information in -L and -H modes */
  if (mode == LispMode && end_position != -1)
    printf("(CONTEXT %d %d)\n", start_context, end_context);
  else if (mode == ConclineMode && printnum)
    printf("%8d: ", start_position);

  /* now print the token sequence (including context) with all requested attributes */
  for (w = start_context; w <= end_context; w++) {

    /* extract s-attribute regions for start and end tags into s_att_regions[] */
    N_regions = 0;                  /* counter and index */
    for (i = 0; i < print_list_index; i++) {
      if (print_list[i].type == ATT_STRUC) {
        if (0 <= (snum = cl_cpos2struc(print_list[i].handle, w)) &&
            cl_struc2cpos(print_list[i].handle, snum, &rng_start, &rng_end) &&
            (w == rng_start || w == rng_end)
            ) {
          has_values = cl_struc_values(print_list[i].handle);
          s_att_regions[N_regions].name = print_list[i].name;
          s_att_regions[N_regions].start = rng_start;
          s_att_regions[N_regions].end = rng_end;
          s_att_regions[N_regions].av_list = print_list[i].av_list;
          if (has_values && !print_list[i].av_list)
            s_att_regions[N_regions].annot = cl_struc2str(print_list[i].handle, snum);
          else
            s_att_regions[N_regions].annot = NULL;
          N_regions++;
        }
      }
    }
    decode_sort_s_att_regions();       /* sort regions to ensure proper nesting of start and end tags */

    /* show corpus positions with -n option */
    if (printnum)
      switch (mode) {
      case LispMode:
        printf("(%d ", w);
        break;
      case EncodeMode:
        /* nothing here (inserted at start of token line below) */
        break;
      case ConclineMode:
        /* nothing here (shown at start of line in -H mode) */
        break;
      case XMLMode:
        /* nothing here */
        break;
      case StandardMode:
      default:
        printf("%8d: ", w);
        break;
      }
    else if (mode == LispMode)
      printf("(");            /* entire match is parenthesised list in -L mode */

    /* print start tags (s- and a-attributes) with -C, -H, -X */
    if ((mode == EncodeMode) || (mode == ConclineMode) || (mode == XMLMode)) {

      /* print a-attributes from print_list[] */
      for (i = 0; i < print_list_index; i++) {
        if (print_list[i].type == ATT_ALIGN) {
          if (((alg = cl_cpos2alg(print_list[i].handle, w)) >= 0) &&
              (cl_alg2cpos(print_list[i].handle, alg,
                           &aligned_start, &aligned_end,
                           &aligned_start2, &aligned_end2)) &&
              (w == aligned_start))
          {
            if (mode == XMLMode) {
              printf("<align type=\"start\" target=\"%s\"", print_list[i].name);
              if (printnum)
                printf(" start=\"%d\" end=\"%d\"", aligned_start2, aligned_end2);
              printf("/>\n");
            }
            else {
              printf("<%s", print_list[i].name);
              if (printnum)
                printf(" %d %d", aligned_start2, aligned_end2);
              printf(">%c", (mode == EncodeMode) ? '\n' : ' ');
            }
          }
        }
      }

      /* print s-attributes from s_att_regions[] (using sar_sort_index[]) */
      for (i = 0; i < N_regions; i++) {
        SAttRegion *region = &(s_att_regions[sar_sort_index[i]]);

        if (region->start == w) {
          if (mode == XMLMode) {
            printf("<tag type=\"start\" name=\"%s\"", region->name);
            if (printnum)
              printf(" cpos=\"%d\"", w);
            if (region->annot)
              printf(" value=\"%s\"", decode_string_escape(region->annot));
            if (region->av_list)
              printf(">");
          }
          else {
            printf("<%s", region->name);
            if (region->annot)
              printf(" %s", region->annot);
          }
          av = region->av_list;
          while (av) {
            wrd = cl_cpos2struc2str(av->handle, w);
            if (!wrd) {
              fprintf(stderr, "Error: Missing attribute-value data for <%s %s='...'> at cpos=%d (aborted).\n",
                      region->name, av->name, w);
              cleanup_and_exit(1);
            }
            wrd = decode_string_escape(wrd);
            if (mode == XMLMode)
              printf(" <attr name=\"%s\">%s</attr>", av->name, wrd);
            else
              printf(" %s=\"%s\"", av->name, wrd);
            av = av->next;
          }
          if (mode == XMLMode)
            printf((region->av_list) ? " </tag>\n" : "/>\n");
          else if (mode == ConclineMode)
            printf("> ");
          else
            printf(">\n");
        }
      }
    }

    if (!skip_token) {
      /* now print token with its attribute values (p-attributes only for -C,-H,-X) */
      if (mode == XMLMode) {
        printf("<token");
        if (printnum)
          printf(" cpos=\"%d\"", w);
        printf(">");
      }
      else if (mode == EncodeMode) {
        if (printnum)
          printf("%d\t", w);
      }

      beg_of_line = 1;
      /* Loop printing each attribute for this cpos (w) */
      for (i = 0; i < print_list_index; i++) {

        switch (print_list[i].type) {
        case ATT_POS:
          if ((wrd = decode_string_escape(cl_cpos2str(print_list[i].handle, w))) != NULL) {
            switch (mode) {
            case LispMode:
              printf("(%s \"%s\")", print_list[i].name, wrd);
              break;

            case EncodeMode:
              if (beg_of_line) {
                printf("%s", wrd);
                beg_of_line = 0;
              }
              else
                printf("\t%s", wrd);
              break;

            case ConclineMode:
              if (beg_of_line) {
                printf("%s", wrd);
                beg_of_line = 0;
              }
              else
                printf("/%s", wrd);
              break;

            case XMLMode:
              printf(" <attr name=\"%s\">%s</attr>", print_list[i].name, wrd);
              break;

            case StandardMode:
            default:
              printf("%s=%s\t", print_list[i].name, wrd);
              break;
            }
          }
          else {
            cl_error("(aborting) cl_cpos2str() failed");
            cleanup_and_exit(1);
          }
          break;

        case ATT_ALIGN:
          /* do not print in encode, concline or xml modes because already done (above) */
          if (mode != EncodeMode && mode != ConclineMode && mode != XMLMode) {
            if (((alg = cl_cpos2alg(print_list[i].handle, w)) >= 0)
                && (cl_alg2cpos(print_list[i].handle, alg,
                    &aligned_start, &aligned_end,
                    &aligned_start2, &aligned_end2))
            ) {
              if (mode == LispMode)
                printf("(ALG %d %d %d %d)", aligned_start, aligned_end, aligned_start2, aligned_end2);
              else {
                printf("%d-%d==>%s:%d-%d\t",
                    aligned_start, aligned_end, print_list[i].name, aligned_start2, aligned_end2);
              }
            }
            else if (cl_errno != CDA_EALIGN) {
              cl_error("(aborting) alignment error");
              cleanup_and_exit(1);
            }
          }
          break;

        case ATT_STRUC:
          /* do not print in encode, concline or xml modes because already done (above) */
          if ((mode != EncodeMode) && (mode != ConclineMode) && (mode != XMLMode)) {
            if (cl_cpos2struc2cpos(print_list[i].handle, w, &rng_start, &rng_end)) {
              /* standard and -L mode don't show tag annotations */
              printf(mode == LispMode ? "(STRUC %s %d %d)" : "<%s>:%d-%d\t",
                     print_list[i].name, rng_start, rng_end);
            }
            else if (cl_errno != CDA_ESTRUC) {
              cl_error("(aborting) cl_cpos2struc2cpos() failed");
              cleanup_and_exit(1);
            }
          }
          break;

        case ATT_DYN:
          /* dynamic attributes aren't implemented */
        default:
          break;
        }
      }

      /* print token separator (or end of token in XML mode) */
      switch (mode) {
      case LispMode:
        printf(")\n");
        break;
      case ConclineMode:
        printf(" ");
        break;
      case XMLMode:
        printf(" </token>\n");
        break;
      case EncodeMode:
      case StandardMode:
      default:
        printf("\n");
        break;
      }
    }

    /* now, after printing all the positional attributes, print end tags with -H,-C,-X */
    if (mode == EncodeMode  || mode == ConclineMode || mode == XMLMode) {

      /* print s-attributes from s_att_regions[] (using sar_sort_index[] in reverse order) */
      for (i = N_regions - 1; i >= 0; i--) {
        SAttRegion *region = &(s_att_regions[sar_sort_index[i]]);

        if (region->end == w) {
          if (mode == XMLMode) {
            printf("<tag type=\"end\" name=\"%s\"", region->name);
            if (printnum)
              printf(" cpos=\"%d\"", w);
            printf("/>\n");
          }
          else {
            printf("</%s>%c", region->name, (mode == ConclineMode ? ' ' : '\n'));
          }
        }
      }

      /* print a-attributes from print_list[] */
      for (i = 0; i < print_list_index; i++) {
        if (print_list[i].type == ATT_ALIGN) {
          if (((alg = cl_cpos2alg(print_list[i].handle, w)) >= 0) &&
              (cl_alg2cpos(print_list[i].handle, alg,
                           &aligned_start, &aligned_end,
                           &aligned_start2, &aligned_end2)) &&
              (w == aligned_end))
          {
            if (mode == XMLMode) {
              printf("<align type=\"end\" target=\"%s\"", print_list[i].name);
              if (printnum)
                printf(" start=\"%d\" end=\"%d\"", aligned_start2, aligned_end2);
              printf("/>\n");
            }
            else {
              printf("</%s", print_list[i].name);
              if (printnum)
                printf(" %d %d", aligned_start2, aligned_end2);
              printf(">%c", (mode == EncodeMode) ? '\n' : ' ');
            }
          }
        }
      }

    } /* end of print end tags */

    if (conll_delimiter) {
      /* print blank line at end of each sentence (or other s-attribute selected with -b option) */
      if (cl_cpos2boundary(conll_delimiter, w) & STRUC_RBOUND)
        printf("\n");
    }
  }  /* end of match range loop: for w from start_context to end_context */

  /* end of match (for matchlist mode in particular) */
  if ((context != NULL) && (mode == LispMode))
    printf(")\n");
  else if (mode == ConclineMode)
    printf("\n");

  return;
}

