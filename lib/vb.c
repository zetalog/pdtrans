#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

/*
 * Line scope parsing:
 *
 * create a cache vector for storing pdt_property_t vector
 * scan first word
 * ---- Begin matches:
 * Begin-End depth++ if first word is "Begin"
 *   do property replacement if depth > 0
 * ---- End matches:
 * Begin-End depth-- if first word is "End" and next is NULL
 *   do sources replacement if depth is 0
 *
 * following is End Scope should be treated in the sources:
 *
 * Enum
 * Type
 * Sub
 * Function
 * If
 * With
 * Select
 * Property
 */

/*
 * Property replacement (Begin-End depth > 0):
 *
 * ---- Begin matches:
 * store current pdt_property_t vector into cache vector's using depth as index
 * create pdt_property_t vector as current for Begin Scope
 * scan word before and after "="
 * if value must be localized
 * {
 *     store following pdt_property_t:
 *         property name
 *         found ID for the value
 *     store pdt_property_t to vector
 * }
 * ---- End matches:
 * output Tag line before End Scope if element count > 0
 */

/*
 * Sources replacement (Begin-End depth = 0):
 *
 * create pdt_replace_t vector for source line (Begin-End depth is 0)
 * scan quoted string
 * if string must be localized
 * {
 *     raw string's begin-end index
 *     found ID or Const for the string
 *     replace word
 * }
 * output replaced line
 */

void pdt_destroy_vb_processor(vb_process_t process)
{
    if (process)
    {
        if (process->property_stack)
            destroy_vector(process->property_stack);
        if (process->prefix_stack)
            destroy_vector(process->prefix_stack);
        if (process->line_stack)
            destroy_vector(process->line_stack);
        if (process->name)
            free(process->name);
        if (process->value)
            free(process->value);

        if (process->before_string)
            free(process->before_string);
        if (process->after_string)
            free(process->after_string);
        if (process->replace_current)
            free(process->replace_current);

        free(process);
    }
}

/* IMPORTANT: do not call this directly, use pdt_create_file_processor instead */
pdt_process_t pdt_create_vb_processor(vb_param_t param)
{
    vb_process_t process = NULL;

    process = malloc(sizeof (struct _vb_process_t));
    if (process)
    {
        process->language = LANGUAGE_VB;
        process->scanner = (pdt_scanner_t)pdt_scan_vb_word;
        process->parser = (pdt_parser_t)pdt_parse_vb_word;
        process->destroyer = (pdt_destroyer_t)pdt_destroy_vb_processor;

        process->to_csv = (pdt_convert_t)pdt_vb_to_csv;
        process->from_csv = (pdt_convert_t)pdt_vb_from_csv;
        process->free_string = free;

        process->line_type = VB_LINE_NORMAL;
        process->line_stack = create_vector(sizeof(vector_t), NULL, NULL);
        process->nblocks = 0;

        process->name = NULL;
        process->value = NULL;

        /* save parameters for constant header generation */
        if (param)
        {
            process->const_header = param->const_header;
            process->cache_properties = param->cache_properties;
            process->property_storer = param->store_name;
        }
        else
        {
            process->const_header = NULL;
            process->cache_properties = NULL;
            process->property_storer = NULL;
        }

        process->property_depth = 0;
        process->current_properties = NULL;
        process->property_stack = create_vector(sizeof(vector_t), destroy_vector, compare_vector);
        process->prefix_stack = create_vector(sizeof(vector_t), free, _stricmp);
    }

    return (pdt_process_t)process;
}

int pdt_convert_vb_columnheader(char **ptag_name, const char *raw, const char *replace)
{
    char *position = NULL;
    char *pnew_tag_name = NULL;

    position = stristr(*ptag_name, raw);
    if (position)
    {
        char *dst = NULL;
        char *src = NULL;
        int old_length = strlen(*ptag_name);

        //strlen don't count the tailed \0 !
        *ptag_name = realloc(*ptag_name, (old_length + 1 + 1));
        assert(*ptag_name);
        
        dst = *ptag_name + old_length + 1;
        do
        {
            *dst = *(dst - 1);
            dst--;
        }
        while (dst != (position + strlen(raw) - 1) );
        *dst = 's';
    }

    return position?1:0;

}

