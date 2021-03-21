#ifndef JSMC_H__
#define JSMC_H__

typedef enum {
    JSMC_NULL,
    JSMC_TRUE,
    JSMC_FALSE,
    JSMC_NUMBER,
    JSMC_STRING,
    JSMC_ARRAY,
    JSMC_OBJECT
} jsmc_type;

enum {
    JSMC_PARSE_OK,
    JSMC_PARSE_ERROR
};

typedef struct jsmc_node {
    jsmc_type type;
    struct jsmc_node *child;
    struct jsmc_node *next;
    char *name;
    union {
        char *string;
        double number;
    };
} jsmc_node;

jsmc_node *jsmc_create_node();
void jsmc_free(jsmc_node *node);
int jsmc_parse(jsmc_node *node, const char *json);

jsmc_type jsmc_get_type(jsmc_node *node);
double jsmc_get_number(jsmc_node *node);
const char *jsmc_get_string(jsmc_node *node);
jsmc_node *jsmc_get_array_element(jsmc_node *node, int index);
jsmc_node *jsmc_get_object_element(jsmc_node *node, int index);
const char *jsmc_get_object_key(jsmc_node *element);

#endif //JSMC_H__
