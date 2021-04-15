#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

void pdt_destroy_cpp_processor(cpp_process_t process)
{
    if (process)
    {
        if (process->before_string)
            free(process->before_string);
        if (process->after_string)
            free(process->after_string);
        if (process->replace_current)
            free(process->replace_current);
        free(process);
    }
}

pdt_process_t pdt_create_cpp_processor(pdt_param_t param)
{
    cpp_process_t process = NULL;

    process = malloc(sizeof (struct _cpp_process_t));
    if (process)
    {
        process->language = LANGUAGE_CPP;
        process->scanner = (pdt_scanner_t)pdt_scan_cpp_word;
        process->parser = (pdt_parser_t)pdt_parse_cpp_word;
        process->destroyer = (pdt_destroyer_t)pdt_destroy_cpp_processor;

        process->to_csv = (pdt_convert_t)pdt_cpp_to_csv;
        process->from_csv = (pdt_convert_t)pdt_cpp_from_csv;
        process->free_string = free;

        process->nblocks = 0;
    }

    return (pdt_process_t)process;
}

void pdt_parse_cpp_word(cpp_process_t process, char *word, FILE *stream)
{
    char *csv_in = NULL;
    char *csv_out = NULL;

    if ((*word == '\"' && *(word + strlen(word) - 1) == '\"') ||
        (*word == '\'' && *(word + strlen(word) - 1) == '\''))
    {
        if (pdt_must_localize(word))
        {
            if (process->mode == PDT_MODE_REPLACE)
            {
                csv_in = pdt_cpp_to_csv(process, word);
            
                if (csv_in)
                {
                    csv_out = pdt_processor_replace_word((pdt_process_t)process, csv_in);
                    if (csv_out)
                    {
                        if (process->replace_type & PDT_REPLACE_TRANSLATE)
                            process->replace_current = pdt_cpp_from_csv(process, csv_out);
                        else
                            process->replace_current = strdup(csv_out);
                    }
                }
                if (csv_in)
                    free(csv_in);
                if (csv_out)
                    free(csv_out);
            }
            else
                process->should_draw = 1;
        }
    }
}

