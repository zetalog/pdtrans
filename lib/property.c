#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pd_intern.h>

int pdt_compare_property(pdt_property_t *property1, pdt_property_t *property2)
{
    if (property1 && property2)
    {
        if (property1->id == property2->id)
        {
            return _stricmp(property1->name, property2->name);
        }
        return property2->id - property1->id;
    }

    return property2 - property1;
}

void pdt_destroy_property(pdt_property_t *property)
{
    if (property)
    {
        if (property->name)
            free(property->name);
        free(property);
    }
}

pdt_properties_t pdt_create_properties()
{
    pdt_properties_t properties = malloc(sizeof (struct _pdt_properties_t));

    if (properties)
    {
        properties->output = FALSE;
        properties->properties = create_vector(sizeof (pdt_property_t), pdt_destroy_property, pdt_compare_property);
    }

    return properties;
}

void pdt_destroy_properties(pdt_properties_t properties)
{
    if (properties)
    {
        if (properties->properties)
            destroy_vector(properties->properties);
        free(properties);
    }
}

void pdt_append_property(pdt_properties_t properties, pdt_property_t *property)
{
    if (properties && property)
    {
        append_element(properties->properties, property, 1);
    }
}

char *pdt_create_properties_string(pdt_properties_t properties)
{
    int properties_count, index;
    pdt_property_t *pproperties = NULL;
    char *result = NULL;
    int required = 0;

    if (!properties || !properties->properties || properties->output)
        return NULL;

    properties->output = 1;
    properties_count = element_count(properties->properties);
    if (properties_count == 0)
        return NULL;
    
    required = MIN_CHUNK;
    result = malloc(required);
    if (!result)
    {
        pdt_report_error(PDT_REPORT_ERROR, "Memory allocation failure.", 0);
        return NULL;
    }

    *result = '\0';
    strcat(result, "\"");
    
    for (index = 0; index < properties_count; index++)
    {
        char id[10];

        pproperties = (pdt_property_t *) get_element(properties->properties, index);

        itoa(pproperties->id, id, 10);
        required += strlen(id)+strlen(pproperties->name)+2;

        result = realloc(result, required);
        if (!result)
        {
            pdt_report_error(PDT_REPORT_ERROR, "Memory allocation failure.", 0);
            return NULL;
        }
        strcat(result, pproperties->name);
        strcat(result, "=");
        strcat(result, id);
        
        if (index < properties_count-1)
            strcat(result, ";");
    }
    
    strcat(result, "\"");
    return result;
}

void pdt_destroy_properties_string(char *string)
{
    if (string)
        free(string);
}
