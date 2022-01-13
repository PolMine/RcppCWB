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
  makeall_do_attribute(attribute, cid, validate);

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
    compressrdx_cleanup(1);
  }
  
  if ((attr = find_attribute(corpus, attr_name, ATT_POS, NULL)) == NULL) {
    Rprintf("Attribute %s.%s doesn't exist. Aborted.\n", corpus_id, attr_name);
    compressrdx_cleanup(1);
  }
  
  compress_reversed_index(attr, output_fn, corpus_id, debug);
  if (! i_want_to_believe) decompress_check_reversed_index(attr, output_fn, corpus_id, debug);
  
  compressrdx_cleanup(0);
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

/**
 * WAttr object: represents a P-attribute being encoded.
 *
 * TODO should probably be called a PAttr
 */
typedef struct {
  char *name;                   /**< TODO */
cl_lexhash lh;                /**< String hash object containing the lexicon for the encoded P attrbute */
int position;                 /**< Byte index of the lexicon file in progress; contains total number of bytes
 written so far (== the beginning of the -next- string that is written) */
int feature_set;              /**< Boolean: is this a feature set attribute? => validate and normalise format */
FILE *lex_fd;                 /**< file handle of lexicon component */
FILE *lexidx_fd;              /**< file handle of lexicon index component */
FILE *corpus_fd;              /**< file handle of corpus component */
} WAttr;

/** A global array for keeping track of P-attributes being encoded. */
WAttr wattrs[MAXRANGES];
/** @see wattrs */
int wattr_ptr = 0;



// [[Rcpp::export(name=".cwb_encode")]]
int cwb_encode(SEXP regfile, SEXP data_dir, SEXP vrt_dir, Rcpp::StringVector p_attributes, Rcpp::StringVector s_attributes_anno, Rcpp::StringVector s_attributes_noanno){
  
  directory = strdup(Rcpp::as<std::string>(data_dir).c_str());
  registry_file = strdup(Rcpp::as<std::string>(regfile).c_str());

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
  cl_string_list vrt_files = encode_scan_directory(strdup(Rcpp::as<std::string>(vrt_dir).c_str()));

  nr_input_files = cwb_encode_worker(vrt_files);
  return nr_input_files;
}
