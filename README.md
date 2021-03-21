## JSMC
A simple JSON parser, build for study.
### Example
```c
#include "jsmc.h"
int main() {
    jsmc_node *node = jsmc_create_node();
    const char *str = "{\"hello\": \"world\", \"object\": {\"num\": 666.666, \"bool\": true}}";
    const char *key;
    const char *value;
    if (jsmc_parse(node, str) != JSMC_PARSE_ERROR) {
        key = jsmc_get_object_key(jsmc_get_object_element(node, 0));
        value = jsmc_get_string(jsmc_get_object_element(node, 0));
        // jsmc_node *object = jsmc_get_object_element(node, 1);
        // double num = jsmc_get_number(jsmc_get_object_element(object, 0));
        // int bool = 2;
        // if (jsmc_get_type(jsmc_get_object_element(object, 1)) == JSMC_TRUE) {
        //     bool = 1;
        // }
        // printf("%.3lf %d\n", num, bool);
    }
    printf("%s %s!", key, value);
    jsmc_free(node);
    return 0;
}
```
### Reference
[miloyip/json-tutorial](https://github.com/miloyip/json-tutorial)

[DaveGamble/cJSON](https://github.com/DaveGamble/cJSON)