int pdt_scan_cpp_word(cpp_process_t process, char **bufptr, size_t *n,
                     int *word_begin, FILE *stream)
{
    int nchars_avail;   /* Allocated but unused chars in *LINEPTR.  */
    char *read_pos;     /* Where we're reading into *LINEPTR. */
    int ret;

    int word_started = 0;
    int single_quoted = 0;
    int double_quoted = 0;
    int line_commented = 0;
    int block_commented = 0;
    int star_occured = 0;
    int reading_operator = 0;

    int more_line = 0;
    
    assert(process && process->language == LANGUAGE_CPP && bufptr && n && word_begin && stream);

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
        unsigned char c, next;
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

        if (line_commented)
        {
            *word_begin = read_pos - *bufptr;
            if (c == '\n')
            {
                process->nblocks++;
                process->nwords = -1;
                line_commented = 0;
            }
            continue;
        }
        else if (double_quoted || single_quoted)
        {
            if (c == '\\')
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    break;

                continue;
            }
            else if ((single_quoted && (c == '\'')) || (double_quoted && (c == '\"')))
            {
                double_quoted = 0;
                single_quoted = 0;
                break;
            }
            else
                continue;
        }
        else if (block_commented)
        {
            *word_begin = read_pos - *bufptr;
            if (c == '*')
                star_occured = 1;
            else if (c == '/')
            {
                if (star_occured)
                {
                    star_occured = 0;
                    block_commented = 0;
                }
            }
            else
                star_occured = 0;
            continue;
        }
        else if (reading_operator)
        {
            if (strchr ("!%><=+-*/&|:~^\\", c))
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
                word_started = double_quoted = 1;
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
        case'\'':
            if (!word_started)
            {
                word_started = single_quoted = 1;
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
        case '{':
        case '}':
        case ';':
        case ',':
        case '%':
        case '!':
        case '\\':
        case '+':
        case '-':
        case '*':
        case '=':
        case '>':
        case '<':
        case '&':
        case '~':
        case '^':
        case '|':
        case ':':
        case '(':
        case ')':
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
        case '/':
            if (word_started)
            {
                ungetc(c, stream);
                read_pos--;
                goto break_char;
            }
            else
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
            
                if (res < 0)
                    return -1;
                else if (res == 0)
                    break;
                if (next == '/')
                {
                    line_commented = 1;
                    *word_begin = read_pos - *bufptr;
                    goto next_char;
                }
                else if (next == '*')
                {
                    block_commented = 1;
                    *word_begin = read_pos - *bufptr;
                    goto next_char;
                }
                else
                {
                    ungetc (next, stream);
                    read_pos--;

                    *word_begin = read_pos - *bufptr -1;
                    word_started = 1;
                    reading_operator = 1;
                    goto next_char;
                }
            }
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

char *pdt_cpp_to_csv(cpp_process_t process, const char *word_in)
{
    int index = 0;
    int word_len = 0;
    char *word_out = NULL;
    int index_out = 0;
    int append = 0;

//	return strdup(word_in);

    word_len = strlen(word_in);
	
	if (!(word_in && word_len >= 2 && 
        ((word_in[0] == '\"') && (word_in[strlen(word_in) - 1] == '\"')) ||
        ((word_in[0] == '\'') && (word_in[strlen(word_in) - 1] == '\''))))
        return NULL;

    word_out = (char *)malloc(sizeof(char)*word_len*2+3);
    if (!word_out) {
        pdt_report_error(PDT_REPORT_ERROR, "Cannot malloc for %s in cpp_to_csv.\n", word_in);
        return NULL;
    }

    word_out[index_out++] = '\"';
    do
    {
        unsigned char c = *(word_in+index);
        index++;
        if (c >= (unsigned char)0x80)
        {
            if ( (c >= 0xA6) && ( c <= 0xDF) )
            {
                //Single-Hiragara
                word_out[index_out++] = c;
                continue;
            }
            else
            {
                unsigned char next = *(word_in + index);
                word_out[index_out++] = c;
                word_out[index_out++] = next;
                index++;
                continue;
            }
        }

        switch (c)
        {
        case '\"':
            word_out[index_out++] = '\"';
            word_out[index_out++] = '\"';
            break;
        case '\n':
            word_out[index_out++] = '\\';
            word_out[index_out++] = 'n';
            break;
        case '\r':
            word_out[index_out++] = '\\';
            word_out[index_out++] = 'r';
            break;
        default:
            word_out[index_out++] = c;
            break;
        }

    }
    while (index < word_len);
    word_out[index_out++] = '\"';
    word_out[index_out] = PDT_TERMINATOR;

    assert( word_out && (strlen(word_out) >= 2) && (word_out[0] == '\"') && 
            (word_out[strlen(word_out) - 1] == '\"'));

    return strdup(word_out);
}

char *pdt_cpp_from_csv(cpp_process_t process, const char *word_in)
{
    int index = 0;
    int word_len = 0;
    char *word_out = NULL;
    unsigned char next_char;
    int index_out = 0;

    word_len = strlen(word_in);

	if (!(word_in && word_len >= 2 && (word_in[0] == '\"') && 
        (word_in[strlen(word_in) - 1] == '\"')))
        return NULL;

    word_out = (char *)malloc(sizeof(char)*word_len);
    if (!word_out) {
        pdt_report_error(PDT_REPORT_ERROR, "Cannot malloc for %s in cpp_from_csv.\n", word_in);
        return NULL;
    }

    index++;
    do
    {
        unsigned char c = word_in[index++];
        if (c >= (unsigned char)0x80)
        {
            if ( (c >= 0xA6) && ( c <= 0xDF) )
            {
                //Single-Hiragara
                word_out[index_out++] = c;
                continue;
            }
            else
            {
                unsigned char next = *(word_in + index);
                word_out[index_out++] = c;
                word_out[index_out++] = next;
                index++;
                continue;
            }
        }

        switch (c)
        {
        case '\"':
            next_char = word_in[index++];
            switch (next_char)
            {
            case '\"':
                word_out[index_out++] = '\"';
                break;
            default:
                pdt_report_error(PDT_REPORT_ERROR, "Illegal escape string for JS found![\"%c].\n", next_char);
                break;
            }
            break;
        default:
            word_out[index_out++] = c;
            break;
        }

    }
    while (index < word_len -1);
    word_out[index_out] = PDT_TERMINATOR;

    assert( word_out && (strlen(word_out) >= 2) );

    return word_out;

}

void pdt_store_vc_property(vector_t *names, char *name)
{
    if (names && name)
        append_element(names, strdup(name), 1);
}

pdt_param_t pdt_create_vc_param(int cache_properties, char *const_header)
{
    vc_param_t param = malloc(sizeof (struct _vc_param_t));

    if (param)
    {
        param->const_header = const_header?strdup(const_header):NULL;
        if (cache_properties)
            param->cache_properties = create_vector(sizeof (char *), free, strcmp);
        else
            param->cache_properties = NULL;
        param->store_name = pdt_store_vc_property;
    }

    return (pdt_param_t)param;
}

void pdt_destroy_vc_param(pdt_param_t param)
{
    vc_param_t vc_param = (vc_param_t)param;

    if (vc_param)
    {
        if (vc_param->const_header)
            free(vc_param->const_header);
        if (vc_param->cache_properties)
            destroy_vector(vc_param->cache_properties);

        free(vc_param);
    }
}

void pdt_save_vc_constants(vector_t *resources, pdt_param_t vc_param)
{
    int const_index;
    int resource_index;
    int resource_count;
    pdt_resource_t *resource;
    FILE *stream = NULL;
    vc_param_t param = (vc_param_t)vc_param;
    
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
        fprintf(stream, "// Generated by pd_replace. Do not modify this file directly.\n");
        
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
                fprintf(stream, "#define %s\t\tIDS_SV_STRBASE+%d\n", temp , resource_index + 1);

            if (value)
            {
                free(value);
                value = NULL;
            }
        }
    }
/*
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

//			STRINGTABLE DISCARDABLE 
//BEGIN
//   AFX_IDS_APP_TITLE       "SecureVisor アンインストール"
//END


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
                    if (name) free(name);
                }
            }

            fprintf(stream, "      Case Else\n");
            fprintf(stream, "         Debug.Print \"unknown name for PDSetProperty\" & Name\n");
            fprintf(stream, "   End Select\n");
            fprintf(stream, "End Function\n");
        }
    }
*/    
	if (stream)
        fclose(stream);
}

pdt_plugin_t pdt_cpp_plugin = {
    ".cpp",
    pdt_create_cpp_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "SV_LOADSTRING(%s)",/* format string */
	/*
    PDT_REPLACE_TRANSLATE,
    NULL,*/
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_c_plugin = {
    ".c",//c
    pdt_create_cpp_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "SV_LOADSTRING(%s)",/* format string */
	/*
    PDT_REPLACE_TRANSLATE,
    NULL,*/
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_hpp_plugin = {
    ".hpp",
    pdt_create_cpp_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "SV_LOADSTRING(%s)",/* format string */
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_h_plugin = {
    ".h",
    pdt_create_cpp_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_CONST | PDT_REPLACE_FORMAT,
    "SV_LOADSTRING(%s)",/* format string */
    NULL,   /* csv handle */
    0,
};
