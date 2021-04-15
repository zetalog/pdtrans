#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

void pdt_destroy_sql_processor(pdt_process_t process)
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

pdt_process_t pdt_create_sql_processor(pdt_param_t param)
{
    pdt_process_t process = NULL;
    static char replace_format[] = "translate";

    process = malloc(sizeof (struct _pdt_process_t));
    if (process)
    {
        process->language = LANGUAGE_SQL;
        process->scanner = (pdt_scanner_t)pdt_scan_sql_word;
        process->parser = (pdt_parser_t)pdt_parse_sql_word;
        process->destroyer = (pdt_destroyer_t)pdt_destroy_sql_processor;
        process->to_csv = (pdt_convert_t)pdt_sql_to_csv;
        process->from_csv = (pdt_convert_t)pdt_sql_from_csv;
        process->free_string = free;

        process->nblocks = 0;
    }

    return (pdt_process_t)process;
}

char *pdt_sql_to_csv(pdt_process_t process, const char *word_in)
{
    int index = 0;
    int word_len = 0;
    char *word_out = NULL;
    int index_out = 0;
    int append = 0;

    if (!(word_in && (strlen(word_in) >= 2) && 
         (word_in[0] == '\'') && 
         (word_in[strlen(word_in) - 1] == '\'')))
        return NULL;

    word_len = strlen(word_in);
    word_out = (char *)malloc(sizeof(char)*word_len*2+1);
    if (!word_out) {
        pdt_report_error(PDT_REPORT_ERROR, "Cannot malloc for %s in sql_to_csv.\n", word_in);
        return NULL;
    }

    word_out[index_out++] = '\"';
    index++;
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
        case '\\':
            word_out[index_out++] = '\\';
            word_out[index_out++] = '\\';
            //append++;
            break;
        case '\"':
            word_out[index_out++] = '\"';
            word_out[index_out++] = '\"';
            //append++;
            break;
        case '\n':
            word_out[index_out++] = '\\';
            word_out[index_out++] = 'n';
            //index++;
            //append++;
            break;
        case '\r':
            word_out[index_out++] = '\\';
            word_out[index_out++] = 'r';
            //append++;
            break;
        default:
            word_out[index_out++] = c;
            break;
        }

    }
    while (index < word_len - 1);
    word_out[index_out++] = '\"';
    word_out[index_out] = PDT_TERMINATOR;

#if 0
    assert( word_out && (strlen(word_out) >= 2) && (word_out[0] == '\"') && 
            (word_out[strlen(word_out) - 1+append] == '\"'));
#endif
    assert( word_out && (strlen(word_out) >= 2) && (word_out[0] == '\"') && 
            (word_out[strlen(word_out) - 1] == '\"'));

    //fprintf(stdout, "Word Out is %s\n", word_out);

    return strdup(word_out);
}

char *pdt_sql_from_csv(pdt_process_t process, const char *word_in)
{
    int index = 0;
    int word_len = 0;
    char *word_out = NULL;
    unsigned char next_char;
    int index_out = 0;

    if (!(word_in && (strlen(word_in) >= 2) && 
         (word_in[0] == '\"') && 
         (word_in[strlen(word_in) - 1] == '\"')))
        return NULL;


    word_len = strlen(word_in);
    word_out = (char *)malloc(sizeof(char)*word_len);
    if (!word_out) {
        pdt_report_error(PDT_REPORT_ERROR, "Cannot malloc for %s in sql_from_csv.\n", word_in);
        return NULL;
    }

    word_out[index_out++] = '\'';
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
        case '\\':
            next_char = word_in[index++];
            switch (next_char)
            {
            case '\\':
                word_out[index_out++] = '\\';
                break;
            case 'n':
                word_out[index_out++] = '\n';
                break;
            case 'r':
                word_out[index_out++] = '\r';
                break;
            default:
                pdt_report_error(PDT_REPORT_ERROR, "Illegal escape string for SQL found![\\%c].\n", next_char);
                break;
            }
            break;
        case '\"':
            next_char = word_in[index++];
            switch (next_char)
            {
            case '\"':
                word_out[index_out++] = '\"';
                break;
            default:
                pdt_report_error(PDT_REPORT_ERROR, "Illegal escape string for SQL found![\"%c].\n", next_char);
                break;
            }
            break;
        default:
            word_out[index_out++] = c;
            break;
        }

    }
    while (index < (word_len - 1));
    word_out[index_out++] = '\'';
    word_out[index_out] = PDT_TERMINATOR;

    assert( word_out && (strlen(word_out) >= 2) && (word_out[0] == '\'') && 
            (word_out[strlen(word_out) - 1] == '\''));

    return strdup(word_out);
}

