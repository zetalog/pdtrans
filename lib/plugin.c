#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

pdt_plugin_t *pdt_plugins[] =
{
    &pdt_bas_plugin,
    &pdt_ctl_plugin,
    &pdt_frm_plugin,
    &pdt_cls_plugin,
    &pdt_asp_plugin,
    &pdt_inc_plugin,
    &pdt_vbs_plugin,
    &pdt_js_plugin,
    &pdt_sql_plugin,
    &pdt_cpp_plugin,
    &pdt_c_plugin,
    &pdt_hpp_plugin,
    &pdt_h_plugin,
    &pdt_rc_plugin,
    &pdt_rc2_plugin,
    NULL,
};

/* plugin function */
unsigned int pdt_escape_sufix_path(const char *file)
{
    int sufix_index;
    unsigned int result = 0;

    for (sufix_index = 0; pdt_plugins[sufix_index] != NULL; sufix_index++)
    {
        if (!pdt_plugins[sufix_index]->activated)
            continue;

        if (stricmp(pdt_plugins[sufix_index]->sufix, ".bas") == 0 &&
            pdt_plugins[sufix_index]->dir_filter(file))
            result |= PDT_ESCAPE_VB;

        if (stricmp(pdt_plugins[sufix_index]->sufix, ".asp") == 0 &&
            pdt_plugins[sufix_index]->dir_filter(file))
            result |= PDT_ESCAPE_ASP;

        if (stricmp(pdt_plugins[sufix_index]->sufix, ".sql") == 0 &&
            pdt_plugins[sufix_index]->dir_filter(file))
            result |= PDT_ESCAPE_SQL;
    }

    return result;
}

int pdt_is_sufix_escape(char *sufix, int escape)
{
    if ((stricmp(sufix, ".bas") == 0) || 
        (stricmp(sufix, ".ctl") == 0) ||
        (stricmp(sufix, ".frm") == 0) ||
        (stricmp(sufix, ".cls") == 0))
    {
        return escape & PDT_ESCAPE_VB;
    }
    if ((stricmp(sufix, ".asp") == 0) || 
        (stricmp(sufix, ".inc") == 0) ||
        (stricmp(sufix, ".js") == 0) ||
        (stricmp(sufix, ".vbs") == 0))
    {
        return escape & PDT_ESCAPE_ASP;
    }
    if ((stricmp(sufix, ".sql") == 0))
    {
        return escape & PDT_ESCAPE_SQL;
    }

    return 0;
}

int pdt_find_sufix_processor(char *file)
{
    int sufix_index;
    char *sufix = get_ext(file);

    for (sufix_index = 0; pdt_plugins[sufix_index] != NULL; sufix_index++)
    {
        if ((stricmp(pdt_plugins[sufix_index]->sufix, sufix) == 0) &&
            pdt_plugins[sufix_index]->activated)
            return TRUE;
    }

    return FALSE;
}

pdt_param_t pdt_set_sufix_processor(char *sufix, pdt_param_t param)
{
    int sufix_index;

    for (sufix_index = 0; pdt_plugins[sufix_index] != NULL; sufix_index++)
    {
        if ((stricmp(pdt_plugins[sufix_index]->sufix, sufix) == 0) &&
            pdt_plugins[sufix_index]->activated)
        {
            pdt_param_t old_param = pdt_plugins[sufix_index]->create_param;
            pdt_plugins[sufix_index]->create_param = param;
            return old_param;
        }
    }

    return 0;
}

void pdt_enable_sufix_processor(char *sufix, pdt_handle_t vector, char *language,
                                char *translation, pdt_param_t param)
{
    int sufix_index;

    assert(vector && language);

    for (sufix_index = 0; pdt_plugins[sufix_index] != NULL; sufix_index++)
    {
        pdt_plugins[sufix_index]->csv_resources = vector;

        if (_stricmp(pdt_plugins[sufix_index]->sufix, sufix) == 0)
        {
            pdt_plugins[sufix_index]->activated = 1;
            pdt_plugins[sufix_index]->compare_language = language;
            pdt_plugins[sufix_index]->translate_language = translation;
            pdt_plugins[sufix_index]->create_param = param;
        }
    }
}

void pdt_modify_sufix_replacement(char *sufix, int *replace_mode, char **format)
{
    int sufix_index;

    assert(sufix && replace_mode && format);

    for (sufix_index = 0; pdt_plugins[sufix_index] != NULL; sufix_index++)
    {
        if (_stricmp(pdt_plugins[sufix_index]->sufix, sufix) == 0)
        {
            int int_swap;
            char *string_swap;

#define SWAP(older, newer, swap)    \
    do {                            \
        swap = older;               \
        older = newer;              \
        newer = swap;               \
    } while (0);
            SWAP(pdt_plugins[sufix_index]->replace_type, *replace_mode, int_swap);
            SWAP(pdt_plugins[sufix_index]->replace_format, *format, string_swap);
        }
    }
}