void pdt_save_vb_constants(vector_t *resources, pdt_param_t vb_param)
{
    int const_index;
    int resource_index;
    int resource_count;
    pdt_resource_t *resource;
    FILE *stream = NULL;
    vb_param_t param = (vb_param_t)vb_param;
    
    assert(resources);
    
    if (param && param->const_header)
    {
        stream = fopen(param->const_header, "w");
        if (!stream)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Cannot open output const file - %s.", param->const_header);
            return;
        }
    }
    
    /* generate output const header in VB*/
    const_index = pdt_find_csv_locale(resources, "const");
    resource = get_element(resources, const_index);
    
    if (resource)
    {
        resource_count = element_count(resource->strings);
        fprintf(stream, "' Generated by pd_replace. Do not modify this file directly.\n");
        
        for (resource_index = 0; resource_index < resource_count; resource_index++)
        {
            char *value = strdup(get_element(resource->strings, resource_index));
            char *temp;
            
            if (!value || strlen(value) <= 0)
                continue;

            temp = value;
            
            /* get rid of the quoter from both sides */
            temp[strlen(temp) - 1] = PDT_TERMINATOR;
            temp++;
            if (temp && strlen(temp) > 1)
                fprintf(stream, "Public Const %s As Integer = %d\n", temp , resource_index + 1);

            if (value)
            {
                free(value);
                value = NULL;
            }
        }
    }

    if (param->cache_properties)
    {
        int num_props = element_count(param->cache_properties);
        int index;

        if (num_props > 0)
        {
            fprintf(stream, "\n\n");
            fprintf(stream, "Public Function PDSetProperty(ByRef PDObject As Object, Name As String, Value As Integer)\n");
            fprintf(stream, "   on error resume next\n");
            fprintf(stream, "   Select Case Name\n");

            for (index = 0; index < num_props; index++)
            {
                char *name = strdup(get_element(param->cache_properties, index));
                if (name && strlen(name) > 0)
                {
                    int found = 0;
                    fprintf(stream, "      Case \"%s\"\n", name);
                    do
                    {
                        int buttons_found, menus_found, tabs_found, columnheader_found, object_tooltiptext_found ;
                        int next_comma_index = 0;
                        char *position = NULL;

                        buttons_found = pdt_convert_vb_tag(name, VB_BUTTONS_BUTTON, VB_BUTTONS);
                        menus_found = pdt_convert_vb_tag(name, VB_BUTTONMENUS_BUTTONMENU, VB_BUTTONMENUS);
                        tabs_found = pdt_convert_vb_tag(name, VB_TABS_TAB, VB_TABS);
                        columnheader_found = pdt_convert_vb_columnheader(&name, VB_COLUMNHEADERQ, VB_COLUMNHEADERS);
                        object_tooltiptext_found = pdt_convert_object_tooltiptext(name, VB_OBJECT_TOOLTIPTEXT, VB_TOOLTIPTEXT);

                        found = buttons_found | menus_found | tabs_found | columnheader_found | object_tooltiptext_found;
                    } while (found);
                    
                    fprintf(stream, "         PDObject.%s = PDLoadString(Value)\n", name);
                    /*
                    if (stristr(name, VB_COLUMNHEADER))
                    {
                        fprintf(stream, "         PDObject.ColumnHeaders%s = PDLoadString(Value)\n", (name + strlen(VB_COLUMNHEADER)));
                    }
                    else
                    {
                        fprintf(stream, "         PDObject.%s = PDLoadString(Value)\n", name);
                    }
                    */
                    if (name) free(name);
                }
            }

            fprintf(stream, "      Case Else\n");
            fprintf(stream, "         Debug.Print \"unknown name for PDSetProperty\" & Name\n");
            fprintf(stream, "   End Select\n");
            fprintf(stream, "End Function\n");
        }
    }

    if (stream)
        fclose(stream);
}

