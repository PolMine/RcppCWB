/* Do not edit by hand! This file has been automatically generated */

/* functions defined in cwb-encode.c */

void Rprintf(const char *,...);
void string_chomp(char *s);
char *encode_strtok(char *s,const char *delim);
void encode_print_time(char *msg);
void encode_print_input_lineno(void);
void encode_error(char *format,...);
cl_string_list encode_scan_directory(char *dir);
int range_find(char *name);
void range_print_registry_line(SAttEncoder *rng,FILE *fd,int print_comment);
SAttEncoder *range_declare(char *name,char *directory,int store_values,int null_attribute);
void range_close(SAttEncoder *rng,int end_pos);
void range_open(SAttEncoder *rng,int start_pos,char *annot);
int wattr_find(char *name);
int wattr_declare(char *name,char *directory,int nr_buckets);
void wattr_close_all(void);
void encode_add_wattr_line(char *str);
int encode_get_input_line(char *buffer,int bufsize);
void encode_generate_registry_file(char *registry_file);
int cwb_encode_worker(cl_string_list dir_files);


/* functions defined in cwb-compress-rdx.c */

void Rprintf(const char *,...);
void compressrdx_usage(char *msg,int error_code);
void compressrdx_cleanup(int error_code);
void compress_reversed_index(Attribute *attr,char *output_fn,char *corpus_id,int debug);
void decompress_check_reversed_index(Attribute *attr,char *output_fn,char *corpus_id,int debug);


/* functions defined in cwb-huffcode.c */

void Rprintf(const char *,...);
void huffcode_usage(char *msg,int error_code);
void bprintf(unsigned int i,int width,FILE *stream);
void dump_heap(int *heap,int heap_size,int node,int indent);
void print_heap(int *heap,int heap_size,char *title);
int WriteHCD(char *filename,HCD *hc);
int ReadHCD(char *filename,HCD *hc);
int compute_code_lengths(Attribute *attr,HCD *hc,char *fname);
int decode_check_huff(Attribute *attr,char *corpus_id,char *fname);


/* functions defined in cwb-makeall.c */

void Rprintf(const char *,...);
int component_ok(Attribute *attr,ComponentID cid);
void makeall_make_component(Attribute *attr,ComponentID cid);
int validate_revcorp(Attribute *attr);
int makeall_do_attribute(Attribute *attr,ComponentID cid,int validate);


