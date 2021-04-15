#ifndef __PD_ASP_H_INCLUDE__
#define __PD_ASP_H_INCLUDE__

/* blocks for ASP scanning */
#define ASP_BLOCK_HTML              0   /* normal HTML */
#define ASP_BLOCK_SCRIPT            1   /* <%  %> */
#define ASP_BLOCK_COMMENT           2   /* <!--  --> */

#define ASP_SYNTAX_LANGUAGE         0   /* <%  %> */
#define ASP_SYNTAX_PROPERTY         1   /* <%@  %> */
#define ASP_SYNTAX_WRITER           2   /* <%=  %> */
#define ASP_SYNTAX_COMMENT          3   /* <%--  %> */

#define ASP_CONTEXT_NORMAL          0   /* need not do things */
#define ASP_CONTEXT_LANGUAGE        1   /* <%@ LANGUAGE %> */
#define ASP_CONTEXT_INCLUDE         2   /* <!-- #INCLUDE --> */
#define ASP_CONTEXT_CODE            3   /* must process quoted string */

typedef struct _asp_param_t {
    unsigned long language;
    char *message_include;
} *asp_param_t;

typedef struct _asp_process_t {
    struct _pdt_process_t;

    int block_type;
    int need_include;

    int syntax_type;
    int context_type;

    int include_output;
    char *message_include;
    /** it will not be LANGUAGE_UNKNOWN */
    int language_type;
    pdt_process_t syntax_parser;
} *asp_process_t;

extern pdt_plugin_t pdt_asp_plugin;
extern pdt_plugin_t pdt_inc_plugin;

/* ASP processor plugins */
pdt_process_t pdt_create_asp_processor(asp_param_t param);
void pdt_destroy_asp_processor(asp_process_t process);
int pdt_scan_asp_word(asp_process_t process, char **bufptr, size_t *n, int *word_begin, FILE *stream);
void pdt_parse_asp_word(asp_process_t process, char *word, FILE *stream);
int pdt_escape_asp_path(const char *path);
char *pdt_asp_from_csv(asp_process_t process, const char *word);
char *pdt_asp_to_csv(asp_process_t process, const char *word);
void pdt_create_asp_syntax(asp_process_t process, int language_type);
void pdt_destroy_asp_syntax(asp_process_t process);
char *pdt_output_asp_include(asp_process_t process);

#endif /* __PD_ASP_H_INCLUDE__ */
