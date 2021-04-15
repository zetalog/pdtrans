#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

pdt_resource_t *pdt_create_resource(char *language, void *handler)
{
    char *lang = "english";
    pdt_resource_t *resource;

    if (language && strlen(language) > 0)
        lang = language;

    resource = (pdt_resource_t *)malloc(sizeof (pdt_resource_t));
    if (resource)
    {
        resource->handler = handler;
        resource->language = strdup(lang);
        resource->strings = create_vector(sizeof (vector_t), free, strcmp);
    }

    return resource;
}

void pdt_destroy_resource(pdt_resource_t *resource)
{
    if (resource)
    {
        if (resource->language)
            free(resource->language);
        if (resource->strings)
            destroy_vector(resource->strings);
        free(resource);
    }
}

int pdt_compare_resource(pdt_resource_t *resource1, pdt_resource_t *resource2)
{
    if (resource1 && resource2)
    {
        if (resource1->language && resource2->language)
        {
            int result = _stricmp(resource1->language, resource2->language);
            if (result == 0)
                return result;
            /* add content comparation */
            else
                return compare_vector(resource1->strings, resource2->strings);
        }
        return (resource2->language - resource1->language);
    }

    return (resource2 - resource1);
}

char *pdt_combine_resource_prefix(vector_t *prefixes, char *name)
{
    char *result = NULL;
    int count;
    int index;
    int required = 0;

    assert(prefixes && name);

    count = element_count(prefixes);
    required = strlen(name)+2;

    for (index = 0; index < count; index++)
    {
        char *prefix = get_element(prefixes, index);
        required += strlen(prefix)+1;

        if (result == NULL)
        {
            result = malloc(required);
            memset(result, 0, required);
            strcpy(result, prefix);
        }
        else
        {
            result = realloc(result, required);
            strcat(result, prefix);
        }
        strcat(result, ".");
    }

    strcat(result, name);
    return result;
}

char *pdt_format_resource(char *format, char *id, char *constant)
{
    int length;
    char *result = NULL;
    char *value = (constant && (strlen(constant) > 0))?constant:id;

    assert(format && value);

    length = strlen(format) + strlen(value);
    result = malloc(length);

    if (result)
    {
        sprintf(result, format, /*length-1, */value);
    }
    else
    {
        pdt_report_error(PDT_REPORT_ERROR, "No memory to combine strings.", 0);
    }

    return result;
}

void pdt_save_csv_resources(vector_t *resources, FILE *stream)
{
    int i, j;
    int rows = -1;
    int columns = -1;

    assert(resources && stream);

    columns = element_count(resources);

    fprintf(stream, "\"id\",");

    /* first round: caculate rows, generate titles */
    for (i = 0; i < columns; i++)
    {
        pdt_resource_t *resource = get_element(resources, i);
        int count = element_count(resource->strings);

        if (count > rows)
            rows = count;

        fprintf(stream, "\"%s\"", resource->language);
        if (i < columns-1)
            fprintf(stream, ",");
        else
            fprintf(stream, "\n");
    }

    /* second round: generate contents */
    for (j = 0; j < rows; j++)
    {
        fprintf(stream, "\"%d\",", j+1);

        for (i = 0; i < columns; i++)
        {
            pdt_resource_t *resource = get_element(resources, i);
            char *value = get_element(resource->strings, j);

            fprintf(stream, "%s", value?value:"\"\"");

            if (i < columns-1)
                fprintf(stream, ",");
            else
                fprintf(stream, "\n");
        }
    }
}

vector_t *pdt_parse_csv_quoted(char *line)
{
    int quoted = 0;
    char *string = line;
    char *matched = line;
    vector_t *vector = create_vector(sizeof (char *), free, strcmp);

    for (string; *string; string++)
    {
        if (*string >= 0x80)
        {
            string++;
            continue;
        }
        if (*string == PDT_QUOTER)
        {
            if (quoted)
            {
                char next = *(string+1);

                if (next != PDT_QUOTER)
                {
                    *(string+1) = PDT_TERMINATOR;
                    quoted = 0;
                    append_element(vector, strdup(matched), 0);
                }

                string++;
            }
            else
            {
                quoted = 1;
                matched = string;
            }
        }
    }

    return vector;
}

