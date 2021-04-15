#ifndef __PD_SQL_H_INCLUDE__
#define __PD_SQL_H_INCLUDE__

#define SQL_LINE_NORMAL             0

extern pdt_plugin_t pdt_sql_plugin;

/* SQL processor plugins */
pdt_process_t pdt_create_sql_processor(pdt_param_t param);
void pdt_parse_sql_word(pdt_process_t process, char *word, FILE *stream);
int pdt_scan_sql_word(pdt_process_t process, char **bufptr, size_t *n, int *word_begin, FILE *stream);
void pdt_destroy_sql_processor(pdt_process_t process);
char *pdt_sql_from_csv(pdt_process_t process, const char *word_in);
char *pdt_sql_to_csv(pdt_process_t process, const char *word_in);
int pdt_escape_sql_path(const char*);

#endif /* __PD_SQL_H_INCLUDE__ */