void pdt_parse_vb_word(vb_process_t process, char *word, FILE *stream)
{
    char *prefix = NULL;

    if (process->nwords == 0)
    {
        /* clear per-line cache */
        if (process->name)
        {
            free(process->name);
            process->name = NULL;
        }
        if (process->value)
        {
            free(process->value);
            process->value = NULL;
        }
        /* determine line type */
        if (_stricmp("BeginProperty", word) == 0)
        {
            push_element(process->line_stack, (void *)process->line_type);
            process->line_type = VB_LINE_BEGIN_PROPERTY;
        }
        else if ((_stricmp("EndProperty", word) == 0) &&
                 (process->line_type == VB_LINE_BEGIN_PROPERTY ||
                  process->line_type == VB_LINE_SCOPE_PROPERTY))
        {
            process->line_type = VB_LINE_END_PROPERTY;
        }
        else if (_stricmp("Begin", word) == 0)
        {
            push_element(process->line_stack, (void *)process->line_type);
            process->line_type = VB_LINE_BEGIN;
        }
        else if (_stricmp("End", word) == 0 &&
                 (process->line_type == VB_LINE_SCOPE || process->line_type == VB_LINE_BEGIN))
        {
            process->line_type = VB_LINE_END;
        }
        else
        {
            if (process->line_type == VB_LINE_BEGIN)
                process->line_type = VB_LINE_SCOPE;
            else if (process->line_type == VB_LINE_BEGIN_PROPERTY)
                process->line_type = VB_LINE_SCOPE_PROPERTY;
        }
        
        /* process first word */
        switch (process->line_type)
        {
        case VB_LINE_BEGIN:
            if (process->mode == PDT_MODE_REPLACE && process->current_properties)
                process->before_string = pdt_create_vb_tag(process->current_properties, process->property_depth);
            process->property_depth++;
            if (process->current_properties)
                push_element(process->property_stack, process->current_properties);
            process->current_properties = pdt_create_properties();
            break;
        case VB_LINE_END:
            if (process->mode == PDT_MODE_REPLACE && process->current_properties)
                process->before_string = pdt_create_vb_tag(process->current_properties, process->property_depth);
            if (process->current_properties)
                pdt_destroy_properties(process->current_properties);
            process->current_properties = pop_element(process->property_stack);
            if (process->property_depth > 0)
                process->property_depth--;
            assert(process->property_depth >= 0);
            process->line_type = (int)pop_element(process->line_stack);
            break;
        case VB_LINE_SCOPE:
            process->name = strdup(word);
            break;
        case VB_LINE_SCOPE_PROPERTY:
            process->name = pdt_combine_resource_prefix(process->prefix_stack, word);
            break;
        case VB_LINE_END_PROPERTY:
            process->name = strdup(word);
            prefix = pop_element(process->prefix_stack);
            if (prefix)
                free(prefix);
            process->line_type = (int)pop_element(process->line_stack);
            break;
        }
    }
    else if (process->nwords == 1)
    {
        /* process second word */
        switch (process->line_type)
        {
        case VB_LINE_SCOPE:
        case VB_LINE_SCOPE_PROPERTY:
            if (strcmp("=", word) != 0)
            {
                int raw_length;
                int word_length;
                char *temp_name;

                assert(process->name);

                raw_length = strlen(process->name);
                word_length = strlen(word);

                temp_name = malloc(raw_length + word_length + 1);
                assert(temp_name);
                memset(temp_name, 0, word_length+raw_length+1);

                memcpy(temp_name, process->name, raw_length);
                memcpy(temp_name+raw_length, word, strlen(word));

                free(process->name);
                process->name = temp_name;

                process->nwords--;
            }
            break;
        case VB_LINE_BEGIN_PROPERTY:
            push_element(process->prefix_stack, strdup(word));
            break;
        }
    }
    else if (process->nwords == 2)
    {
        switch (process->line_type)
        {
        case VB_LINE_SCOPE:
        case VB_LINE_SCOPE_PROPERTY:
            process->value = strdup(word);
            if (*word == '\"' && pdt_must_localize(process->value))
            {
                if (process->mode == PDT_MODE_REPLACE)
                {
                    pdt_property_t *property = malloc(sizeof (pdt_property_t));
                    property->name = strdup(process->name);
                    property->id = find_element(process->resources->strings, process->value) + 1;
                    if (property->id > 0)
                    {
                        /* finally, we got property here */
                        pdt_append_property(process->current_properties, property);
                        /* store property name to the cache */
                        if (process->cache_properties && process->property_storer)
                            process->property_storer(process->cache_properties, process->name);
                    }
                    else
                    {
                        pdt_destroy_property(property);
                        if (process->mode == PDT_MODE_REPLACE)
                            pdt_report_error(PDT_REPORT_WARNING, "Can not find resource %s.", process->value);
                    }
#ifdef ENABLE_VB_TRANSLATION
                    if (process->replace_type == PDT_REPLACE_TRANSLATE)
                    {
                        process->replace_current = pdt_processor_replace_word((pdt_process_t)process, word);
                    }
#endif
                }
                else
                {
                    process->should_draw = 1;
                }
            }
            break;
        }
    }
    
    if (*word == '\"')
    {
        if (pdt_must_localize(word))
        {
            if (process->line_type == VB_LINE_NORMAL && process->mode == PDT_MODE_REPLACE)
            {
                process->replace_current = pdt_processor_replace_word((pdt_process_t)process, word);
            }
            process->should_draw = 1;
        }
    }
}

