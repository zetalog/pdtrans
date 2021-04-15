#ifndef __PD_RC_H_INCLUDE__
#define __PD_RC_H_INCLUDE__

#define RC_OPERATORS            "+-=*/<>"

typedef struct _rc_process_t {
    struct _pdt_process_t;
    
    /* Whether we are supressing lines from cpp (including windows.h or
       headers from your C sources may bring in externs and typedefs).
       When active, we return IGNORED_TOKEN, which lets us ignore these
       outside of resource constructs.  Thus, it isn't required to protect
       all the non-preprocessor lines in your header files with #ifdef
       RC_INVOKED.  It also means your RC file can't include other RC
       files if they're named "*.h".  Sorry.  Name them *.rch or whatever.  */
    int suppress_cpp_data;
#define MAYBE_RETURN(p, x) return p->suppress_cpp_data ? IGNORED_TOKEN : (x)

    /* Whether we are in rcdata mode, in which we returns the lengths of
       strings.  */
    int rcdata_mode;
} *rc_process_t;

extern pdt_plugin_t pdt_rc_plugin;
extern pdt_plugin_t pdt_rc2_plugin;


/* RC processor plugins */
pdt_process_t pdt_create_rc_processor(pdt_param_t param);
void pdt_destroy_rc_processor(rc_process_t process);
int pdt_scan_rc_word(rc_process_t process, char **bufptr, size_t *n, int *word_begin, FILE *stream);
void pdt_parse_rc_word(rc_process_t process, char *word, FILE *stream);
char *pdt_rc_to_csv(rc_process_t process, const char *word_in);
char *pdt_rc_from_csv(rc_process_t process, const char *word_in);

#endif /* __PD_RC_H_INCLUDE__ */