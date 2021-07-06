extern "C" {
  #include "cl_min.h"
  #include <attributes.h>
  #include "utils.h"
}


  
#include <Rcpp.h>
/* using namespace Rcpp; */

// [[Rcpp::export(name=".cwb_makeall")]]
int cwb_makeall(SEXP x, SEXP registry_dir, SEXP p_attribute){
  
  /* char *progname = "RcppCWB"; */
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char * corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  int validate = 1;

  ComponentID cid = CompLast;

  corpus = cl_new_corpus(registry_directory, corpus_id);
  
  Rprintf("=== Makeall: processing corpus %s ===\n", corpus_id);
  Rprintf("Registry directory: %s\n", corpus->registry_dir);
  
  Attribute *attribute = cl_new_attribute(corpus, attr_name, ATT_POS);
  do_attribute(attribute, cid, validate);

  Rprintf("========================================\n");
  return 0;
}


// [[Rcpp::export(name=".cwb_huffcode")]]
int cwb_huffcode(SEXP x, SEXP registry_dir, SEXP p_attribute) {
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char * corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  
  char *output_fn = NULL;
  Attribute *attr;
  
  HCD hc;
  
  int i_want_to_believe = 0;        /* skip error checks? */
  /* int all_attributes = 0; */
  
  /* protocol = stdout;   */             /* 'delayed' init (see top of file) */
  
  
  if ((corpus = cl_new_corpus(registry_directory, corpus_id)) == NULL) {
    Rprintf("Corpus %s not found in registry %s . Aborted.\n", 
            corpus_id,
            (registry_directory ? registry_directory
               : central_corpus_directory()));
    return 1;
  }
  
  if ((attr = cl_new_attribute(corpus, attr_name, ATT_POS)) == NULL) {
    Rprintf("Attribute %s.%s doesn't exist. Aborted.\n",  corpus_id, attr_name);
    return 1;
  }
  
  compute_code_lengths(attr, &hc, output_fn);
  if (! i_want_to_believe) decode_check_huff(attr, corpus_id, output_fn);
  
  cl_delete_corpus(corpus);
  
  return 0;
}


// [[Rcpp::export(name=".cwb_compress_rdx")]]
int cwb_compress_rdx(SEXP x, SEXP registry_dir, SEXP p_attribute) {
  
  char *registry_directory = strdup(Rcpp::as<std::string>(registry_dir).c_str());
  char *attr_name = strdup(Rcpp::as<std::string>(p_attribute).c_str());
  char *corpus_id = strdup(Rcpp::as<std::string>(x).c_str());
  
  Attribute *attr;
  
  char *output_fn = NULL;
  
#ifdef _WIN32
  int i_want_to_believe = 1;        /* skip error on Windows, for the time being */
#else
  int i_want_to_believe = 0;        /* do not skip error checks on macOS and Linux */
#endif
  
  
  
  
  /* debug_output = stderr; */        /* 'delayed' init (see top of file) */
  int debug = 0;
  
  if ((corpus = cl_new_corpus(registry_directory, corpus_id)) == NULL) {
    Rprintf("Corpus %s not found in registry %s . Aborted.\n", 
            corpus_id,
            (registry_directory ? registry_directory
               : central_corpus_directory()));
    cleanup(1);
  }
  
  if ((attr = find_attribute(corpus, attr_name, ATT_POS, NULL)) == NULL) {
    Rprintf("Attribute %s.%s doesn't exist. Aborted.\n", corpus_id, attr_name);
    cleanup(1);
  }
  
  compress_reversed_index(attr, output_fn, corpus_id, debug);
  if (! i_want_to_believe) decompress_check_reversed_index(attr, output_fn, corpus_id, debug);
  
  cleanup(0);
  return 0;                        /* to keep gcc from complaining */
}

 

/** User privileges of new files (octal format) */
#define UMASK              0644

/* TODO the following belongs in the CL. */
/** Default string used as value of P-attributes when a value is missing ie if a tab-delimited field is empty */
#define UNDEF_VALUE strdup("__UNDEF__")
/** Default string containing the characters that can function as field separators */
#define FIELDSEPS  (char*)"\t\n"

/** max number of s-attributes; also max number of p-attributes (-> could change this to implementation as a linked list) */
#define MAXRANGES 1024

