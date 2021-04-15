#ifndef __PD_PLUGIN_H_INCLUDE__
#define __PD_PLUGIN_H_INCLUDE__

/* replace type masks */
#define PDT_REPLACE_CONST           1
#define PDT_REPLACE_TRANSLATE       2
#define PDT_REPLACE_FORMAT          4

#define PDT_ESCAPE_VB               1
#define PDT_ESCAPE_ASP              2
#define PDT_ESCAPE_SQL              4

typedef int (*pdt_scanner_t)(pdt_process_t process, char **bufptr, size_t *size, int *begin, FILE *stream);
typedef void (*pdt_parser_t)(pdt_process_t process, char *word, FILE *stream);
typedef void (*pdt_destroyer_t)(void *);
typedef pdt_process_t (*pdt_creator_t)(pdt_param_t);
typedef char *(*pdt_convert_t)(pdt_process_t process, const char *quoted);
typedef int (*pdt_escape_t)(const char*);

/* processor plugin factories */
typedef struct _pdt_plugin_t {
    char *sufix;
    pdt_creator_t parser_creator;
    pdt_escape_t dir_filter;

    /** is this plugin activated */
    int activated;
    /** base language name to compare with */
    char *compare_language;
    /** target translation language name */
    char *translate_language;

    /** replace type, defined in pd_trans.h */
    int replace_type;
    /** replacement string format */
    char *replace_format;

    /** CSV resource handle */
    vector_t *csv_resources;

    pdt_param_t create_param;
} pdt_plugin_t;

extern pdt_plugin_t *pdt_plugins[];

typedef struct _pdt_process_t {
    /* following members should be maintained by sufix plugin container */
    char *compare_language;
    char *translate_language;
    int generate_const;

    char *input;
    char *output;
    int mode;

    vector_t *block_strings;
    int filtered;

    vector_t *csv_resources;
    pdt_resource_t *resources;
    pdt_resource_t *constants;
    pdt_resource_t *translate;

    unsigned long replace_type;
    char *replace_format;

    /* following members should be maintained by processor implementations */
    int language;

    /** lexial scanner */
    pdt_scanner_t scanner;
    /** syntax parser */
    pdt_parser_t parser;
    pdt_convert_t to_csv;
    pdt_convert_t from_csv;
    pdt_destroyer_t free_string;
    pdt_destroyer_t destroyer;

    int nblocks;    /* block number of whole text */
    int nwords;     /* word number of whole line */

    int should_draw;
    char *before_string;
    char *replace_current;
    char *after_string;
} *pdt_process_t;

/* per-sufix plugin functions */
pdt_param_t pdt_set_sufix_processor(char *sufix, pdt_param_t param);
int pdt_find_sufix_processor(char *file);
int pdt_is_sufix_escape(char *sufix, int escape);
unsigned int pdt_escape_sufix_path(const char *file);

/* processor internal functions */
void pdt_init_file_processor(pdt_process_t process, char *input, char *output, int mode);
void pdt_set_processor_resources(pdt_process_t process, vector_t *resources, char *language,
                                 char *translation, int use_const, int must_translate);
void pdt_set_processor_replacement(pdt_process_t process, unsigned long replace_type,
                                   char *replace_format);
void pdt_modify_sufix_replacement(char *sufix, int *replace_mode, char **format);

/* for double quoted strings */
char *pdt_processor_replace_word(pdt_process_t process, char *word);
void pdt_draw_csv_step(pdt_process_t process, char *word);
void pdt_draw_csv_end(pdt_process_t process);

/* IMPORTANT: do not call plugin functions directly, use processor functions instead */

#endif /* __PD_PLUGIN_H_INCLUDE__ */