pdt_handle_t pdt_startup()
{
    return (pdt_handle_t)create_vector(sizeof (vector_t), pdt_destroy_resource, pdt_compare_resource);
}

void pdt_cleanup(vector_t *resources)
{
    destroy_vector(resources);
}

typedef struct _pdt_traverser_t {
    int index;
    char *language;
} pdt_traverser_t;

int pdt_csv_locale_traverse(pdt_traverser_t *traverser, int index, pdt_resource_t *resource)
{
    if (_stricmp(resource->language, traverser->language) == 0)
    {
        traverser->index = index;
        return VECTOR_TRAVERSE_BREAK;
    }

    return !(VECTOR_TRAVERSE_BREAK);
}

/*
 * Get index of resources with the same language name
 */
int pdt_find_csv_locale(vector_t *resources, char *language)
{
    pdt_traverser_t traverser;

    assert(resources);

    traverser.index = -1;
    traverser.language = language;

    traverse_vector(resources, pdt_csv_locale_traverse, &traverser);

    return traverser.index;
}

void pdt_load_csv_resources(vector_t *resources, FILE *stream, int id, int start)
{
    int lineno = -1;
    int index;
    int count;
    int id_index = id;
    int line_id = 0;

    int found_id = 0;

    int length;
    char *buffer = NULL;
    int size = 0;

    /* read input csv file */
    vector_t *messages = NULL;
    vector_t *title = NULL;

    assert(resources && stream);

    title = create_vector(sizeof (vector_t), free, _stricmp);
    if (!title)
    {
        pdt_report_error(PDT_REPORT_ERROR, "create_vector failure", 0);
        return;
    }

    while (!feof(stream))
    {
        length = getline(&buffer, &size, stream);
        chomp(buffer);
        //fprintf(stdout, "buffer is %s\n", buffer);

        messages = pdt_parse_csv_quoted(buffer);

        if (lineno == -1)
        {
            /* title line */
            count = element_count(messages);
            for (index = 0; index < count; index++)
            {
		        char *value = (char *)get_element(messages, index);

                value++;
                *(value+strlen(value)-1) = '\0';

                if (_stricmp(value, "id") == 0)
                {
                    id_index = index;
                    found_id = 1;
                }
                else if (index >= start)
                {
                    /* ignore index less than start limit */

                    if (pdt_find_csv_locale(resources, value) < 0)
                        append_element(resources, pdt_create_resource(value, NULL), 1);
                }

                set_element(title, index, strdup(value));
            }
        }
        else
        {
            for (index = 0; index < count; index++)
            {
		        char *value = (char *)get_element(messages, index);

                if (!value)
                    continue;

                if (found_id && index == id_index)
                {
                    value++;
                    *(value+strlen(value)-1) = '\0';

                    line_id = atoi(value);
                    if (line_id < 1)
                    {
                        pdt_report_error(PDT_REPORT_WARNING, "Invalid ID value at line %d", lineno+1);
                        break;
                    }
                }
                else if (index >= start)
                {
                    char *language = get_element(title, index);
                    int found = 0;

                    found = pdt_find_csv_locale(resources, language);

                    if (found > -1)
                    {
                        /* we have stored this locale before */
                        set_element(((pdt_resource_t *)get_element(resources, found))->strings,
                                    found_id?line_id-1:lineno, strdup(value));
                    }
                }
            }
        }
        
        if (messages)
        {
            destroy_vector(messages);
            messages = NULL;
        }
        
        lineno++;
    }

    if (buffer) free(buffer);
    destroy_vector(title);
}

int pdt_append_locale_string(vector_t *vector, char *language, char *value)
{
    int index = pdt_find_csv_locale(vector, language);
    pdt_resource_t *resource = get_element(vector, index);
    append_element(resource->strings, strdup(value), 1);

    return find_element(resource->strings, value);
}

