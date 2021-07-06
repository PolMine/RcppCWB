extern Corpus *corpus;
int cleanup(int error_code);
void compress_reversed_index(Attribute *attr, char *output_fn, char *corpus_id, int debug);
void decompress_check_reversed_index(Attribute *attr, char *output_fn, char *corpus_id, int debug);
int decode_check_huff(Attribute *attr, char* corpus_id, char *fname);
int compute_code_lengths(Attribute *attr, HCD *hc, char *fname);
int do_attribute(Attribute *attr, ComponentID cid, int validate);


/* --------------- functions in cwb_encode.c ------------------------------ */

typedef struct _SAttEncoder SAttEncoder;
char * encode_strtok(char *s, const char *delim);
void encode_print_time(char *msg);
void encode_usage(void);
void encode_print_input_lineno(void);
void encode_error(char *format, ...);
cl_string_list encode_scan_directory(char *dir);
int range_find(char *name);
void range_print_registry_line(SAttEncoder *rng, FILE *fd, int print_comment);
SAttEncoder *range_declare(char *name, char *directory, int store_values, int null_attribute);
void range_close(SAttEncoder *rng, int end_pos);
void range_open(SAttEncoder *rng, int start_pos, char *annot);
int wattr_find(char *name);
int wattr_declare(char *name, char *directory, int nr_buckets);
void wattr_close_all(void);
void encode_add_wattr_line(char *str);
int encode_get_input_line(char *buffer, int bufsize);
void encode_generate_registry_file(char *registry_file);
void string_chomp(char *s); /* replaces cl_string_chomp */

/* ---------------------------------------------------------------------- */
