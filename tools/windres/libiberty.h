/* Choose a temporary directory to use for scratch files.  */

extern char *choose_temp_base (void);

/* Definitions used by the pexecute routine.  */

#define install_error_msg "installation problem, cannot exec `%s'"

#define PEXECUTE_FIRST   1
#define PEXECUTE_LAST    2
#define PEXECUTE_ONE     (PEXECUTE_FIRST + PEXECUTE_LAST)
#define PEXECUTE_SEARCH  4
#define PEXECUTE_VERBOSE 8

/* Execute a program.  */

extern int pexecute (const char *, char * const *, const char *,
			    const char *, char **, char **, int);

/* Wait for pexecute to finish.  */

extern int pwait (int, int *, int);

void xmalloc_set_program_name (const char *s);


extern int strcasecmp(const char *s1, const char *s2);

void fatal (const char *format, ...);
void non_fatal (const char *format, ...);

extern char *program_name;
