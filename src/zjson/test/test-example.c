//
// Created by ran on 24-9-30.
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "help_test/framework.h"
#include "zjson.h"

/**
*   Example: Creating a JSON Object
*   Here's an example of how to create and manipulate a JSON object:
 */
static void test_create() {
    json_value obj;
    init_as_object(&obj);

    // Add a number
    json_value *n = set_json_object_sk(&obj, "pi");
    set_json_number(n, 3.1415926);

    // Add a string
    json_value *name = set_json_object_sk(&obj, "name");
    set_json_str_sk(name, "Ran");

    // Create an array
    // Add array to object
    json_value *numbers = set_json_object_sk(&obj, "numbers");
    init_as_array(numbers, 0);
    set_json_number(insert_json_array_last(numbers), 1.0);
    set_json_number(insert_json_array_last(numbers), 2.0);
    set_json_number(insert_json_array_last(numbers), 3.0);

    // Stringify and print the JSON object
    size_t json_len;
    char *json_str = json_stringify(&obj, &json_len);
    printf("%s\n", json_str);

    // Free memory
    free(json_str);
    free_json_value(&obj);
}

static void test_parse() {
    json_value value;
    const char *json_str = "{\"name\":\"John\", \"age\":30}";

    int result = parse_json_string(&value, json_str);

    if (result == JSON_PARSE_OK) {
        // Successfully parsed the JSON
        const char *name = get_json_str(get_json_object_value(&value, "name", 4));
        json_number age = get_json_number(get_json_object_value(&value, "age", 3));
        printf("Name: %s, Age: %.0f\n", name, age);
    } else {
        printf("Failed to parse JSON\n");
    }

    // Free memory
    free_json_value(&value);
}

static void test_write_json() {
    json_value obj;
    init_as_object(&obj);

    // Add a number
    json_value *n = set_json_object_sk(&obj, "pi");
    set_json_number(n, 3.1415926);

    // Add a string
    json_value *name = set_json_object_sk(&obj, "name");
    set_json_str_sk(name, "Ran");

    // Create an array
    // Add array to object
    json_value *numbers = set_json_object_sk(&obj, "numbers");
    init_as_array(numbers, 0);
    set_json_number(insert_json_array_last(numbers), 1.0);
    set_json_number(insert_json_array_last(numbers), 2.0);
    set_json_number(insert_json_array_last(numbers), 3.0);


    int result = json_dump(&obj, "output.json");
    if (result == JSON_DUMP_OK) {
        printf("JSON dumped to file successfully.\n");
    } else {
        printf("Failed to dump JSON to file.\n");
    }

    free_json_value(&obj);
}

static void test_read_json() {
    json_value value;

    int result = json_load(&value, "input.json");
    if (result == JSON_PARSE_OK) {
        printf("JSON loaded successfully.\n");
        // Process JSON...
    } else {
        printf("Failed to load JSON.\n");
    }
    size_t len;
    char *json_str = json_stringify(&value, &len);
    printf("%s.\n", json_str);
    printf("json string length: %lu.\n", len);
    free(json_str);
    free_json_value(&value);
}

static UnitTestFunction tests[] = {
    test_create,
    test_parse,
    test_write_json,
    test_read_json,
    NULL
};

int main(int argc, char *argv[]) {
    run_tests(tests);
    return 0;
}
