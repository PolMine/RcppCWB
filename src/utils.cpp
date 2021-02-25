extern "C" {
  #include "cl_min.h"
  #include <attributes.h>
  #include "utils.h"
}


  
#include <Rcpp.h>
using namespace Rcpp;

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



// [[Rcpp::export(name=".cwb_encode")]]
int cwb_encode(SEXP registry_dir, SEXP dir){
  
  /* the following code is copied from cwb_encode.c */
  
  /* ---------------------------------------------------------------------- */
  
  /* global variables representing configuration */
  
  char *field_separators = FIELDSEPS;     /**< string containing the characters that can function as field separators */
  char *undef_value = UNDEF_VALUE;        /**< string used as value of P-attributes when a value is missing
   ie if a tab-delimited field is empty */
  int debugmode = 0;                          /**< debug mode on or off? */
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
  char *corpus_character_set = strdup("latin1");  /**< character set label that is inserted into the registry file */
  CorpusCharset encoding_charset;         /**< a charset object to be generated from corpus_character_set */
  int clean_strings = 0;                  /**< clean up input strings by replacing invalid bytes with '?' */
  
  /* ---------------------------------------------------------------------- */
  
  
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
  corpus_character_set = cl_charset_name_canonical(strdup("UTF-8"));
  directory = strdup(Rcpp::as<std::string>(dir).c_str());
  skip_empty_lines++; /* -s: skip empty lines */
  
  /* -S: declare s-attribute without annotations */
  /* -V: declare s-attribute with annotations */
  
  wattr_declare(strdup("word"), directory, 0);
  wattr_declare(strdup("pos"), directory, 0);
  wattr_declare(strdup("lemma"), directory, 0);

  input_files = cl_new_string_list();
  cl_string_list dir_files;
  dir_files = encode_scan_directory(directory);
  
  int l;
  l = cl_string_list_size(dir_files);
  for (i = 0; i < l; i++) {
    cl_string_list_append(input_files, cl_string_list_get(dir_files, i));
  }
  cl_delete_string_list(dir_files); /* allocated strings have been moved into input_files, so don't free() them */
  
  nr_input_files = cl_string_list_size(input_files);
  
  return nr_input_files;                        /* to keep gcc from complaining */
}
