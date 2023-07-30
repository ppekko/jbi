#ifndef JBI_PARSER_H
#define JBI_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jbi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Structure to hold a single variable
typedef struct Variable {
    char* name;
    char* value;
    struct Variable* next;
} Variable;

// Helper function to trim leading and trailing whitespaces from a string
char* jbi_trim_whitespace(char* str);

// Helper function to extract variable name and value from a line
int jbi_extract_variable(const char* line, char** var_name, char** var_value);

// Function to parse the justbuild.it file
int jbi_parse_file(const char* filename);

#ifdef __cplusplus
}
#endif

#endif // JBI_PARSER_H