/** nr of buckets of lexhashes used for checking duplicate errors (undeclared element and attribute names in XML tags) */
#define REP_CHECK_LEXHASH_SIZE 1000

/** Input buffer size. If we have XML tags with attributes, input lines can become pretty long
 * (but there's basically just a single buffer)
 */
#define MAX_INPUT_LINE_LENGTH  65536

/** Normal extension for CWB input text files. (must have exactly 4 characters; .gz/.bz2 may be added to this if the file is compressed.) */
#define DEFAULT_INFILE_EXTENSION ".vrt"

/* implicit knowledge about CL component files naming conventions */
#define STRUC_RNG  "%s" SUBDIR_SEP_STRING "%s.rng"            /**< CL naming convention for S-attribute RNG files */
#define STRUC_AVX  "%s" SUBDIR_SEP_STRING "%s.avx"            /**< CL naming convention for S-attribute AVX (attribute-value index) files */
#define STRUC_AVS  "%s" SUBDIR_SEP_STRING "%s.avs"            /**< CL naming convention for S-attribute AVS (attribute values) files */
#define POS_CORPUS "%s" SUBDIR_SEP_STRING "%s.corpus"         /**< CL naming convention for P-attribute Corpus files */
#define POS_LEX    "%s" SUBDIR_SEP_STRING "%s.lexicon"        /**< CL naming convention for P-attribute Lexicon files */
#define POS_LEXIDX "%s" SUBDIR_SEP_STRING "%s.lexicon.idx"    /**< CL naming convention for P-attribute Lexicon-index files */

/* ---------------------------------------------------------------------- */

cl_lexhash undeclared_sattrs;

/**
 * Range object: represents an S-attribute being encoded, and holds some
 * information about the currently-being-processed instance of that S-attribute.
 *
 * TODO should probably be called an SAttr or SAttEncoder or something.
 */
typedef struct _SAttEncoder {
  char *dir;                    /**< directory where this s-attribute is stored */
char *name;                   /**< name of the s-attribute (range) */

int in_registry;              /**< with "-R {reg_file}", this is set to 1 when the attribute is written to the registry
 (avoid duplicates) */

int store_values;             /**< flag indicating whether to store values (does _not_ automatically apply to children, see below) */
int feature_set;              /**< stored values are feature sets => validate and normalise format */
int null_attribute;           /**< a NULL attribute ignores all corresponding XML tags, without checking structure or annotations */
int automatic;                /**< automatic attributes are the 'children' used for recursion and element attributes below  */

FILE *fd;                     /**< fd of rng component */
FILE *avx;                    /**< fd of avx component (the attribute value index) */
FILE *avs;                    /**< fd of avs component (the attribute values) */
int offset;                   /**< string offset for next string (in avs component) */

cl_lexhash lh;                /**< lexicon hash for attribute values */

int has_children;             /**< whether attribute values of XML elements are stored in s-attribute 'children' */
cl_lexhash el_attributes;     /**< maps XML element attribute names to the appropriate s-attribute 'children' (Range *) */
cl_string_list el_atts_list;  /**< list of declared element attribute names, required by range_close() function */
cl_lexhash el_undeclared_attributes; /**< remembers undeclared element attributes, so warnings will be issued only once */

int max_recursion;            /**< maximum auto-recursion level; 0 = no recursion (maximal regions), -1 = assume flat structure */
int recursion_level;          /**< keeps track of level of embedding when auto-recursion is activated */
int element_drop_count;       /**< count how many recursive subelements were dropped because of the max_recursion limit */
struct _SAttEncoder **recursion_children;   /**< (usually very short) list of s-attribute 'children' for auto-recursion;
 use as array; recursion_children[0] points to self! */

int is_open;                  /**< boolean: whether there is an open structure at the moment */
int start_pos;                /**< if this->is_open, remember start position of current range */
char *annot;                  /**< and annotation (if there is one) */

int num;                      /**< number of current (if this->is_open) or next structure */

} SAttEncoder;


SAttEncoder ranges[MAXRANGES];
int range_ptr = 0;

/* ---------------------------------------------------------------------- */

/* global variables representing configuration */

char *field_separators = FIELDSEPS;     /**< string containing the characters that can function as field separators */
char *undef_value = UNDEF_VALUE;        /**< string used as value of P-attributes when a value is missing
 ie if a tab-delimited field is empty */
