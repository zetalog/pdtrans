#ifndef __PD_JS_H_INCLUDE__
#define __PD_JS_H_INCLUDE__

#define JS_OPERATORS            "+-=*/<>"

typedef struct _js_process_t {
    struct _pdt_process_t;

} *js_process_t;

extern pdt_plugin_t pdt_js_plugin;

/* JS processor plugins */
pdt_process_t pdt_create_js_processor(pdt_param_t param);
void pdt_destroy_js_processor(js_process_t process);
int pdt_scan_js_word(js_process_t process, char **bufptr, size_t *n, int *word_begin, FILE *stream);
void pdt_parse_js_word(js_process_t process, char *word, FILE *stream);
char *pdt_js_to_csv(js_process_t process, const char *word_in);
char *pdt_js_from_csv(js_process_t process, const char *word_in);

#endif /* __PD_JS_H_INCLUDE__ */