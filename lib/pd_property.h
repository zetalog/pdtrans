#ifndef __PD_PROPERTY_H_INCLUDE__
#define __PD_PROPERTY_H_INCLUDE__

typedef struct _pdt_properties_t {
    /* flag of already output */
    int output;
    vector_t *properties;
} *pdt_properties_t;

/* useful for name-value pair parsing */
typedef struct _pdt_property_t {
    char *name;
    int id;
} pdt_property_t;

pdt_properties_t pdt_create_properties();
void pdt_destroy_properties(pdt_properties_t properties);
void pdt_append_property(pdt_properties_t properties, pdt_property_t *property);

char *pdt_create_properties_string(pdt_properties_t properties);
void pdt_destroy_properties_string(char *string);

void pdt_destroy_property(pdt_property_t *property);
int pdt_compare_property(pdt_property_t *property1, pdt_property_t *property2);

#endif /* __PD_PROPERTY_H_INCLUDE__ */