int debugmode = 1;                          /**< debug mode on or off? */
int quietly = 0;                         /**< hide messages */
int verbose = 0;                        /**< show progress (this is _not_ the opposite of quietly!) */
int xml_aware = 0;                      /**< substitute XML entities in p-attributes & ignore <? and <! lines */
int skip_empty_lines = 0;               /**< skip empty lines when encoding? */
unsigned line = 0;                      /**< corpus position currently being encoded (ie cpos of _next_ token) */
/* unsigned so it doesn't wrap after first 2^31 tokens and we can abort encoding when corpus size is exceeded */
int strip_blanks = 0;                   /**< strip leading and trailing blanks from input and token annotations */
cl_string_list input_files = NULL;      /**< list of input file(s) (-f option(s)) */
int nr_input_files = 0;                 /**< number of input files (length of list after option processing) */
int current_input_file = 0;             /**< index of input file currently being processed */
char *current_input_file_name = NULL;   /**< filename of current input file, for error messages */
FILE *input_fd = NULL;                  /**< file handle for current input file (or pipe) (text mode!) */
unsigned long input_line = 0;           /**< input line number (reset for each new file) for error messages */
char *registry_file = NULL;             /**< if set, auto-generate registry file named {registry_file}, listing declared attributes */
char *directory = NULL;                 /**< corpus data directory (no longer defaults to current directory) */
char *corpus_character_set = (char*)"latin1";  /**< character set label that is inserted into the registry file */
CorpusCharset encoding_charset;         /**< a charset object to be generated from corpus_character_set */
int clean_strings = 0;                  /**< clean up input strings by replacing invalid bytes with '?' */

/* ---------------------------------------------------------------------- */


