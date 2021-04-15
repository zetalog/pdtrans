#ifndef __PD_VB_H_INCLUDE__
#define __PD_VB_H_INCLUDE__

#define VB_TAG_HEADER               "Tag             =   "
#define VB_TAG_TAB                  "   "

#define VB_COMMENT                  '\''
#define VB_CONTINUE                 '_'

#define VB_WORD                     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"
#define VB_OPERATERS                "+-=*/\\<>~^&"

#define VB_LINE_NORMAL              0
#define VB_LINE_BEGIN               1   /* Begin */
#define VB_LINE_END                 2   /* End */
#define VB_LINE_SCOPE               3   /* between Begin-End */
#define VB_LINE_BEGIN_PROPERTY      4   /* BeginProperty */
#define VB_LINE_END_PROPERTY        5   /* EndProperty */
#define VB_LINE_SCOPE_PROPERTY      6   /* between BeginProperty-EndProperty */

#define VB_BUTTONS_BUTTON               "Buttons.Button"
#define VB_BUTTONMENUS_BUTTONMENU       "ButtonMenus.ButtonMenu"
#define VB_TABS_TAB                     "Tabs.Tab"
#define VB_BUTTONS                      "Buttons"
#define VB_BUTTONMENUS                  "ButtonMenus"
#define VB_TABS                         "Tabs"
#define VB_COLUMNHEADER                 "ColumnHeader"
#define VB_COLUMNHEADERQ                "ColumnHeader("
#define VB_COLUMNHEADERS                "ColumnHeaders"
#define VB_OBJECT_TOOLTIPTEXT           "Object.ToolTipText"
#define VB_TOOLTIPTEXT                  "ToolTipText"

typedef struct _vb_param_t {
    char *const_header;
    vector_t *cache_properties;
    void (*store_name)(vector_t *properties, char *name);
} *vb_param_t;

typedef struct _vb_process_t {
    struct _pdt_process_t;

    int line_type;
    /** line type stack used for property block */
    vector_t *line_stack;
    int property_depth;

    vector_t *prefix_stack;
    vector_t *property_stack;

    vector_t *cache_properties;
    char *const_header;
    void (*property_storer)(vector_t *properties, char *name);

    /* name value pair cache */
    char *name;
    char *value;
    /* control properties */
    pdt_properties_t current_properties;
} *vb_process_t;

extern pdt_plugin_t pdt_bas_plugin;
extern pdt_plugin_t pdt_cls_plugin;
extern pdt_plugin_t pdt_frm_plugin;
extern pdt_plugin_t pdt_ctl_plugin;
extern pdt_plugin_t pdt_vbs_plugin;

/* VB processor plugins */
pdt_process_t pdt_create_vb_processor(vb_param_t param);
void pdt_destroy_vb_processor(vb_process_t process);
void pdt_parse_vb_word(vb_process_t process, char *word, FILE *stream);
int pdt_scan_vb_word(vb_process_t process, char **bufptr, size_t *n, int *word_begin, FILE *stream);
char *pdt_create_vb_tag(pdt_properties_t properties, int depth);
char *pdt_vb_to_csv(vb_process_t process, const char *quoted);
char *pdt_vb_from_csv(vb_process_t process, const char *quoted);
int pdt_convert_vb_columnheader(char **tag_name, const char *raw, const char *replace);
int pdt_convert_object_tooltiptext(char *tag_name, const char *raw, const char *replace);
int pdt_escape_vb_path(const char *path);
int pdt_convert_vb_tag(char *tag_name, const char *raw, const char *replace);

#endif /* __PD_VB_H_INCLUDE__ */
