#ifndef __PD_MISC_H_INCLUDE__
#define __PD_MISC_H_INCLUDE__

#define PDT_TERMINATOR      '\0'
#define PDT_QUOTER          '\"'
#define PDT_SPACES          " \r\n\t"

#define INCREMENT           10
#define BUFFER_SIZE         1024
#define MIN_CHUNK           64

void chomp(char *line);
char *get_ext(const char *path);
void strupper(char *string);
char *stristr(const char *string, const char *set);

/* utilities for safe scanner */
int create_buffer(char **bufptr, int *n, char **posptr, int *avail);
int read_char(FILE *stream, char **bufptr, int *n, char **posptr, int *avail, char *pchar);

int pdt_must_localize(char *string);
int pdt_copy_file(const char *src_file, const char *dst_file);
int pdt_compare_dir(const char *src_path, const char *dst_path);
int pdt_is_dir(const char *file);
void pdt_output_word(char *block, int size, char *word, FILE *stream);

char *pdt_include_to_path(const char *ref_path, const char *include);
char *pdt_include_from_path(const char *ref_path, const char *path);
char *pdt_get_absolute_path(const char *path);
int pdt_is_file(const char *file);

#endif /* __PD_MISC_H_INCLUDE__ */