void pdt_set_locale_string(vector_t *vector, char *language, int index, char *value)
{
    int locale_index = pdt_find_csv_locale(vector, language);
    pdt_resource_t *resource = get_element(vector, locale_index);

    if (resource)
        set_element(resource->strings, index, strdup(value));
}

char *pdt_get_locale_string(vector_t *vector, char *language, int index)
{
    int locale_index = pdt_find_csv_locale(vector, language);
    pdt_resource_t *resource = get_element(vector, locale_index);

    if (!resource)
        return NULL;
    else
        return get_element(resource->strings, index);
}

void pdt_copy_resource_row(vector_t *from, vector_t *to, int from_index, int to_index)
{
    int locale_count;
    int locale_index;

    assert(from && to && from_index > -1);

    locale_count = element_count(from);
    for (locale_index = 0; locale_index < locale_count; locale_index++)
    {
        pdt_resource_t *resource = get_element(from, locale_index);
        char *language = NULL;

        assert(resource);
        language = resource->language;
        assert(language);

        if (pdt_find_csv_locale(to, language) < 0)
            append_element(to, pdt_create_resource(language, NULL), 1);

        if (to_index > -1)
        {
            if (!pdt_get_locale_string(to, language, to_index))
                pdt_set_locale_string(to, language, to_index, get_element(resource->strings, from_index));
        }
        else
        {
            pdt_append_locale_string(to, language, get_element(resource->strings, from_index));
        }
    }
}

pdt_handle_t pdt_diff_csv_resources(pdt_handle_t from, pdt_handle_t to, char *language, int patch)
{
    int index;
    pdt_resource_t *from_resource, *to_resource;
    vector_t *results = NULL;
    int from_row = -1;
    int to_row = -1;
    int to_count;
    int from_count;

    assert(from && to && language);

    index = pdt_find_csv_locale(from, language);
    if (index < 0)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Can not find %s locale.", language);
        goto failure;
    }
    from_resource = get_element(from, index);

    index = pdt_find_csv_locale(to, language);
    if (index < 0)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Can not find %s locale.", language);
        goto failure;
    }
    to_resource = get_element(to, index);

    /* initialize results */
    results = pdt_startup();
    set_element(results, 0, pdt_create_resource("diff", NULL));
    set_element(results, 1, pdt_create_resource(language, NULL));

    to_count = element_count(to_resource->strings);
    for (index = 0; index < to_count; index++)
    {
		char *value = get_element(to_resource->strings, index);
        if (value)
        {
            int set_index;

            from_row = find_element(from_resource->strings, value);

            /* this is an plus row */
            if (!patch && from_row < 0 || patch)
                set_index = pdt_append_locale_string(results, language, value);
            if (from_row < 0)
                pdt_set_locale_string(results, "diff", set_index, "\"+\"");
            else if (patch)
                pdt_set_locale_string(results, "diff", set_index, "\"=\"");
            if (patch)
                pdt_copy_resource_row(to, results, index, set_index);
            if (from_row > -1 && patch)
            {
                /* need to combine tow translation */
                pdt_copy_resource_row(from, results, from_row, set_index);
            }
        }
        else
            pdt_report_error(PDT_REPORT_WARNING, "String %d might not be empty.", index+1);
    }

    from_count = element_count(from_resource->strings);
    for (index = 0; index < from_count; index++)
    {
		char *value = get_element(from_resource->strings, index);
        if (value)
        {
            to_row = find_element(to_resource->strings, value);
            if (to_row < 0)
            {
                /* this is an minus row */
                int set_index;

                /* this is an plus row */
                set_index = pdt_append_locale_string(results, language, value);
                pdt_set_locale_string(results, "diff", set_index, "\"-\"");
                if (patch)
                    pdt_copy_resource_row(from, results, index, set_index);
            }
        }
        else
            pdt_report_error(PDT_REPORT_WARNING, "String %d might not be empty.", index+1);
    }

failure:
    return results;
}
