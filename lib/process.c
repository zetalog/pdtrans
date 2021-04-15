#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

void pdt_destroy_file_processor(pdt_process_t process)
{
    if (process)
    {
        if (process->input)
            free(process->input);
        if (process->output)
            free(process->output);
        if (process->block_strings)
            destroy_vector(process->block_strings);
        if (process->destroyer)
            (process->destroyer)(process);
    }
}

pdt_process_t pdt_create_file_processor(char *input, char *output, int mode)
{
    int index;

    char *sufix = get_ext(input);

    for (index = 0; pdt_plugins[index] != NULL; index++)
    {
        /* disabled */
        if (!pdt_plugins[index]->activated)
            continue;

        if (_stricmp(sufix, pdt_plugins[index]->sufix) == 0)
        {
            pdt_process_t process = (pdt_plugins[index]->parser_creator)(pdt_plugins[index]->create_param);

            /* disabled by process itself */
            if (!process)
                return NULL;

            pdt_init_file_processor(process, input, output, mode);

            if (mode == PDT_MODE_REPLACE)
            {
                pdt_set_processor_replacement(process,
                                              pdt_plugins[index]->replace_type,
                                              pdt_plugins[index]->replace_format);
                pdt_set_processor_resources(process,
                                            pdt_plugins[index]->csv_resources,
                                            pdt_plugins[index]->compare_language,
                                            pdt_plugins[index]->translate_language,
                                            pdt_plugins[index]->replace_type & PDT_REPLACE_CONST,
                                            process->replace_type & PDT_REPLACE_TRANSLATE);
            }
            else
            {
                pdt_set_processor_replacement(process, 0, NULL);
                pdt_set_processor_resources(process,
                                            pdt_plugins[index]->csv_resources,
                                            pdt_plugins[index]->compare_language,
                                            NULL, FALSE, FALSE);
            }
            return process;
        }
    }

    return NULL;
}

void pdt_init_file_processor(pdt_process_t process, char *input, char *output, int mode)
{
    assert(process);

    process->mode = mode;
    /* initialize replacer */
    if (process->mode == PDT_MODE_DRAW)
    {
        process->block_strings = create_vector(sizeof (char *), free, _stricmp);
    }
    else
        process->block_strings = NULL;
    process->filtered = 0;
    
    if (input)
        process->input = strdup(input);
    else
        process->input = NULL;
    if (output)
        process->output = strdup(output);
    else
        process->output = NULL;
}

void pdt_set_processor_replacement(pdt_process_t process, unsigned long replace_type,
                                   char *replace_format)
{
    assert(process);

    process->replace_type = replace_type;
    process->replace_format = replace_format;
}

void pdt_set_processor_resources(pdt_process_t process, vector_t *resources, char *language,
                                 char *translation, int generate_const, int must_translate)
{
    int locale_index = 0;
    int const_index = 0;
    int translate_index = 0;

    assert(resources && language);

    process->csv_resources = resources;
    process->compare_language = language;
    process->translate_language = translation;
    process->generate_const = generate_const;

    locale_index = pdt_find_csv_locale(process->csv_resources, language);
    if (locale_index >= 0)
        process->resources = get_element(process->csv_resources, locale_index);
    else
        process->resources = NULL;
    
    if (!process->resources || !process->resources->strings)
    {
        pdt_report_error(PDT_REPORT_WARNING, "Can not find resource for %s.", language);
    }
    
    if (generate_const)
    {
        const_index = pdt_find_csv_locale(process->csv_resources, "const");
        if (const_index >= 0)
            process->constants = get_element(process->csv_resources, const_index);
        else
            process->constants = NULL;
        
        if (!process->constants || !process->constants->strings)
        {
            pdt_report_error(PDT_REPORT_WARNING, "Can not find constants, use value directly.", 0);
        }
    }
    else
        process->constants = NULL;
    
    if (must_translate)
    {
        translate_index = pdt_find_csv_locale(process->csv_resources, translation);
        if (translate_index >= 0)
            process->translate = get_element(process->csv_resources, translate_index);
        else
            process->translate = NULL;
        
        if (!process->translate || !process->translate->strings)
        {
            pdt_report_error(PDT_REPORT_WARNING, "Can not find translation for %s.", process->input);
        }
    }
    else
        process->translate = NULL;

    process->before_string = NULL;
    process->after_string = NULL;
    process->replace_current = NULL;
    process->should_draw = 0;

    process->nblocks = -1;
    process->nwords = -1;
}

