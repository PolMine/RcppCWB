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
extern Corpus *corpus;
extern which_app_t which_app;
extern int insecure;
extern int inhibit_activation;
extern int parse_only;
extern int verbose_parser;
extern int show_symtab;
extern int show_gconstraints;
extern int show_evaltree;
extern int show_patlist;
extern int show_compdfa;
extern int show_dfa;
extern int symtab_debug;
extern int parser_debug;
extern int tree_debug;
extern int eval_debug;
extern int search_debug;
extern int initial_matchlist_debug;
extern int simulate_debug;
extern int activate_cl_debug;
extern int server_log;
extern int server_debug;
extern int snoop;
extern int private_server;
extern int server_port;
extern int localhost;
extern int server_quit;
extern int query_lock;
extern int query_lock_violation;
extern int enable_macros;
extern int macro_debug;
extern int hard_boundary;
extern int hard_cut;
extern int auto_subquery;
extern char *def_unbr_attr;
extern int query_optimize;
extern int anchor_number_target;
extern int anchor_number_keyword;
extern MatchingStrategy matching_strategy;
extern char *matching_strategy_name;
extern int strict_regions;
extern int use_readline;
extern int highlighting;
extern int paging;
extern char *pager;
extern char *tested_pager;
extern char *less_charset_variable;
extern int use_colour;
extern int progress_bar;
extern int pretty_print;
extern int autoshow;
extern int timing;
extern int show_tag_attributes;
extern int show_targets;
extern char *printModeString;
extern char *printModeOptions;
extern int printNrMatches;
extern char *printStructure;
extern char *left_delimiter;
extern char *right_delimiter;
extern char *attribute_separator;
extern char *token_separator;
extern char *structure_delimiter;
extern char *registry;
extern char *data_directory;
extern int auto_save;
extern int save_on_exit;
extern char *cqp_init_file;
extern char *macro_init_file;
extern char *cqp_history_file;
extern int write_history_file;
extern int batchmode;
extern int silent;
extern char *default_corpus;
extern char *query_string;
extern int UseExternalSort;
extern char *ExternalSortCommand;
extern int UseExternalGroup;
extern char *ExternalGroupCommand;
extern int user_level;
extern int output_binary_ranges;
extern int child_process;
extern ContextDescriptor CD;
extern int handle_sigpipe;
extern char *progname;
extern char *licensee;
extern FILE *batchfh;
#define OPTION_VISIBLE_IN_CQP    1

/** Default value for the HardBoundary configuration option. */
#define DEFAULT_HARDBOUNDARY 500

/** Default value for the context scope configuration option (counted in characters) */
#define DEFAULT_CONTEXT 25

#define DEFAULT_LOCAL_PATH_ENV_VAR "CQP_LOCAL_CORP_DIR"

#define CQP_FALLBACK_PAGER "more"
extern CQPOption cqpoptions;
