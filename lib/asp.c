#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

void pdt_create_asp_syntax(asp_process_t process, int language_type)
{
    static char vb_sufix[] = ".bas";
    static char js_sufix[] = ".js";
    char *sufix = vb_sufix;

    assert(process && language_type != LANGUAGE_UNKNOWN);

    process->language_type = language_type;
    if (language_type == LANGUAGE_JS)
    {
        process->syntax_parser = (pdt_process_t)pdt_create_js_processor(0);
        sufix = js_sufix;
    }
    else
    {
        process->syntax_parser = (pdt_process_t)pdt_create_vb_processor(0);
        sufix = vb_sufix;
    }
    
    pdt_init_file_processor(process->syntax_parser, sufix, sufix, process->mode);
    
    if (process->mode == PDT_MODE_REPLACE)
    {
        pdt_set_processor_replacement(process->syntax_parser,
                                      process->replace_type,
                                      process->replace_format);
        pdt_set_processor_resources(process->syntax_parser,
                                    process->csv_resources,
                                    process->compare_language,
                                    process->translate_language,
                                    process->replace_type & PDT_REPLACE_CONST,
                                    process->replace_type & PDT_REPLACE_TRANSLATE);
    }
    else
    {
        pdt_set_processor_replacement(process->syntax_parser, 0, NULL);
        pdt_set_processor_resources(process->syntax_parser,
                                    process->csv_resources,
                                    process->compare_language,
                                    NULL, FALSE, FALSE);
    }
}

void pdt_destroy_asp_syntax(asp_process_t process)
{
    assert(process);

    if (process->syntax_parser)
    {
        pdt_destroy_file_processor(process->syntax_parser);
        process->syntax_parser = NULL;
        process->language_type = LANGUAGE_VB;
    }
}

void pdt_destroy_asp_processor(asp_process_t process)
{
    if (process)
    {
        if (process->before_string)
            free(process->before_string);
        if (process->after_string)
            free(process->after_string);
        if (process->replace_current)
            free(process->replace_current);

        pdt_destroy_asp_syntax(process);
        free(process);
    }
}

/* IMPORTANT: do not call this directly, use pdt_create_file_processor instead */
pdt_process_t pdt_create_asp_processor(asp_param_t param)
{
    asp_process_t process = NULL;

    if (!param || param->language == LANGUAGE_UNKNOWN)
        return NULL;

    process = malloc(sizeof (struct _asp_process_t));
    if (process)
    {
        process->language = LANGUAGE_ASP;
        process->scanner = (pdt_scanner_t)pdt_scan_asp_word;
        process->parser = (pdt_parser_t)pdt_parse_asp_word;
        process->destroyer = (pdt_destroyer_t)pdt_destroy_asp_processor;

        process->to_csv = (pdt_convert_t)pdt_asp_to_csv;
        process->from_csv = (pdt_convert_t)pdt_asp_from_csv;
        process->free_string = free;

        process->need_include = 0;
        process->block_type = ASP_BLOCK_HTML;
        process->context_type = ASP_CONTEXT_NORMAL;
        process->syntax_parser = NULL;
        process->language_type = param->language;
        process->message_include = param->message_include;
        process->include_output = FALSE;
    }

    return (pdt_process_t)process;
}

void pdt_destroy_asp_param(pdt_param_t param)
{
    asp_param_t asp_param = (asp_param_t)param;

    if (asp_param)
    {
        if (asp_param->message_include)
            free(asp_param->message_include);
        free(asp_param);
    }
}

pdt_param_t pdt_create_asp_param(int language, char *message_include)
{
    asp_param_t param = malloc(sizeof (struct _asp_param_t));

    if (param)
    {
        param->language = language;
        param->message_include = message_include?strdup(message_include):NULL;
    }

    return (pdt_param_t)param;
}

char *pdt_asp_from_csv(asp_process_t process, const char *word)
{
    assert(process && word);

    if (process->should_draw && process->syntax_parser)
        return process->syntax_parser->from_csv(process->syntax_parser, word);
    else
        return NULL;
}