/**
 * Get one word from the given text. Includes quoter
 * if word is a quoted string.
 *
 * @param word_begin: [OUT] begin index of the word from the text
 *
 * @return: scanned word, must be destroyed outside fuction,
 *          or return NULL means no more word available
 */
int pdt_scan_vb_word(vb_process_t process, char **bufptr, size_t *n,
                     int *word_begin, FILE *stream)
{
    int nchars_avail;   /* Allocated but unused chars in *LINEPTR.  */
    char *read_pos;     /* Where we're reading into *LINEPTR. */
    int ret;

    int word_started = 0;
    int columnheader_started = 0;
    int quoted = 0;
    int commented = 0;
    int reading_operator = 0;

    int more_line = 0;
    
    assert(process && process->language == LANGUAGE_VB && bufptr && n && word_begin && stream);

    if (process->before_string)
    {
        free(process->before_string);
        process->before_string = NULL;
    }
    if (process->after_string)
    {
        free(process->after_string);
        process->after_string = NULL;
    }
    if (process->replace_current)
    {
        free(process->replace_current);
        process->replace_current = NULL;
    }
    process->should_draw = 0;

    *word_begin = -1;

    if (create_buffer (bufptr, n, &read_pos, &nchars_avail) < 0)
        return -1;

    for (; ; )
    {
        char c, next;
        int res;
        
        res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &c);

        if (res < 0)
            return -1;
        else if (res == 0)
            break;

        if (c >= 0x80)
        {
            res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
            
            if (res < 0)
                return -1;
            else if (res == 0)
                break;

            continue;
        }

        if (commented)
        {
            *word_begin = read_pos - *bufptr;
            if (c == '_')
            {
                more_line = 1;
            }
            else if (c == '\n')
            {
                if (more_line)
                    more_line = 0;
                else
                {
                    process->nblocks++;
                    process->nwords = -1;
                    commented = 0;
                }
            }
            else if (!strchr(PDT_SPACES, c))
            {
                more_line = 0;
            }
            continue;
        }
        else if (quoted)
        {
            if (c == '\"')
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    break;

                if (next == '\"')
                    continue;
                else
                {
                    ungetc(next, stream);
                    read_pos--;
                    break;
                }
            }
            else
                continue;
        }
        else if (reading_operator)
        {
            if (strchr ("><=+-*/&|:~^", c))
                continue;
            else
            {
                ungetc(c, stream);
                read_pos--;
                break;
            }
        }

        switch (c)
        {
        case'\"':
            if (!word_started)
            {
                word_started = quoted = 1;
                *word_begin = read_pos - *bufptr -1;
                break;
            }
            else
            {
                ungetc(c, stream);
                read_pos--;
                goto break_char;
            }
            break;
        case '\n':
            if (!word_started)
            {
                process->nblocks++;
                process->nwords = -1;
            }
        case '\r':
        case '\t':
        case ' ':
            if (word_started)
            {
                ungetc(c, stream);
                read_pos--;
                goto break_char;
            }
            else
                *word_begin = read_pos - *bufptr;
            break;
        case '_':
            break;
        case '+':
        case '%':
        case '-':
        case '/':
        case '*':
        case '=':
        case '>':
        case '<':
        case '&':
        case '~':
        case '^':
        case '|':
        case ':':
            if (word_started)
            {
                ungetc(c, stream);
                read_pos--;
                goto break_char;
            }
            else
            {
                *word_begin = read_pos - *bufptr -1;
                word_started = 1;
                reading_operator = 1;
                goto next_char;
            }
        /*
        If you Let the ColumnHeader(X) look like one word, 
        there will be a disaster for other com such as Buttons
        */
        case '(':
            if (word_started)
            {
                if (stristr(*bufptr, VB_COLUMNHEADER)) 
                {
                    if (!columnheader_started)
                    {
                        columnheader_started = 1;
                    }
                    break;
                }
                else 
                {
                    ungetc(c, stream);
                    read_pos--;
                    goto break_char;
                }
            }
            else
            {
                *word_begin = read_pos - *bufptr -1;
                word_started = 1;
                reading_operator = 1;
                goto next_char;
            }
        case ')':
            if (word_started)
            {
                if (columnheader_started) 
                {
                    columnheader_started = 0;
                    goto break_char;
                }
                else 
                {
                    ungetc(c, stream);
                    read_pos--;
                    goto break_char;
                }
            }
            else
            {
                *word_begin = read_pos - *bufptr -1;
                word_started = 1;
                reading_operator = 1;
                goto next_char;
            }
        case '\'':   /* comment */
            if (word_started)
            {
                ungetc(c, stream);
                read_pos--;
                goto break_char;
            }
            else
            {
                commented = 1;
                more_line = 0;
                *word_begin = read_pos - *bufptr;
                goto next_char;
            }
            break;
        default:
            if (!word_started)
            {
                word_started = 1;
                *word_begin = read_pos - *bufptr -1;
            }
            break;
        }