void pdt_process_stream(FILE *in_stream, FILE *out_stream, pdt_process_t process)
{
    char *buffer = NULL;
    int size = 0;
    int length = 0;

    int begin;

    assert(process && in_stream &&
           (process->mode == PDT_MODE_DRAW || (process->mode == PDT_MODE_REPLACE && out_stream)));

    while (!feof(in_stream))
    {
        char *word = NULL;

        length = (process->scanner)(process, &buffer, &size, &begin, in_stream);

        if (length < 0)
        {
            /* do not report error in the end of file */
            if (!feof(in_stream))
            {
                pdt_report_error(PDT_REPORT_ERROR, "Read %s file's word failure.", process->input);
            }
            break;
        }

        if (begin > -1)
        {
            word = buffer+begin;
            assert(process->nwords >= 0);

            /* always parse word */
            (process->parser)(process, word, in_stream);
            
            if (!stricmp(word, VB_COLUMNHEADER)) {
                int index = 0;
                char ch;
                assert(*(word + strlen(VB_COLUMNHEADER) + 2));
                ch = *(word + strlen(VB_COLUMNHEADER) + index + 1);
                while (strchr("0123456789", *(word + strlen(VB_COLUMNHEADER) + index + 1)))
                {
                    *(word + strlen(VB_COLUMNHEADER) + index ) = *(word + strlen(VB_COLUMNHEADER) + index + 1);
                    index++;
                } 
                *(word + strlen(VB_COLUMNHEADER) + index ) = PDT_TERMINATOR;
                
            }

            if (process->mode == PDT_MODE_REPLACE)
            {
                if (process->before_string)
                    fprintf(out_stream, "%s", process->before_string);
                
                if (process->replace_current)
                    word = process->replace_current;
            
                pdt_output_word(buffer, begin, word, out_stream);
            
                if (process->after_string)
                    fprintf(out_stream, "%s", process->after_string);
            }
            else if (process->mode == PDT_MODE_DRAW)
            {
                pdt_draw_csv_step(process, word);
            }
        }
    }

    if (process->mode == PDT_MODE_DRAW)
    {
        /* commit found results */
        pdt_draw_csv_end(process);
    }
    
    if (buffer) free(buffer);
}

void pdt_process_file(char *input, char *output, int mode, int escape)
{
    FILE *in_stream = NULL;
    FILE *out_stream = NULL;

    pdt_process_t process = NULL;

    assert(input && (mode == PDT_MODE_DRAW || (mode == PDT_MODE_REPLACE && output)));

    if (pdt_is_sufix_escape(get_ext(input), escape))
        return;

    process = pdt_create_file_processor(input, output, mode);
    if (!process)
    {
        if (mode == PDT_MODE_REPLACE)
            pdt_copy_file(input, output);
    }
    else
    {
        in_stream = fopen(input, "r");
        if (!in_stream)
        {
            pdt_report_error(PDT_REPORT_WARNING, "Cannot open input file - %s.", input);
            goto failure;
        }

        if (mode == PDT_MODE_REPLACE)
        {
            char *ext = NULL;

            out_stream = fopen(output, "w");
            if (!out_stream)
            {
                pdt_report_error(PDT_REPORT_WARNING, "Cannot open output file - %s.", output);
                goto failure;
            }
        }
        
        pdt_process_stream(in_stream, out_stream, process);
    }

failure:
    if (in_stream) fclose(in_stream);
    if (out_stream) fclose(out_stream);
    if (process) pdt_destroy_file_processor(process);
}