char *pdt_asp_to_csv(asp_process_t process, const char *word)
{
    assert(process && word);

    if (process->should_draw && process->syntax_parser)
        return process->syntax_parser->to_csv(process->syntax_parser, word);
    else
        return NULL;
}

char *pdt_output_asp_include(asp_process_t process)
{
    char *include = NULL;
    char *result = NULL;

    if (process->include_output)
        return NULL;
    
    include = strdup(pdt_include_from_path(process->input, process->message_include));
    if (include)
    {
        static int include_length = 28;
        result = malloc(include_length + strlen(include));
        if (result)
        {
            process->include_output = TRUE;
            strcpy(result, "\n<!-- #INCLUDE FILE=\"");
            strcat(result, include);
            strcat(result, "\" -->");
        }
        free(include);
    }

    return result;
}

void pdt_parse_asp_word(asp_process_t process, char *word, FILE *stream)
{
    if (process->block_type == ASP_BLOCK_COMMENT)
    {
        if (process->nwords == 0)
            process->context_type = ASP_CONTEXT_NORMAL;
        else if (process->context_type == ASP_CONTEXT_NORMAL)
        {
            if (stricmp(word, "#include") == 0 && process->nwords == 1)
            {
                process->context_type = ASP_CONTEXT_INCLUDE;
                process->need_include = 1;
            }
            else if (strcmp(word, "<%") == 0)
            {
                process->block_type = ASP_BLOCK_SCRIPT;
                process->nblocks++;
                process->nwords = 0;
            }
            else if (strcmp(word, "<%=") == 0)
            {
                process->block_type = ASP_BLOCK_SCRIPT;
                process->syntax_type = ASP_SYNTAX_WRITER;
                process->nblocks++;
                process->nwords = 0;
            }
            else if (strcmp(word, "<%@") == 0)
            {
                process->context_type = ASP_CONTEXT_LANGUAGE;
                process->syntax_type = ASP_SYNTAX_LANGUAGE;
                process->nblocks++;
                process->nwords = 0;
            }
        }
    }
    else if (process->block_type == ASP_BLOCK_SCRIPT)
    {
        if (process->nwords == 0)
        {
            process->context_type = ASP_CONTEXT_NORMAL;
        }
        else if (process->nwords == 1)
        {
            switch (process->syntax_type)
            {
            case ASP_SYNTAX_PROPERTY:
                if (stricmp(word, "LANGUAGE") == 0)
                {
                    process->need_include = 1;
                    process->context_type = ASP_CONTEXT_LANGUAGE;
                }
                break;
            case ASP_SYNTAX_LANGUAGE:
            case ASP_SYNTAX_WRITER:
                process->context_type = ASP_CONTEXT_CODE;
                break;
            default:
                process->context_type = ASP_CONTEXT_NORMAL;
                break;
            }
        }
    }
    else
        process->context_type = ASP_CONTEXT_NORMAL;

    switch (process->context_type)
    {
    case ASP_CONTEXT_INCLUDE:
        if ((process->nwords == 2) && (stricmp(word, "FILE") != 0) ||
            (process->nwords == 3) && (stricmp(word, "=") != 0))
            process->context_type = ASP_CONTEXT_NORMAL;
        else if (process->nwords == 4)
        {
            char *absolute_input = NULL;
            char *absolute_output = NULL;
            pdt_param_t old_param;
            pdt_param_t new_param;
            char *temp_word = strdup(word+1);

            new_param = pdt_create_asp_param(process->language_type, process->message_include);
            old_param = pdt_set_sufix_processor(".inc", new_param);

            *(temp_word+strlen(temp_word)-1) = PDT_TERMINATOR;

            absolute_input = strdup(pdt_include_to_path(process->input, temp_word));
            if (process->mode == PDT_MODE_REPLACE)
                absolute_output = strdup(pdt_include_to_path(process->output, temp_word));
            
            /* Can't guess the output for absolute path, pdt_include_to_path return  */
            if (absolute_output && strcmp(absolute_output, temp_word) == 0)
                pdt_report_error(PDT_REPORT_ERROR, "Can't guess the output for absolute path %s.\n", temp_word);

            if (!absolute_input)
                pdt_report_error(PDT_REPORT_ERROR, "Failed to calculate input %s.\n", process->input);
            if (process->mode == PDT_MODE_REPLACE && !absolute_output)
                pdt_report_error(PDT_REPORT_ERROR, "Failed to calculate output %s.\n", process->output);

            pdt_process_file(absolute_input, absolute_output, process->mode, 0);

            if (absolute_input)
                free(absolute_input);
            if (absolute_output)
                free(absolute_output);
            if (temp_word)
                free(temp_word);

            pdt_set_sufix_processor(".inc", old_param);
            pdt_destroy_asp_param(new_param);
        }
        else if (stricmp(word, "-->") == 0 && process->nwords == 5)
        {
            if (process->mode == PDT_MODE_REPLACE &&
                process->message_include &&
                stricmp(get_ext(process->input), ".asp") == 0)
            {
                if (process->after_string)
                {
                    free(process->after_string);
                    process->after_string = NULL;
                }
                process->after_string = pdt_output_asp_include(process);
            }
        }
        break;
    case ASP_CONTEXT_LANGUAGE:
        if ((process->nwords == 2) && (stricmp(word, "=") != 0))
            process->context_type = ASP_CONTEXT_NORMAL;
        else if (process->nwords == 3)
        {
            int language_type;

            if (stricmp(word, "\"JScript\"") == 0)
            {
                language_type = LANGUAGE_JS;
            }
            else/* if (stricmp(word, "VBScript") == 0) */
            {
                language_type = LANGUAGE_VB;
            }
            if (process->syntax_parser)
                pdt_destroy_asp_syntax(process);
            pdt_create_asp_syntax(process, language_type);
        }
        else if (stricmp(word, "%>") == 0 && process->nwords == 4)
        {
            if (process->mode == PDT_MODE_REPLACE &&
                process->message_include &&
                stricmp(get_ext(process->input), ".asp") == 0)
            {
                if (process->after_string)
                {
                    free(process->after_string);
                    process->after_string = NULL;
                }
                process->after_string = pdt_output_asp_include(process);
            }
        }
        break;
    case ASP_CONTEXT_CODE:
        process->syntax_parser->parser(process->syntax_parser, word, stream);

        /* route syntax processor's result to the parent */
        if (process->mode == PDT_MODE_REPLACE)
        {
            process->before_string = process->syntax_parser->before_string?strdup(process->syntax_parser->before_string):NULL;
            process->after_string = process->syntax_parser->after_string?strdup(process->syntax_parser->after_string):NULL;
            process->replace_current = process->syntax_parser->replace_current?strdup(process->syntax_parser->replace_current):NULL;
        }
        else
            process->should_draw = process->syntax_parser->should_draw;
        /* this will be processed by syntax parser */
        break;
    default:
        if (strcmp(word, "%>") == 0 || strcmp(word, "-->") == 0)
        {
            if (process->need_include &&
                process->mode == PDT_MODE_REPLACE &&
                process->message_include &&
                stricmp(get_ext(process->input), ".asp") == 0)
                process->after_string = pdt_output_asp_include(process);
        }
        /* do nothing */
        break;
    }
}

