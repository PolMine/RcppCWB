/* ---------------------------------------------------------------------- */

/** User privileges of new files (octal format) */
#define UMASK              0644

/** String containing the characters that can function as field separators */
#define FIELDSEPS  (char*)"\t\n"

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

char *field_separators = FIELDSEPS;     /**< string containing the characters that can function as field separators */
char *undef_value = (char *)CWB_PA_UNDEF_VALUE; /**< string used as value of P-attributes when a value is missing,
                                             ie if a tab-delimited field is empty */
int debugmode = 0;                          /**< debug mode on or off? */
int quietly = 0;                         /**< hide messages */
int verbose = 0;                        /**< show progress (this is _not_ the opposite of silent!) */
int xml_aware = 0;                      /**< substitute XML entities in p-attributes & ignore <? and <! lines */
int skip_empty_lines = 0;               /**< skip empty lines when encoding? */
int auto_null = 0;                      /**< auto-declare null attributes for unknown XML tags */
unsigned line = 0;                      /**< corpus position currently being encoded (ie cpos of _next_ token);
                                             unsigned so it doesn't wrap after first 2^31 tokens
                                             and thus we can abort encoding when corpus size is exceeded */
int strip_blanks = 0;                   /**< strip leading and trailing blanks from input and token annotations */
cl_string_list input_files = NULL;      /**< list of input file(s) (-f option(s)) */
int nr_input_files = 0;                 /**< number of input files (length of list after option processing) */
int current_input_file = 0;             /**< index of input file currently being processed */
char *current_input_file_name = NULL;   /**< filename of current input file, for error messages */
FILE *input_fh = NULL;                  /**< file handle for current input file (or pipe) (text mode!) */
unsigned long input_line = 0;           /**< input line number (reset for each new file) for error messages */
char *registry_file = NULL;             /**< if set, auto-generate registry file named {registry_file}, listing declared attributes */
char *directory = NULL;                 /**< corpus data directory (no longer defaults to current directory) */
const char *encoding_charset_name = "latin1";  /**< character set label that is inserted into the registry file */
CorpusCharset encoding_charset;         /**< a charset object to be generated from corpus_character_set */
int clean_strings = 0;                  /**< clean up input strings by replacing invalid bytes with '?' */
int numbered = 0;                       /**< alternative input mode with token lines numbered in first column */
int encode_token_numbers = 0;           /**< whether token numbers in this input mode are encoded in a p-attribute */
char *conll_sentence_attribute = NULL;  /**< encode blank lines as sentence breaks in this attribute */

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
extern s_att_builder s_encoder[MAX_ATTRIBUTES];
/** @see s_encoder */
int s_encoder_ix = 0;

s_att_builder *conll_sentence_satt = NULL;  /**< optional hidden s-attribute for encoding blank lines as sentence breaks (-L option) */

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
extern p_att_builder p_encoder[MAX_ATTRIBUTES];
/** @see p_encoder */
int p_encoder_ix = 0;

/**
 * lookup hash for undeclared s-attributes and s-attributes declared with -S that
 * have annotations (which will be ignored), so warnings are issued only once
 */
cl_lexhash undeclared_sattrs = NULL;