int pdt_process_folder(char *input, char *output, int mode, int escape, char *vb_trans)
{
    struct direct *dirp;
    DIR *dp;
    int count = 0;
    char input_path[MAX_PATH];
	char output_path[MAX_PATH];

    assert(input && (mode == PDT_MODE_DRAW || (mode == PDT_MODE_REPLACE && output)));

	if (mode == PDT_MODE_REPLACE && !pdt_compare_dir(input, output))
    {
        pdt_report_error(PDT_REPORT_ERROR, "Do not use same target as source for replacement.", 0);
        return 0;
	}
    if ((dp = opendir(input)) == NULL)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Can not read directory %s.", input);
        return 0;
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        int temp_escape = escape;

        if (strcmp(dirp->d_name, ".") == 0 ||
            strcmp(dirp->d_name, "..") == 0)
            continue;

        temp_escape |= pdt_escape_sufix_path(dirp->d_name);
        
        if (*(input+strlen(input)-1) == '\\')
        {
			sprintf(input_path, "%s%s", input, dirp->d_name);
            if (mode == PDT_MODE_REPLACE && output)
                sprintf(output_path, "%s%s", output, dirp->d_name);
        }
		else
        {
			sprintf(input_path, "%s\\%s", input, dirp->d_name);
            if (mode == PDT_MODE_REPLACE && output)
                sprintf(output_path, "%s\\%s", output, dirp->d_name);
        }

        if (pdt_is_dir(input_path)) 
        {
            if (mode == PDT_MODE_REPLACE)
                mkdir(output_path);
            count += pdt_process_folder(input_path, output?output_path:NULL, mode, temp_escape, vb_trans);
        }
        else
        {
            if (mode == PDT_MODE_REPLACE && !pdt_find_sufix_processor(input_path))
            {
                pdt_copy_file(input_path, output_path);
            }
            else
            {
                /* count execution */
                count++;
                pdt_process_file(input_path, output?output_path:NULL, mode, temp_escape);

#ifdef ENABLE_VB_TRANSLATION
                if (mode == PDT_MODE_REPLACE && vb_trans)
                {
                    char *ext = strdup(get_ext(input_path));
                    
                    if (ext && strlen(ext) > 0 &&
                        (stricmp(ext, ".frm") == 0 || stricmp(ext, ".ctl") == 0))
                    {
                        int replace_mode = PDT_REPLACE_TRANSLATE;
                        char *format = NULL;
                        char *trans_output = NULL;
                        int file_len = strlen(output_path);
                        int trans_len = strlen(vb_trans);
                        int length = 0;
                        
                        pdt_modify_sufix_replacement(ext, &replace_mode, &format);
                        
                        trans_output = malloc(file_len + trans_len + 2);
                        if (trans_output)
                        {
                            memcpy(trans_output+length, output_path, file_len-4);
                            length += file_len-4;
                            *(trans_output+length) = '_';
                            length += 1;
                            memcpy(trans_output+length, vb_trans, trans_len);
                            length += trans_len;
                            memcpy(trans_output+length, ext, strlen(ext));
                            length += strlen(ext);
                            *(trans_output+length) = '\0';
                            length += 1;

                            assert(length == file_len+trans_len+2);
                            
                            pdt_process_file(input_path, trans_output, mode, temp_escape);
                            free(trans_output);
                        }

                        pdt_modify_sufix_replacement(ext, &replace_mode, &format);
                    }
                    
                    if (ext) free(ext);
                }
#endif
            }

        }
    }
    closedir(dp);
	return count;
}

/**
 * get replaced string
 *
 * @param strings: string table for compare
 * @param consts: constant table for ID represents
 * @param format: replaced string format (PDLoadString(%s) or GetMess(%s))
 * @param string: quoted string to be replaced
 */
char *pdt_processor_replace_word(pdt_process_t process, char *word)
{
    /* word should be replaced here */
    int id;
    char *constant_value = NULL;
    char *translate_value;
    char id_value[20];
    char *result = NULL;
    
    assert(process && word);

    id = find_element(process->resources->strings, word) + 1;
    if (id > 0)
    {
        if ((process->replace_type & PDT_REPLACE_CONST) && process->constants)
        {
            char *temp_const = get_element(process->constants->strings, id-1);
            constant_value = strdup(temp_const+1);
            *(constant_value+(strlen(constant_value)-1)) = '\0';
        }
        else
            constant_value = NULL;
        
        if ((process->replace_type & PDT_REPLACE_TRANSLATE) && process->translate)
        {
            translate_value = get_element(process->translate->strings, id-1);
        }
        else
            translate_value = NULL;

        if (process->replace_type & PDT_REPLACE_TRANSLATE)
        {
            if (translate_value && strlen(translate_value) > 0)
                result = strdup(translate_value);
            else
            {

                pdt_report_error(PDT_REPORT_ERROR, "Can not find translation for %s.[%s]", process->input);
                fprintf(stdout,"Error[%s]\n", word);
            }
        }
        else
            result = pdt_format_resource(process->replace_format, itoa(id, id_value, 10), constant_value);
    }
    else
    {
        pdt_report_error(PDT_REPORT_WARNING, "Can not find resource %s.", word);
    }

    if (constant_value)
    {
        assert(!strchr(constant_value, '\"'));
        free(constant_value);
    }

    return result;
}