next_char:
        continue;
    }
    
break_char:
    /* Done - NUL terminate and return the number of chars read.  */
    *read_pos = '\0';
    
    ret = read_pos - (*bufptr);
    process->nwords++;
    return ret;
}

char *pdt_create_vb_tag(pdt_properties_t properties, int depth)
{
    int index;
    char *result = NULL;
    int required = 0;
    char *tag = NULL;

    if (!properties)
        return NULL;

    tag = pdt_create_properties_string(properties);
    if (!tag)
        return NULL;

    required = depth*strlen(VB_TAG_TAB)+strlen(VB_TAG_HEADER)+strlen(tag)+2;
    result = malloc(required);
    if (!result)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Memory allocation failure.", 0);
        goto failure;
    }

    *result = '\0';
    strcat(result, "\n");
    for (index = 0; index < depth; index++)
        strcat(result, VB_TAG_TAB);

    strcat(result, VB_TAG_HEADER);
    strcat(result, tag);

failure:
    if (tag)
        pdt_destroy_properties_string(tag);
    return result;
}

void pdt_output_vb_tag(vector_t *properties, int depth, FILE *stream)
{
    int properties_count, index;
    pdt_property_t * pproperties = NULL;
    
    properties_count = element_count(properties);
    if (properties_count == 0)
        return;
    
    for (index = 0; index < depth; index++)
        fprintf(stream, "%s", VB_TAG_TAB);
    
    fprintf(stream, "%s\"", VB_TAG_HEADER);
    
    for (index = 0; index < properties_count; index++)
    {
        pproperties = (pdt_property_t *) get_element(properties, index);
        fprintf(stream, "%s=%d", pproperties->name, pproperties->id);
        
        if (index < properties_count-1)
            fprintf(stream, ";");
    }
    
    fprintf(stream, "\"\n");
}

