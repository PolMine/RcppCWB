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