// [[Rcpp::export(name=".cwb_encode")]]
int cwb_encode(SEXP regfile, SEXP data_dir, SEXP vrt_dir, Rcpp::StringVector p_attributes, Rcpp::StringVector s_attributes_anno, Rcpp::StringVector s_attributes_noanno){
  
  /* the following code is copied from cwb_encode.c */
  
  int i, j, k, rng, handled;
  
  char linebuf[MAX_INPUT_LINE_LENGTH];
  char *buf;                    /* 'virtual' buffer; may be advanced to skip leading blanks */
  char separator;
  
  int input_length;             /* length of input line */
  
  /* initialise global variables */
  /* progname = "cwb-encode"; */
  
  /* default settings that substitute encode_parse_options() */
  
  verbose++;
  quietly = 0; /* values larger than 0 would silence cwb_encode, but we want messages */
  strip_blanks++; /* -B: strip leading and trailing blanks from tokens and annotations by default */
  clean_strings++; /*< clean up input strings by replacing invalid bytes with '?' */
  xml_aware++; /* -x: translate XML entities and ignore declarations & comments */
  corpus_character_set = cl_charset_name_canonical(strdup("utf8"));
  directory = strdup(Rcpp::as<std::string>(data_dir).c_str());
  registry_file = strdup(Rcpp::as<std::string>(regfile).c_str());
  skip_empty_lines++; /* -s: skip empty lines */
  
  /* declare p-attributes */
  
  int p_attrs_n = p_attributes.length();
  int m;
  for (m = 0; m < p_attrs_n; m++){
    wattr_declare(p_attributes(m), directory, 0);
  }

  /* declare s-attribute with annotations */
  
  int s_attrs_len = s_attributes_anno.length();
  for (m = 0; m < s_attrs_len; m++){
    range_declare(s_attributes_anno(m), directory, 1, 0);
  }
  
  /* declare s-attribute without annotations */
  s_attrs_len = s_attributes_noanno.length();
  for (m = 0; m < s_attrs_len; m++){
    range_declare(s_attributes_noanno(m), directory, 0, 0);
  }
  
  input_files = cl_new_string_list();
  cl_string_list dir_files;
  dir_files = encode_scan_directory(strdup(Rcpp::as<std::string>(vrt_dir).c_str()));
  
  int l;
  l = cl_string_list_size(dir_files);
  for (i = 0; i < l; i++) {
    cl_string_list_append(input_files, cl_string_list_get(dir_files, i));
  }
  cl_delete_string_list(dir_files); /* allocated strings have been moved into input_files, so don't free() them */
  
  nr_input_files = cl_string_list_size(input_files);
  
  /* initialisation debug messages */
  if (debugmode) {
    cl_set_debug_level(1);
    if (nr_input_files > 0) {
      Rprintf("List of input files:\n");
      for (i = 0; i < nr_input_files; i++)
        Rprintf(" - %s\n", cl_string_list_get(input_files, i));
    }
    else {
      Rprintf("Reading from standard input.\n");
    }
    encode_print_time(strdup("Start"));
  }
  
  /* initialise loop variables ... */
  encoding_charset = cl_charset_from_name(corpus_character_set);
  line = 0;
  input_line = 0;
  
  /* lookup hash for (undeclared) structural attributes (inserted as tokens into corpus) */
  undeclared_sattrs = cl_new_lexhash(REP_CHECK_LEXHASH_SIZE);
  
  /* -------------------------------------------------- */
  
  /* MAIN LOOP: read one line of input and process it */
  while ( encode_get_input_line(linebuf, MAX_INPUT_LINE_LENGTH) ) {
    if (verbose && (line % 15000 == 0)) {
      Rprintf("%" COMMA_SEP_THOUSANDS_CONVSPEC "9dk tokens processed\r", line >> 10);
      /* fflush(stdout); */
    }
    
    input_line++;
    input_length = strlen(linebuf);
    if (input_length >= (MAX_INPUT_LINE_LENGTH - 1)) { /* buffer filled -> line may have been longer */
  encode_error(strdup("Input line too long (max: %d characters/bytes)."), MAX_INPUT_LINE_LENGTH - 2);
    }
    
    /* remove trailing line break (LF or CR-LF) */
    string_chomp(linebuf);
    
    buf = linebuf;
    if (strip_blanks) {
      while (*buf == ' ')
        /* strip leading blanks (trailing blanks will be erased during further processing) */
        buf++;
    }
    
    /* This bit runs UNLESS either (a) skip_empty_lines (-s) is active and this an empty line;
     * or (b) xml_aware (-x) is active and this line is an XML comment or declaration, i.e. <? or <!
     * To put it another way "if (this is a line that should be encoded)" ...  */
    if ( (! (skip_empty_lines && (buf[0] == '\0')) ) &&                            /* skip empty lines with -s  */
    (! (xml_aware && (buf[0] == '<') && ((buf[1] == '?') || (buf[1] == '!'))) ) /* skip XML declarations/comments with -x  */
    ) {
      /* skip empty lines with -s option (for an empty line, first character will usually be newline) */
      handled = 0;
      
      if (buf[0] == '<') {
        /* XML tag (may be declared or undeclared s-attribute, start or end tag) */
        k = (buf[1] == '/' ? 2 : 1);
        
        /* identify XML element name (according to slightly relaxed attribute naming conventions!) */
        i = k;                  
        while (cl_xml_is_name_char(buf[i]))
          i++;
        /* first non-valid XML element name character must be whitespace or '>' or '/' (for empty XML element) */
        if (! ((buf[i] == ' ') || (buf[i] == '\t') || (buf[i] == '>') || (buf[i] == '/')) ) 
          i = k;                /* no valid element name found */
        
        if (i > k) {
          /* looks like a valid XML tag */
          separator = buf[i];   /* terminate string containing element name, but remember original char */
          buf[i] = '\0';        /* so that we can reconstruct the line if we have to insert it literally after all */
          
          if ((rng = range_find(&buf[k])) >= 0) {
            /* good, it's a declared s-attribute and can be handled */
            handled = 1;
            
            if (ranges[rng].automatic) {
              if (!cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
                Rprintf("explicit XML tag <%s%s> for implicit s-attribute ignored (", 
                        (k == 1) ? "" : "/", &buf[k]);
                encode_print_input_lineno();
                Rprintf(", warning issued only once).\n");
                cl_lexhash_add(undeclared_sattrs, &buf[k]); /* can reuse lexhash for undeclared attributes here */
              }
            }
            else {
              if (k == 1) {     /* XML start tag or empty tag */
            i++;            /* identify annotation string, i.e. tag attributes (if there are any) */
            while ((buf[i] == ' ') || (buf[i] == '\t')) /* skip whitespace between element name and first attribute */
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
            range_open(&ranges[rng], line, buf+i);
              }
              else {            /* XML end tag */
            if (separator != '>') {
              Rprintf("Warning: no annotations allowed on XML close tag </%s ...> (", &buf[k]);
              encode_print_input_lineno();
              Rprintf(", ignored).\n");
            }
            range_close(&ranges[rng], line - 1); /* end tag belongs to previous line! */
              }
            }
          }
          else {
            /* no appropriate s-attribute declared -> insert tag literally */
            if (!quietly) {
              if (!cl_lexhash_freq(undeclared_sattrs, &buf[k])) {
                Rprintf("s-attribute <%s> not declared, inserted literally (", &buf[k]);
                encode_print_input_lineno();
                Rprintf(", warning issued only once).\n");
                cl_lexhash_add(undeclared_sattrs, &buf[k]);
              }
            }
            buf[i] = separator; /* restore original line, which will be interpreted as token line */
          }
        }
        /* malformed XML tag (no element name found) */
        else if (!quietly) {
          Rprintf("Malformed tag %s, inserted literally (", buf);
          encode_print_input_lineno();
          Rprintf(").\n");
        }
      } /* endif line begins with < */
        
        /* if we haven't handled the line so far, it must be data for the positional attributes */
        if (!handled) {
          encode_add_wattr_line(buf);
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
    } /* endif (this is a line that should be encoded) */
  } /* endwhile (main loop for each line) */
          
  if (verbose) {
    Rprintf("%50s\r", "");       /* clear progress line */
    Rprintf("Total size: %" COMMA_SEP_THOUSANDS_CONVSPEC "d tokens (%.1fM)\n", line, ((float) line) / 1048576);
  }
  
  /* close open regions at end of input; then close file handles for s-attributes */
  for (i = 0; i < range_ptr; i++) {
    SAttEncoder *rng = &ranges[i];
    
    if (! rng->null_attribute) { /* don't attempt to close NULL attribute */
  
  /* This is fairly tricky: When multiple end tags are missing for an attribute declared with recursion (even ":0"),
   we have to call range_close() repeatedly to ensure that the open region at the top level is really closed
   (which happens when rng->recursion_level == 1). At the same time, range_close() will also close the corresponding
   ranges of any implicitly defined attributes (used to resolve recursive embedding and element attributes).
   Therefore, the following code calls range_close() repeatedly until the current range is actually closed.
   It also relies on the ordering of the ranges[] array, where top level attributes always precede their children,
   so they should be closed automatically before cleanup reaches them. If the ordering were different, children might
   be closed directly at first, and the following attempt to close them automatically from within the range_close()
   function would produce highly confusing error messages. To be on the safe side (for some definition of safe :-),
   we _never_ close ranges for implicit attributes, and issue a warning if they're still open when cleanup reaches them. 
   */
  if (rng->automatic) {     /* implicitly generated s-attributes should have been closed automatically */
  if (!quietly && rng->is_open) {
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
        Rprintf("Warning: missing </%s> tag inserted at end of input.\n", rng->name);
      
      /* close open region; this will automatically close children from recursion and element attributes;
       if multiple end tags are missing, we have to call range_close() repeatedly until we reach the top level */
      while (rng->is_open) { /* should _not_ create an infinite loop, I hope */
      range_close(rng, line - 1);
      }
    }
    
    if (!quietly && (rng->max_recursion >= 0) && (rng->element_drop_count > 0)) {
      Rprintf("%7d <%s> regions dropped because of deep nesting.\n",
              rng->element_drop_count, rng->name);
    }
  }
  
  /* close file handles for s-attributes */
  if (EOF == fclose(rng->fd)) {
    perror("fclose() failed");
    encode_error(strdup("Error writing .rng file for s-attribute <%s>"), rng->name);
  }
  if (rng->store_values) {
    if (EOF == fclose(rng->avs)) {
      perror("fclose() failed");
      encode_error(strdup("Error writing .avs file for s-attribute <%s>"), rng->name);
    }
    if (EOF == fclose(rng->avx)) {
      perror("fclose() failed");
      encode_error(strdup("Error writing .avx file for s-attribute <%s>"), rng->name);
    }
  }
  
    }
  } /* endfor: closing each open region and s-attribute filehandle for each Range */
  
  /* close file handles for positional attributes */
  wattr_close_all();
  
  /* if registry_file != NULL, write appropriate registry entry to file named <registry_file> */
  if (registry_file != NULL) {
    encode_generate_registry_file(registry_file);
  }
  
  if (debugmode)
    encode_print_time(strdup("Done"));

  return nr_input_files;
}