int pdt_scan_asp_word(asp_process_t process, char **bufptr, size_t *n,
                      int *word_begin, FILE *stream)
{
    int nchars_avail;   /* Allocated but unused chars in *LINEPTR.  */
    char *read_pos;     /* Where we're reading into *LINEPTR. */
    int ret;

    int word_started = 0;
    int double_quoted = 0;
    int operator_found = 0;

    int bang = 0;
    int percent = 0;
    int left = 0;
    int right = 0;
    int first_minus = 0;
    int second_minus = 0;
    int at = 0;
    int equal = 0;
    int sharp = 0;

    assert(process && process->language == LANGUAGE_ASP && bufptr && n && word_begin && stream);

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

    if (!process->syntax_parser)
        pdt_create_asp_syntax(process, process->language_type);

    *word_begin = -1;

    if (create_buffer (bufptr, n, &read_pos, &nchars_avail) < 0)
        return -1;

    for (; ; )
    {
        char first, second, third;
        int res;
        
        res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &first);

        if (res < 0)
            return -1;
        else if (res == 0)
            break;

        if (process->block_type & ASP_BLOCK_SCRIPT)
        {
            if (first == '%')
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &second);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    break;
                
                if (second == '>')
                {
                    process->block_type = ASP_BLOCK_HTML;
                    *word_begin = read_pos - *bufptr - 1 - 1;
                    goto break_char;
                }
                else
                {
                    ungetc(second, stream);
                }
            }
            if (process->syntax_type == ASP_SYNTAX_LANGUAGE || process->syntax_type == ASP_SYNTAX_WRITER)
            {
                ungetc(first, stream);
                ret = (process->syntax_parser->scanner)(process->syntax_parser, bufptr, n, word_begin, stream);

                if (ret > 0)
                {
                    char *word = *bufptr + *word_begin;

                    /* do not count internal words */
                    process->nwords++;

                    read_pos = ret + (*bufptr);
                    nchars_avail = *n - ret;

                    if (strlen(word) > 1 && strncmp(word, "%>", 2) == 0)
                    {
                        int offset = strlen(word)-2;

                        /* go back */
                        if (offset > 0)
                        {
                            fseek(stream, -offset, SEEK_CUR);
                            *(word+2) = 0;
                        }
                        process->block_type = ASP_BLOCK_HTML;
                        return 2;
                    }
                    else if (strlen(word) == 1 && *word == '%')
                    {
                        res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &second);
                        
                        if (res < 0)
                            return -1;
                        else if (res == 0)
                            break;

                        if (second == '>')
                        {
                            process->block_type = ASP_BLOCK_HTML;
                            goto break_char;
                        }
                        else
                        {
                            ungetc(second, stream);
                            read_pos--;
                            goto break_char;
                        }
                    }
                }
                return res;
            }
        }
        else if (process->block_type == ASP_BLOCK_COMMENT && first == '-')
        {
            res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &second);
            
            if (res < 0)
                return -1;
            else if (res == 0)
                break;

            if (second == '-')
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &third);
            
                if (res < 0)
                    return -1;
                else if (res == 0)
                    break;

                if (third == '>')
                {
                    process->block_type = ASP_BLOCK_HTML;
                    *word_begin = read_pos - *bufptr - 1 - 2;
                    goto break_char;
                }
            }
            continue;
        }

        /* ignore non-ASCii */
        if (first >= 0x80)
        {
            res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &second);
            
            if (res < 0)
                return -1;
            else if (res == 0)
                break;
            continue;
        }

        if (sharp)
        {
            if (strchr("INCLUDEinclude", first))
                continue;
            else if (strchr(PDT_SPACES, first))
            {
                ungetc(first, stream);
                read_pos--;
                goto break_char;
            }
            else
            {
                sharp = 0;
                *word_begin = -1;
            }
        }

        if (process->block_type == ASP_BLOCK_HTML ||
            (process->block_type == ASP_BLOCK_COMMENT && process->context_type != ASP_CONTEXT_INCLUDE))
        {
            switch (first)
            {
            case '#':
                if (second_minus)
                {
                    *word_begin = read_pos - *bufptr - 1 - 4;
                    if (percent)
                    {
                        process->block_type = ASP_BLOCK_SCRIPT;
                        process->syntax_type = ASP_SYNTAX_COMMENT;
                    }
                    else
                        process->block_type = ASP_BLOCK_COMMENT;
                    process->nwords = -1;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (at)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_PROPERTY;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 3;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (equal)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_WRITER;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 3;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (percent)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_LANGUAGE;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 2;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (process->block_type == ASP_BLOCK_COMMENT)
                {
                    *word_begin = read_pos - *bufptr - 1;
                    sharp = 1;
                }
                else
                {
                    left = right = percent = bang = first_minus = second_minus = at = equal = 0;
                    if (!word_started)
                        *word_begin = read_pos - *bufptr;
                }
                break;
            case '<':
                left = 1;
                break;
            case '%':
                if (left)
                    percent = 1;
                break;
            case '!':
                if (left)
                    bang = 1;
                break;
            case '@':
                if (percent)
                    at = 1;
                else
                    left = 0;
                break;
            case '=':
                if (percent)
                    equal = 1;
                else
                    left = 0;
                break;
            case '-':
                if (percent || bang)
                {
                    if (first_minus)
                        second_minus = 1;
                    else
                    {
                        first_minus = 1;
                        second_minus = 0;
                    }
                }
                else
                    left = 0;
                break;
            case '\t':
            case '\r':
            case '\n':
            case ' ':
                if (second_minus)
                {
                    *word_begin = read_pos - *bufptr - 1 - 4;
                    if (percent)
                    {
                        process->block_type = ASP_BLOCK_SCRIPT;
                        process->syntax_type = ASP_SYNTAX_COMMENT;
                    }
                    else
                        process->block_type = ASP_BLOCK_COMMENT;
                    process->nwords = -1;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (at)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_PROPERTY;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 3;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (equal)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_WRITER;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 3;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (percent)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_LANGUAGE;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 2;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else
                {
                    left = right = percent = bang = first_minus = second_minus = at = equal = 0;
                    if (!word_started)
                        *word_begin = read_pos - *bufptr;
                }
                break;
            default:
                if (second_minus)
                {
                    *word_begin = read_pos - *bufptr - 1 - 4;
                    if (percent)
                    {
                        process->block_type = ASP_BLOCK_SCRIPT;
                        process->syntax_type = ASP_SYNTAX_COMMENT;
                    }
                    else
                        process->block_type = ASP_BLOCK_COMMENT;
                    process->nwords = -1;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (at)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_PROPERTY;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 3;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (equal)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_WRITER;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 3;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else if (percent)
                {
                    process->block_type = ASP_BLOCK_SCRIPT;
                    process->syntax_type = ASP_SYNTAX_LANGUAGE;
                    process->nwords = -1;
                    *word_begin = read_pos - *bufptr - 1 - 2;
                    ungetc(first, stream);
                    read_pos--;
                    process->nblocks++;
                    goto break_char;
                }
                else
                    left = right = percent = bang = first_minus = second_minus = at = equal = 0;
                break;
            }
        }
        else if (process->block_type == ASP_BLOCK_SCRIPT ||
                 (process->block_type == ASP_BLOCK_COMMENT && process->context_type == ASP_CONTEXT_INCLUDE))
        {
            switch (first)
            {
            case '=':
                if (!word_started)
                {
                    word_started = 1;
                    operator_found = 1;
                    *word_begin = read_pos - *bufptr - 1;
                }
                else
                {
                    ungetc(first, stream);
                    read_pos--;
                }
                goto break_char;
                break;
            case '\"':
                if (!word_started)
                {
                    word_started = 1;
                    double_quoted = 1;
                    *word_begin = read_pos - *bufptr - 1;
                }
                else if (double_quoted)
                {
                    double_quoted = 0;
                    goto break_char;
                }
                break;
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                if (word_started)
                {
                    ungetc(first, stream);
                    read_pos--;
                    goto break_char;
                }
                break;
            default:
                if (!word_started)
                {
                    word_started = 1;
                    *word_begin = read_pos - *bufptr - 1;
                }
                break;
            }
        }
        continue;
    }
    
break_char:
    /* Done - NUL terminate and return the number of chars read.  */
    *read_pos = '\0';
    
    ret = read_pos - (*bufptr);
    process->nwords++;
    return ret;
}

int pdt_escape_asp_path(const char *path)
{
    assert(path);
    return (strstr(path, "Help") || 
            (strcmp(path, "PMS") == 0) ||
            (strcmp(path, "PMS.SJIS") == 0) ||
            (strcmp(get_ext(path), ".inc")) == 0) ;
}

pdt_plugin_t pdt_asp_plugin = {
    ".asp",
    (pdt_creator_t)pdt_create_asp_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_FORMAT,
    "GetMess(%s)",
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_inc_plugin = {
    ".inc",
    (pdt_creator_t)pdt_create_asp_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_FORMAT,
    "GetMess(%s)",
    NULL,   /* csv handle */
    0,
};
