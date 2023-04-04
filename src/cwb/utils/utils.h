/* Do not edit by hand! This file has been automatically generated */

extern Corpus *corpus;
typedef struct _SAttEncoder SAttEncoder;


/* functions defined in cwb-encode.c */

void Rprintf(const char *,...);
char *encode_strtok(char *s,const char *delim);
void encode_print_time(char *msg);
void encode_print_input_lineno(void);
int encode_error(char *format,...);
cl_string_list encode_scan_directory(char *dir);
int s_att_builder_find(char *name);
void s_att_print_registry_line(s_att_builder *encoder,FILE *dst,int print_comment);
s_att_builder *s_att_declare(char *name,char *directory,int store_values,int null_attribute);
void s_att_close_range(s_att_builder *encoder,int end_pos);
void s_att_open_range(s_att_builder *encoder,int start_pos,char *annot);
int p_att_builder_find(char *name);
int p_att_declare(char *name,char *directory,int nr_buckets);
void p_att_builder_close_all(void);
void encode_add_p_attr_line(char *str);
int encode_get_input_line(char *buffer,int bufsize);
void encode_generate_registry_file(char *registry_file);
int looks_like_a_token(const char *line);
int cwb_encode_worker(cl_string_list input_files);


/* functions defined in cwb-compress-rdx.c */

void Rprintf(const char *,...);
void compressrdx_usage(char *msg,int error_code);
void compressrdx_cleanup(int error_code);
void compress_reversed_index(Attribute *attr,char *output_fn,char *corpus_id,int debug);
void decompress_check_reversed_index(Attribute *attr,char *output_fn,char *corpus_id,int debug);


/* functions defined in cwb-huffcode.c */

void Rprintf(const char *,...);
void print_binary_integer(unsigned int i,int width,FILE *stream);
int compute_code_lengths(Attribute *attr,HCD *hc,char *fname);
int decode_check_huff(Attribute *attr,char *corpus_id,char *fname);


/* functions defined in cwb-makeall.c */

void Rprintf(const char *,...);
int component_ok(Attribute *attr,ComponentID cid);
int makeall_make_component(Attribute *attr,ComponentID cid);
int validate_revcorp(Attribute *attr);
int makeall_do_attribute(Attribute *attr,ComponentID cid,int validate);


/* functions defined in cwb-decode.c  */
#undef INTERFACE
#define MAX_ATTRS 2048
#define MAX_PRINT_VALUES 1024

typedef struct AVList {
  char *name;                   /**< name of the XML attribute to be displayed */
  Attribute *handle;            /**< CWB handle of the attribute */
  struct AVList *next;          /**< pointer to next element of chain */
} AVList;

typedef struct {
  char *name;                   /**< display name (may be different from CWB attribute name for certain s-attributes) */
  Attribute *handle;            /**< CWB handle of the attribute */
  int type;                     /**< attribute type, same as handle->type */
  AVList av_list;               /**< for XML attribute only: linked list of attribute-value pairs to be displayed in start tag */
} AttSpec;

void decode_print_token_sequence(int start_position,int end_position,Attribute *context,int skip_token);
void decode_expand_context(int *start,int *end,Attribute *context);
void decode_print_surrounding_s_att_values(int position);
void decode_verify_print_value_list(void);
int decode_add_s_attribute(const char *att_spec);
int decode_add_attribute(const char *name,int type,const char *display_name,const char *avspec,int recursion);
Attribute *get_attribute_handle(const char *name,int type);
int decode_attribute_is_in_list(Attribute *handle,AttSpec *att_list,int att_list_size);
void decode_sort_s_att_regions(void);
void decode_print_xml_declaration(void);
const char *decode_string_escape(const char *s);
int is_num(char *s);
void decode_usage(int exit_code);
void cleanup_and_exit(int error_code);
extern int xml_compatible;

typedef enum _output_modes {
  StandardMode, LispMode, EncodeMode, ConclineMode, XMLMode
} OutputMode;
extern OutputMode mode;

extern Attribute *conll_delimiter;
extern int printnum;
extern int maxlast;
extern int last_token;
extern int first_token;
extern int printValuesIndex;
extern Attribute *printValues[MAX_PRINT_VALUES];
extern int N_sar;
extern int sar_sort_index[MAX_ATTRS];

typedef struct {
  const char *name;             /**< display name of the s-attribute */
  int start;                    /**< start and end of region (for sorting) */
  int end;
  const char *annot;            /**< NULL if there is no annotation; otherwise the content of the annotation */
  AVList av_list;               /**< optional linked list of attribute-value pairs to be displayed in a start tag */
} SAttRegion;
extern SAttRegion s_att_regions[MAX_ATTRS];

extern int print_list_index;
extern AttSpec print_list[MAX_ATTRS];
extern Corpus *corpus;
extern char *corpus_name;
extern char *registry_directory;
extern char *progname;
