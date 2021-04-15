#ifndef __PD_RESOURCE_H_INCLUDE__
#define __PD_RESOURCE_H_INCLUDE__

typedef struct _pdt_resource_t {
    char *language;
    vector_t *strings;

    /* private members */
    void *handler;       /* user handler for processing */
} pdt_resource_t, *PDT_RESOURCE;

/* resource handlers */
pdt_resource_t *pdt_create_resource(char *language, void *handler);
void pdt_destroy_resource(pdt_resource_t *resource);
int pdt_compare_resource(pdt_resource_t *resource1, pdt_resource_t *resource2);

vector_t *pdt_parse_csv_quoted(char *line);

/* resource table handlers */
int pdt_find_csv_locale(vector_t *resources, char *language);
int pdt_append_locale_string(vector_t *vector, char *language, char *value);
void pdt_set_locale_string(vector_t *vector, char *language, int index, char *value);
char *pdt_get_locale_string(vector_t *vector, char *language, int index);
void pdt_copy_resource_row(vector_t *from, vector_t *to, int from_index, int to_index);

char *pdt_format_resource(char *format, char *id, char *constant);
char *pdt_combine_resource_prefix(vector_t *prefixes, char *name);

#endif /* __PD_RESOURCE_H_INCLUDE__ */
