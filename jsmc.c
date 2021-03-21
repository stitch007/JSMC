#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include "jsmc.h"

typedef struct {
    const char *json;
} jsmc_context;

static void jsmc_skip_whitespace(jsmc_context *context);

static int jsmc_parse_null(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_true(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_false(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_string(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_number(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_array(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_object(jsmc_node *node, jsmc_context *context);
static int jsmc_parse_value(jsmc_node *node, jsmc_context *context);

static void jsmc_skip_whitespace(jsmc_context *context) {
    while (*context->json == ' ' || *context->json == '\r' || *context->json == '\t' || *context->json == '\n') {
        context->json++;
    }
}

static int jsmc_parse_null(jsmc_node *node, jsmc_context *context) {
    if (context->json[0] != 'n' || context->json[1] != 'u' || context->json[2] != 'l' ||
        context->json[3] != 'l') {
        return JSMC_PARSE_ERROR;
    }
    node->type = JSMC_NULL;
    context->json += 4;
    return JSMC_PARSE_OK;
}

static int jsmc_parse_true(jsmc_node *node, jsmc_context *context) {
    if (context->json[0] != 't' || context->json[1] != 'r' || context->json[2] != 'u' ||
        context->json[3] != 'e') {
        return JSMC_PARSE_ERROR;
    }
    node->type = JSMC_TRUE;
    context->json += 4;
    return JSMC_PARSE_OK;
}

static int jsmc_parse_false(jsmc_node *node, jsmc_context *context) {
    if (context->json[0] != 'f' || context->json[1] != 'a' || context->json[2] != 'l' ||
        context->json[3] != 's' || context->json[4] != 'e') {
        return JSMC_PARSE_ERROR;
    }
    node->type = JSMC_FALSE;
    context->json += 5;
    return JSMC_PARSE_OK;
}

static int jsmc_parse_string(jsmc_node *node, jsmc_context *context) {
    const char *str_begin = context->json;
    const char *str = str_begin + 1;
    int size = 0;
    while (*str != '\"') {
        if (*str++ == '\\') str++;
        size++;
    }
    str = str_begin + 1;
    char *result = (char *)malloc(size + 1);
    size = 0;
    while (1) {
        char ch = *str++;
        switch (ch) {
            case '\"':
                context->json = str;
                result[size] = '\0';
                node->string = result;
                node->type = JSMC_STRING;
                return JSMC_PARSE_OK;
            case '\\':
                switch (*str++) {
                    case '\"': result[size++] = '\"'; break;
                    case '\\': result[size++] = '\\'; break;
                    case '/':  result[size++] = '/';  break;
                    case 'b':  result[size++] = '\b'; break;
                    case 'f':  result[size++] = '\f'; break;
                    case 'n':  result[size++] = '\n'; break;
                    case 'r':  result[size++] = '\r'; break;
                    case 't':  result[size++] = '\t'; break;
                    default:
                        return JSMC_PARSE_ERROR;
                }
                break;
            case '\0':
                return JSMC_PARSE_ERROR;
            default:
                if ((unsigned char )*str < 0x20) {
                    return JSMC_PARSE_ERROR;
                }
                result[size++] = ch;
                break;
        }
    }
}

static int jsmc_parse_number(jsmc_node *node, jsmc_context *context) {
    const char *t = context->json;
    if (*t == '-') {
        t++;
    }
    if (!(*t >= '0' && *t <= '9')) {
        return JSMC_PARSE_ERROR;
    }
    char *str;
    errno = 0;
    node->number = strtod(context->json, &str);
    if (errno == ERANGE && (node->number == HUGE_VAL || node->number == -HUGE_VAL)) {
        return JSMC_PARSE_ERROR;
    }
    node->type = JSMC_NUMBER;
    context->json = str;
    return JSMC_PARSE_OK;
}

static int jsmc_parse_array(jsmc_node *node, jsmc_context *context) {
    if (*context->json++ == '[') {
        node->type = JSMC_ARRAY;
        jsmc_skip_whitespace(context);
    }
    if (*context->json == ']') {
        context->json++;
        return JSMC_PARSE_OK;
    }
    jsmc_node *child = jsmc_create_node();
    if (jsmc_parse_value(child, context) != JSMC_PARSE_OK) {
        return JSMC_PARSE_ERROR;
    }
    node->child = child;
    jsmc_skip_whitespace(context);
    while (*context->json == ',') {
        context->json++;
        jsmc_skip_whitespace(context);
        jsmc_node *next = jsmc_create_node();
        if (jsmc_parse_value(next, context) != JSMC_PARSE_OK) {
            return JSMC_PARSE_ERROR;
        }
        child->next = next;
        child = child->next;
        jsmc_skip_whitespace(context);
    }
    if (*context->json++ == ']') {
        return JSMC_PARSE_OK;
    }
    else return JSMC_PARSE_ERROR;
}

static int jsmc_parse_object(jsmc_node *node, jsmc_context *context) {
    if (*context->json++ == '{') {
        node->type = JSMC_OBJECT;
        jsmc_skip_whitespace(context);
    }
    if (*context->json == '}') {
        context->json++;
        return JSMC_PARSE_OK;
    }
    jsmc_node *child = jsmc_create_node();
    jsmc_node *key = jsmc_create_node();
    if (jsmc_parse_string(key, context) != JSMC_PARSE_OK) {
        return JSMC_PARSE_ERROR;
    }
    jsmc_skip_whitespace(context);
    if (*context->json++ != ':') return JSMC_PARSE_ERROR;
    jsmc_skip_whitespace(context);
    if (jsmc_parse_value(child, context) != JSMC_PARSE_OK) {
        return JSMC_PARSE_ERROR;
    }
    node->child = child;
    node->child->name = key->string;
    memset(key, 0, sizeof(jsmc_node));
    jsmc_skip_whitespace(context);
    while (*context->json == ',') {
        context->json++;
        jsmc_skip_whitespace(context);
        if (jsmc_parse_string(key, context) != JSMC_PARSE_OK) {
            return JSMC_PARSE_ERROR;
        }
        jsmc_skip_whitespace(context);
        if (*context->json++ != ':') {
            return JSMC_PARSE_ERROR;
        }
        jsmc_skip_whitespace(context);
        jsmc_node *next = jsmc_create_node();
        if (jsmc_parse_value(next, context) != JSMC_PARSE_OK) {
            return JSMC_PARSE_ERROR;
        }
        next->name = key->string;
        memset(key, 0, sizeof(jsmc_node));
        child->next = next;
        child = child->next;
        jsmc_skip_whitespace(context);
    }
    free(key);
    if (*context->json++ == '}') {
        jsmc_skip_whitespace(context);
        return JSMC_PARSE_OK;
    }
    else return JSMC_PARSE_ERROR;
}

static int jsmc_parse_value(jsmc_node *node, jsmc_context *context) {
    switch (*context->json) {
        case 'n':  return jsmc_parse_null(node, context);
        case 't':  return jsmc_parse_true(node, context);
        case 'f':  return jsmc_parse_false(node, context);
        case '\"': return jsmc_parse_string(node, context);
        case '[':  return jsmc_parse_array(node, context);
        case '{':  return jsmc_parse_object(node, context);
        default:   return jsmc_parse_number(node, context);
        case '\0': return JSMC_PARSE_ERROR;
    }
}

int jsmc_parse(jsmc_node *node, const char *json) {
    jsmc_context context = { json };
    jsmc_skip_whitespace(&context);
    if (jsmc_parse_value(node, &context) == JSMC_PARSE_OK) {
        jsmc_skip_whitespace(&context);
        if (*context.json != '\0') {
            return JSMC_PARSE_ERROR;
        }
    }
    return JSMC_PARSE_OK;
}

jsmc_node *jsmc_create_node() {
    jsmc_node *node = (jsmc_node *)malloc(sizeof(jsmc_node));
    memset(node, 0, sizeof(jsmc_node));
    return node;
}

void jsmc_free(jsmc_node *node) {
    jsmc_node *next = NULL;
    while (node) {
        next = node->next;
        if (node->child) {
            jsmc_free(node->child);
        }
        if (node->type == JSMC_STRING) {
            free(node->string);
        }
        if (node->name) {
            free(node->name);
        }
        free(node);
        node = next;
    }
}

jsmc_type jsmc_get_type(jsmc_node *node) {
    return node->type;
}

double jsmc_get_number(jsmc_node *node) {
    return node->number;
}

const char *jsmc_get_string(jsmc_node *node) {
    return node->string;
}

jsmc_node *jsmc_get_array_element(jsmc_node *node, int index) {
    jsmc_node *next = node->child;
    for (int i = 0; i < index; i++) {
        next = next->next;
    }
    return next;
}

jsmc_node *jsmc_get_object_element(jsmc_node *node, int index) {
    jsmc_node *next = node->child;
    for (int i = 0; i < index; i++) {
        next = next->next;
    }
    return next;
}

const char *jsmc_get_object_key(jsmc_node *element) {
    return element->name;
}
