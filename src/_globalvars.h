typedef enum which_app { undef, cqp, cqpcl, cqpserver} which_app_t;
typedef enum _opttype {
  OptInteger, OptString, OptBoolean, OptContext
} OptType;

/**
 * A CQPOption represents a single configuration option for CQP.
 *
 * It does not actually contain the config-option itself; that is held as
 * a global variable somewhere. Instead, it holds metadata about the
 * config-option, including a pointer to the actual variable.
 *
 * It's possible to have two CQPOption objects referring to the same
 * actual variable - in this case the two option names in question
 * are synonymous.
 */
typedef struct _cqpoption {
  char    *opt_abbrev;           /**< Short version of this option's name. */
  char    *opt_name;             /**< Name of this option as referred to in the interactive control syntax */
  OptType  type;                 /**< Data type of this configuration option. */
  void    *address;              /**< Pointer to the actual variable that contains this config option. */
  char    *cdefault;             /**< Default value for this option (string value) */
  int      idefault;             /**< Default value for this option (integer/boolean value) */
  char    *envvar;               /**< The environment variable from which CQP will take a value for this option */
  int      side_effect;          /**< Ref number of the side effect that changing this option has. @see execute_side_effects */
  int      flags;                /**< Flags for this option: the only one currently used is OPTION_VISIBLE_IN_CQP @see OPTION_VISIBLE_IN_CQP */
} CQPOption;
Corpus *corpus;
which_app_t which_app;
int insecure;
int inhibit_activation;
int parse_only;
int verbose_parser;
int show_symtab;
int show_gconstraints;
int show_evaltree;
int show_patlist;
int show_compdfa;
int show_dfa;
int symtab_debug;
int parser_debug;
int tree_debug;
int eval_debug;
int search_debug;
int initial_matchlist_debug;
int simulate_debug;
int activate_cl_debug;
int server_log;
int server_debug;
int snoop;
int private_server;
int server_port;
int localhost;
int server_quit;
int query_lock;
int query_lock_violation;
int enable_macros;
int macro_debug;
int hard_boundary;
int hard_cut;
int auto_subquery;
char *def_unbr_attr;
int query_optimize;
int anchor_number_target;
int anchor_number_keyword;
MatchingStrategy matching_strategy;
char *matching_strategy_name;
int strict_regions;
int use_readline;
int highlighting;
int paging;
char *pager;
char *tested_pager;
char *less_charset_variable;
int use_colour;
int progress_bar;
int pretty_print;
int autoshow;
int timing;
int show_tag_attributes;
int show_targets;
char *printModeString;
char *printModeOptions;
int printNrMatches;
char *printStructure;
char *left_delimiter;
char *right_delimiter;
char *attribute_separator;
char *token_separator;
char *structure_delimiter;
char *registry;
char *data_directory;
int auto_save;
int save_on_exit;
char *cqp_init_file;
char *macro_init_file;
char *cqp_history_file;
int write_history_file;
int batchmode;
int silent;
char *default_corpus;
char *query_string;
int UseExternalSort;
char *ExternalSortCommand;
int UseExternalGroup;
char *ExternalGroupCommand;
int user_level;
int output_binary_ranges;
int child_process;
ContextDescriptor CD;
int handle_sigpipe;
char *progname;
char *licensee;
FILE *batchfh;
#define OPTION_VISIBLE_IN_CQP    1

/** Default value for the HardBoundary configuration option. */
#define DEFAULT_HARDBOUNDARY 500

/** Default value for the context scope configuration option (counted in characters) */
#define DEFAULT_CONTEXT 25

#define DEFAULT_LOCAL_PATH_ENV_VAR "CQP_LOCAL_CORP_DIR"

#define CQP_FALLBACK_PAGER "more"
extern CQPOption cqpoptions;
