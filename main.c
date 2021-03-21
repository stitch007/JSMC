#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jsmc.h"

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
    } while(0)

#define EXPECT_EQ_INT(expect, actual)    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual) EXPECT_EQ_BASE((strcmp(expect, actual) == 0), expect, actual, "%s")

static void test();
static void test_ntf();
static void test_number();
static void test_parse_string();
static void test_parse_array();
static void test_parse_object();

static int test_count = 0;
static int test_pass = 0;

int main() {
    test();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return 0;
}

static void test() {
    test_ntf();
    test_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();
}

static void test_ntf() {
    jsmc_node *node = jsmc_create_node();
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, "null"));
    EXPECT_EQ_INT(JSMC_NULL, jsmc_get_type(node));
    memset(node, 0, sizeof(jsmc_node));
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, "true"));
    EXPECT_EQ_INT(JSMC_TRUE, jsmc_get_type(node));
    memset(node, 0, sizeof(jsmc_node));
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, "false"));
    EXPECT_EQ_INT(JSMC_FALSE, jsmc_get_type(node));
    memset(node, 0, sizeof(jsmc_node));
    free(node);
}

#define TEST_NUMBER(expect, json)\
    do {\
        EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, json));\
        EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(node));\
        EXPECT_EQ_DOUBLE(expect, jsmc_get_number(node));\
        memset(node, 0, sizeof(jsmc_node));\
    } while(0)

static void test_number() {
    jsmc_node *node = jsmc_create_node();
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000");
    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324");
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
    free(node);
}

#define TEST_STRING(expect, json)\
    do {\
        EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, json));\
        EXPECT_EQ_INT(JSMC_STRING, jsmc_get_type(node));\
        EXPECT_EQ_STRING(expect, jsmc_get_string(node));\
        free(node->string);\
        memset(node, 0, sizeof(jsmc_node));\
    } while(0)

static void test_parse_string() {
    jsmc_node *node = jsmc_create_node();
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    free(node);
}

static void test_parse_array() {
    jsmc_node *node = jsmc_create_node();
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, "[ ]"));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(node));
    free(node);

    node = jsmc_create_node();
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(node));
    EXPECT_EQ_INT(JSMC_NULL, jsmc_get_type(jsmc_get_array_element(node, 0)));
    EXPECT_EQ_INT(JSMC_FALSE, jsmc_get_type(jsmc_get_array_element(node, 1)));
    EXPECT_EQ_INT(JSMC_TRUE, jsmc_get_type(jsmc_get_array_element(node, 2)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(node, 3)));
    EXPECT_EQ_INT(JSMC_STRING, jsmc_get_type(jsmc_get_array_element(node, 4)));
    EXPECT_EQ_DOUBLE(123.0, jsmc_get_number(jsmc_get_array_element(node, 3)));
    EXPECT_EQ_STRING("abc", jsmc_get_string(jsmc_get_array_element(node, 4)));
    jsmc_free(node);

    node = jsmc_create_node();
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(node));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(jsmc_get_array_element(node, 0)));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(jsmc_get_array_element(node, 1)));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(jsmc_get_array_element(node, 2)));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(jsmc_get_array_element(node, 3)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(jsmc_get_array_element(node, 1), 0)));
    EXPECT_EQ_DOUBLE(0.0, jsmc_get_number(jsmc_get_array_element(jsmc_get_array_element(node, 1), 0)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(jsmc_get_array_element(node, 2), 0)));
    EXPECT_EQ_DOUBLE(0.0, jsmc_get_number(jsmc_get_array_element(jsmc_get_array_element(node, 2), 0)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(jsmc_get_array_element(node, 2), 0)));
    EXPECT_EQ_DOUBLE(1.0, jsmc_get_number(jsmc_get_array_element(jsmc_get_array_element(node, 2), 1)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(jsmc_get_array_element(node, 3), 0)));
    EXPECT_EQ_DOUBLE(0.0, jsmc_get_number(jsmc_get_array_element(jsmc_get_array_element(node, 3), 0)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(jsmc_get_array_element(node, 3), 1)));
    EXPECT_EQ_DOUBLE(1.0, jsmc_get_number(jsmc_get_array_element(jsmc_get_array_element(node, 3), 1)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_array_element(jsmc_get_array_element(node, 3), 2)));
    EXPECT_EQ_DOUBLE(2.0, jsmc_get_number(jsmc_get_array_element(jsmc_get_array_element(node, 3), 2)));
    jsmc_free(node);
}