void pdt_save_rc_resources(vector_t *resources, FILE *stream)
{
    int count;
    int index;

    assert(resources && stream);

    count = element_count(resources);

    /* generate output rc file */
    fprintf(stream, "/* Generated by pd_resgen. Do not modify this file directly. */\n");
    fprintf(stream, "#include <winres.h>\n\n");
    
    for (index = 0; index < count; index++)
    {
		pdt_resource_t *resource = get_element(resources, index);
        if (resource)
        {
            char *language = resource->language;
            int resource_count = element_count(resource->strings);
            int resource_index;

            if (!strcmp(language, "english"))
            {
                fprintf(stream, "LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US\n");
                fprintf(stream, "#pragma code_page(1252)\n");
            }
            else if (!strcmp(language, "chinese"))
            {
                fprintf(stream, "LANGUAGE LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED\n");
                fprintf(stream, "#pragma code_page(936)\n");
            }
            else if (!strcmp(language, "japanese"))
            {
                fprintf(stream, "LANGUAGE LANG_JAPANESE, SUBLANG_DEFAULT\n");
                fprintf(stream, "#pragma code_page(932)\n");
            }
            
            fprintf(stream, "STRINGTABLE DISCARDABLE\n");
            fprintf(stream, "BEGIN\n");
            
            for (resource_index = 0; resource_index < resource_count; resource_index++)
            {
                char *value = get_element(resource->strings, resource_index);
                if (value && strlen(value))
                    fprintf(stream, "    %d                       %s\n", resource_index+1, value);
            }
            
            fprintf(stream, "END\n\n");
        }
    }
}

void pdt_draw_csv_resources(vector_t *vector, char *language, char *keyword)
{
    assert(language);
    set_element(vector, 0, pdt_create_resource(language, keyword));
}

void pdt_draw_csv_end(pdt_process_t process)
{
    int index;
    int count;
    pdt_resource_t *resource;
    char *keyword;

    assert(process->mode == PDT_MODE_DRAW && process->block_strings);
    resource = get_element(process->csv_resources, 0);
    assert(resource && resource->strings);

    keyword = (char *)resource->handler;

    if (process->filtered)
    {
        count = element_count(process->block_strings);
        for (index = 0; index < count; index++)
            append_element(resource->strings, strdup(get_element(process->block_strings, index)), 1);
    }
    process->filtered = 0;
}

void pdt_draw_csv_step(pdt_process_t process, char *word)
{
    int index;
    int count;
    pdt_resource_t *resource;
    char *keyword;

    assert(process->mode == PDT_MODE_DRAW && word && process->block_strings);
    resource = get_element(process->csv_resources, 0);
    assert(resource && resource->strings);

    keyword = (char *)resource->handler;

    if (process->nwords == 0)
    {
        if (process->filtered)
        {
            count = element_count(process->block_strings);
            for (index = 0; index < count; index++)
                append_element(resource->strings, strdup(get_element(process->block_strings, index)), 1);
        }

        destroy_vector(process->block_strings);
        process->block_strings = create_vector(sizeof (char *), process->free_string, stricmp);
        process->filtered = 0;
    }
    if (!keyword || (keyword && stricmp(word, keyword) == 0))
    {
        process->filtered = 1;
    }
    if (process->should_draw && pdt_must_localize(word))
    {
        char *temp = process->to_csv(process, word);

        if (temp)
            append_element(process->block_strings, temp, 1);
    }
}
