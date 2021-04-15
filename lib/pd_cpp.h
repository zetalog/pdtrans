#ifndef __PD_CPP_H_INCLUDE__
#define __PD_CPP_H_INCLUDE__

#define CPP_OPERATORS            "+-=*/<>"

typedef struct _vc_param_t {
    char *const_header;
    vector_t *cache_properties;
    void (*store_name)(vector_t *properties, char *name);
} *vc_param_t;

typedef struct _cpp_process_t {
    struct _pdt_process_t;

} *cpp_process_t;

extern pdt_plugin_t pdt_cpp_plugin;
extern pdt_plugin_t pdt_c_plugin;
extern pdt_plugin_t pdt_hpp_plugin;
extern pdt_plugin_t pdt_h_plugin;


/* CPP processor plugins */
pdt_process_t pdt_create_cpp_processor(pdt_param_t param);
void pdt_destroy_cpp_processor(cpp_process_t process);
int pdt_scan_cpp_word(cpp_process_t process, char **bufptr, size_t *n, int *word_begin, FILE *stream);
void pdt_parse_cpp_word(cpp_process_t process, char *word, FILE *stream);
char *pdt_cpp_to_csv(cpp_process_t process, const char *word_in);
char *pdt_cpp_from_csv(cpp_process_t process, const char *word_in);

#endif /* __PD_CPP_H_INCLUDE__ */