char *pdt_vb_to_csv(vb_process_t process, const char *quoted)
{
    if (process->should_draw)
        return strdup(quoted);
    else
        return NULL;
}

char *pdt_vb_from_csv(vb_process_t process, const char *quoted)
{
    if (process->should_draw)
        return strdup(quoted);
    else
        return NULL;
}

void pdt_store_vb_property(vector_t *names, char *name)
{
    if (names && name)
        append_element(names, strdup(name), 1);
}

pdt_param_t pdt_create_vb_param(int cache_properties, char *const_header)
{
    vb_param_t param = malloc(sizeof (struct _vb_param_t));

    if (param)
    {
        param->const_header = const_header?strdup(const_header):NULL;
        if (cache_properties)
            param->cache_properties = create_vector(sizeof (char *), free, strcmp);
        else
            param->cache_properties = NULL;
        param->store_name = pdt_store_vb_property;
    }

    return (pdt_param_t)param;
}

void pdt_destroy_vb_param(pdt_param_t param)
{
    vb_param_t vb_param = (vb_param_t)param;

    if (vb_param)
    {
        if (vb_param->const_header)
            free(vb_param->const_header);
        if (vb_param->cache_properties)
            destroy_vector(vb_param->cache_properties);

        free(vb_param);
    }
}

int pdt_escape_vb_path(const char *path)
{
    assert(path);
    return (strcmp(path, "FRIEND2") == 0);
}

int pdt_convert_object_tooltiptext(char *tag_name, const char *raw, const char *replace)
{
    char *position = NULL;

    position = stristr(tag_name, raw);
    if (position)
    {
        int length = strlen(replace);
        char *dot = NULL;
        char *cursor = (char *)position;
        int rlen = strlen(raw);

        memcpy(cursor, replace, length);
        cursor += length;

        dot = strchr(position+rlen, '.');
        if ( 0 == dot  ) 
        {
            *(cursor) = PDT_TERMINATOR;
        }
        else
        {
            length = dot-(position+rlen);
            memcpy(cursor, position+rlen, length);
            cursor += length;
            memcpy(cursor, dot, strlen(dot)+1);
        }
    }

    return position?1:0;

}

int pdt_convert_vb_tag(char *tag_name, const char *raw, const char *replace)
{
    char *position = NULL;

    position = stristr(tag_name, raw);
    if (position)
    {
        int length = strlen(replace);
        char *dot = NULL;
        char *cursor = (char *)position;
        int rlen = strlen(raw);

        memcpy(cursor, replace, length);
        cursor += length;
        *cursor = '(';
        cursor++;

        dot = strchr(position+rlen, '.');
        length = dot-(position+rlen);
        memcpy(cursor, position+rlen, length);
        cursor += length;
        *cursor = ')';
        cursor++;
        memcpy(cursor, dot, strlen(dot)+1);
    }

    return position?1:0;
}

/* these should be converted to multi-language version */
pdt_plugin_t pdt_bas_plugin = {
    ".bas",
    (pdt_creator_t)pdt_create_vb_processor,
    pdt_escape_vb_path,
    0,
    "japanese",
    NULL,
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "PDLoadString(%s)",
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_cls_plugin = {
    ".cls",
    (pdt_creator_t)pdt_create_vb_processor,
    pdt_escape_vb_path,
    0,
    "japanese",
    NULL,
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "PDLoadString(%s)",
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_frm_plugin = {
    ".frm",
    (pdt_creator_t)pdt_create_vb_processor,
    pdt_escape_vb_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "PDLoadString(%s)",
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_ctl_plugin = {
    ".ctl",
    (pdt_creator_t)pdt_create_vb_processor,
    pdt_escape_vb_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "PDLoadString(%s)",
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_vbs_plugin = {
    ".vbs",
    (pdt_creator_t)pdt_create_vb_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_TRANSLATE,
    NULL,   /* format string */
    NULL,   /* csv handle */
    0,
};
