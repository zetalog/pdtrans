#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

void pdt_destroy_rc_processor(rc_process_t process)
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

pdt_process_t pdt_create_rc_processor(pdt_param_t param)
{
    cpp_process_t process = NULL;

    process = malloc(sizeof (struct _rc_process_t));
    if (process)
    {
        process->language = LANGUAGE_RC;
        process->scanner = (pdt_scanner_t)pdt_scan_rc_word;
        process->parser = (pdt_parser_t)pdt_parse_rc_word;
        process->destroyer = (pdt_destroyer_t)pdt_destroy_rc_processor;

        process->to_csv = (pdt_convert_t)pdt_rc_to_csv;
        process->from_csv = (pdt_convert_t)pdt_rc_from_csv;
        process->free_string = free;

        process->nblocks = 0;
    }

    return (pdt_process_t)process;
}

void pdt_parse_rc_word(rc_process_t process, char *word, FILE *stream)
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
                csv_in = pdt_rc_to_csv(process, word);
            
                if (csv_in)
                {
                    csv_out = pdt_processor_replace_word((pdt_process_t)process, csv_in);
                    if (csv_out)
                    {
                        if (process->replace_type & PDT_REPLACE_TRANSLATE)
                            process->replace_current = pdt_rc_from_csv(process, csv_out);
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

int pdt_scan_rc_word(rc_process_t process, char **bufptr, size_t *n,
                     int *word_begin, FILE *stream)
{
    int nchars_avail;   /* Allocated but unused chars in *LINEPTR.  */
    char *read_pos;     /* Where we're reading into *LINEPTR. */
    int ret;

    int word_started = 0;
    int double_quoted = 0;
    int line_commented = 0;
    int block_commented = 0;
    int star_occured = 0;
    int reading_operator = 0;

    int more_line = 0;
    
    assert(process && process->language == LANGUAGE_RC && bufptr && n && word_begin && stream);

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
        else if (double_quoted)
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
            else if (double_quoted && (c == '\"'))
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);

				if (next == '\"')
                {
                    goto next_char;
                }
                else
                {
                    ungetc (next, stream);
                    read_pos--;

					double_quoted = 0;
					break;
                }
              
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

char *pdt_rc_to_csv(rc_process_t process, const char *word_in)
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
        pdt_report_error(PDT_REPORT_ERROR, "Cannot malloc for %s in rc_to_csv.\n", word_in);
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
			if('\"'== *(word_in+index))
			{
				index++;
			}
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

char *pdt_rc_from_csv(rc_process_t process, const char *word_in)
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
        pdt_report_error(PDT_REPORT_ERROR, "Cannot malloc for %s in rc_from_csv.\n", word_in);
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

void pdt_store_rc_property(vector_t *names, char *name)
{
    if (names && name)
        append_element(names, strdup(name), 1);
}

pdt_plugin_t pdt_rc_plugin = {
    ".rc",
    pdt_create_rc_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_TRANSLATE,
    NULL,
    NULL,   /* csv handle */
    0,
};

pdt_plugin_t pdt_rc2_plugin = {
    ".rc2",
    pdt_create_rc_processor,
    pdt_escape_asp_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_TRANSLATE,
    NULL,   /* format string */
    NULL,   /* csv handle */
    0,
};