static void test_parse_object() {
    jsmc_node *node = jsmc_create_node();
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node, " { } "));
    EXPECT_EQ_INT(JSMC_OBJECT, jsmc_get_type(node));
    jsmc_free(node);

    node = jsmc_create_node();
    EXPECT_EQ_INT(JSMC_PARSE_OK, jsmc_parse(node,
                                            " { "
                                            "\"n\" : null , "
                                            "\"f\" : false , "
                                            "\"t\" : true , "
                                            "\"i\" : 123 , "
                                            "\"s\" : \"abc\", "
                                            "\"a\" : [ 1, 2, 3 ],"
                                            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                            " } "
    ));
    EXPECT_EQ_INT(JSMC_OBJECT, jsmc_get_type(node));
    EXPECT_EQ_STRING("n", jsmc_get_object_key(jsmc_get_object_element(node, 0)));
    EXPECT_EQ_INT(JSMC_NULL, jsmc_get_type(jsmc_get_object_element(node, 0)));
    EXPECT_EQ_STRING("f", jsmc_get_object_key(jsmc_get_object_element(node, 1)));
    EXPECT_EQ_INT(JSMC_FALSE, jsmc_get_type(jsmc_get_object_element(node, 1)));
    EXPECT_EQ_STRING("t", jsmc_get_object_key(jsmc_get_object_element(node, 2)));
    EXPECT_EQ_INT(JSMC_TRUE, jsmc_get_type(jsmc_get_object_element(node, 2)));
    EXPECT_EQ_STRING("i", jsmc_get_object_key(jsmc_get_object_element(node, 3)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_object_element(node, 3)));
    EXPECT_EQ_DOUBLE(123.0, jsmc_get_number(jsmc_get_object_element(node, 3)));
    EXPECT_EQ_STRING("s", jsmc_get_object_key(jsmc_get_object_element(node, 4)));
    EXPECT_EQ_INT(JSMC_STRING, jsmc_get_type(jsmc_get_object_element(node, 4)));
    EXPECT_EQ_STRING("abc", jsmc_get_string(jsmc_get_object_element(node, 4)));
    EXPECT_EQ_STRING("a", jsmc_get_object_key(jsmc_get_object_element(node, 5)));
    EXPECT_EQ_INT(JSMC_ARRAY, jsmc_get_type(jsmc_get_object_element(node, 5)));
    for (int i = 0; i < 3; i++) {
        jsmc_node *next = jsmc_get_array_element(jsmc_get_object_element(node, 5), i);
        EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(next));
        EXPECT_EQ_DOUBLE(i + 1.0, jsmc_get_number(next));
    }
    EXPECT_EQ_STRING("o", jsmc_get_object_key(jsmc_get_object_element(node, 6)));
    EXPECT_EQ_INT(JSMC_OBJECT, jsmc_get_type(jsmc_get_object_element(node, 6)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_object_element(jsmc_get_object_element(node, 6), 0)));
    EXPECT_EQ_DOUBLE(1.0, jsmc_get_number(jsmc_get_object_element(jsmc_get_object_element(node, 6), 0)));
    EXPECT_EQ_STRING("1", jsmc_get_object_key(jsmc_get_object_element(jsmc_get_object_element(node, 6), 0)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_object_element(jsmc_get_object_element(node, 6), 1)));
    EXPECT_EQ_DOUBLE(2.0, jsmc_get_number(jsmc_get_object_element(jsmc_get_object_element(node, 6), 1)));
    EXPECT_EQ_STRING("2", jsmc_get_object_key(jsmc_get_object_element(jsmc_get_object_element(node, 6), 1)));
    EXPECT_EQ_INT(JSMC_NUMBER, jsmc_get_type(jsmc_get_object_element(jsmc_get_object_element(node, 6), 2)));
    EXPECT_EQ_DOUBLE(3.0, jsmc_get_number(jsmc_get_object_element(jsmc_get_object_element(node, 6), 2)));
    EXPECT_EQ_STRING("3", jsmc_get_object_key(jsmc_get_object_element(jsmc_get_object_element(node, 6), 2)));
    jsmc_free(node);
}