void pdt_parse_sql_word(pdt_process_t process, char *word, FILE *stream)
{
    char *csv_in = NULL;
    char *csv_out = NULL;

    if (*word == '\'')
    {
        if (pdt_must_localize(word))
        {
            if (process->mode == PDT_MODE_REPLACE)
            {
                csv_in = pdt_sql_to_csv((pdt_process_t)process, word);
            
                if (csv_in)
                {
                    csv_out = pdt_processor_replace_word(process, csv_in);
                    if (csv_out)
                        process->replace_current = pdt_sql_from_csv((pdt_process_t)process, csv_out);
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

/**
 * Get one word from the given text. Includes quoter
 * if word is a quoted string.
 *
 * @param begin: [OUT] begin index of the word from the text
 *
 * @return: scanned word, must be destroyed outside fuction,
 *          or return NULL means no more word available
 */
int pdt_scan_sql_word(pdt_process_t process, char **bufptr, size_t *n,
                     int *word_begin, FILE *stream)
{
    int nchars_avail;   /* Allocated but unused chars in *LINEPTR.  */
    char *read_pos;     /* Where we're reading into *LINEPTR. */
    int ret;
    int word_started = 0;
    int block_commentted = 0;
    int minus_commentted = 0;

    assert(process && process->language == LANGUAGE_SQL && bufptr && n && word_begin && stream);

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

    block_commentted = 0;
    minus_commentted = 0;

    for (; ; )
    {
        char c, next;
        int res;
        
        res = read_char(stream, bufptr, n, &read_pos, &nchars_avail, &c);

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

        switch (c)
        {
        case'\'':
            if (!word_started)
            {
                if (!(( block_commentted > 0 ) ||
                    ( minus_commentted )))
                {
                    word_started = 1;
                    *word_begin = read_pos - *bufptr -1;
                }
            }
            else
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    goto break_char;

                if (next == '\'')
                    break;
                else
                {
                    ungetc(next, stream);
                    read_pos--;
                    goto break_char;
                }
            }
            break;
        case '/':
            if (!word_started) 
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    goto break_char;
                
                if (next == '*')
                    block_commentted++;
                else
                {
                    ungetc(next, stream);
                    read_pos--;
                }
            }

            break;
        case '*':
            if (!word_started) 
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    goto break_char;
                
                if (next == '/')
                    block_commentted--;
                else
                {
                    ungetc(next, stream);
                    read_pos--;
                }
            }

            break;
        case '-':
            if (!word_started) 
            {
                res = read_char (stream, bufptr, n, &read_pos, &nchars_avail, &next);
                
                if (res < 0)
                    return -1;
                else if (res == 0)
                    goto break_char;
                
                if (next == '-')
                    minus_commentted = 1;
                else
                {
                    ungetc(next, stream);
                    read_pos--;
                }
            }
            break;
        case '\n':
            minus_commentted = 0;
            break;
        case '\r':
            minus_commentted = 0;
            break;
        default:
            if (!word_started)
                *word_begin = read_pos - *bufptr;
            break;
        }
        continue;
    }
    
break_char:
    /* Done - NUL terminate and return the number of chars read.  */
    *read_pos = '\0';
    
    ret = read_pos - (*bufptr);
    process->nwords++;
    /*
    fprintf(stdout, "----------------------------------------\n");
    fprintf(stdout, "word %s\n", *bufptr + *word_begin);
    fprintf(stdout, "----------------------------------------\n");
    */
    return ret;

}

int pdt_escape_sql_path(const char *path)
{
    assert(path);
    return (strstr(path,"FRIEND2") ||
            strcmp(path,"UNICODE") == 0 ||
            strstr(path,"MsgList") ||
            (strstr(path,"STD2002Con") && stricmp(get_ext(path), ".sql")));
}

pdt_plugin_t pdt_sql_plugin = {
    ".sql",
    pdt_create_sql_processor,
    pdt_escape_sql_path,
    0,
    "japanese",
    "english",
    PDT_REPLACE_TRANSLATE,
    NULL,   /* format string */
    NULL,   /* csv handle */
    0,
};
