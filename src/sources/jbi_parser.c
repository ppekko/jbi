#include "jbi_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jbi_config.h"

void jbi_free_variable(Variable* var) {
    free(var->name);
    free(var->value);
    free(var);
}

static void jbi_add_variable(Variable** list, const char* name, const char* value) {
    Variable* new_var = (Variable*)malloc(sizeof(Variable));
    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->next = *list;
    *list = new_var;
}

// Helper function to trim leading and trailing whitespaces from a string
char* jbi_trim_whitespace(char* str) {
    while (*str && (*str == ' ' || *str == '\t'))
        str++;
    
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n'))
        str[--len] = '\0';

    return str;
}

// Helper function to extract variable name and value from a line
int jbi_extract_variable(const char* line, char** var_name, char** var_value) {
    const char* equal_sign = strchr(line, '=');
    if (equal_sign == NULL)
        return 0;

    size_t var_name_len = equal_sign - line;

    // Remove any leading whitespaces from variable name
    while (var_name_len > 0 && (line[var_name_len - 1] == ' ' || line[var_name_len - 1] == '\t')) {
        var_name_len--;
    }

    *var_name = (char*)malloc(var_name_len + 1);
    if (*var_name == NULL) {
        printf("Memory allocation failed for var_name.\n");
        return 0;
    }
    strncpy(*var_name, line, var_name_len);
    (*var_name)[var_name_len] = '\0';

    // Find the start of the value, skipping the '=' character
    const char* value_start = equal_sign + 1;

    // Remove any leading whitespaces from the variable value
    while (*value_start && (*value_start == ' ' || *value_start == '\t')) {
        value_start++;
    }

    size_t var_value_len = strlen(value_start);
    // Remove any trailing whitespaces from variable value
    while (var_value_len > 0 && (value_start[var_value_len - 1] == ' ' || value_start[var_value_len - 1] == '\t' || value_start[var_value_len - 1] == '\n')) {
        var_value_len--;
    }

    *var_value = (char*)malloc(var_value_len + 1);
    if (*var_value == NULL) {
        printf("Memory allocation failed for var_value.\n");
        free(*var_name); // Free previously allocated memory for var_name
        return 0;
    }
    strncpy(*var_value, value_start, var_value_len);
    (*var_value)[var_value_len] = '\0';

    return 1;
}

int jbi_parse_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return 0;
    }
    Variable* variables = NULL;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *var_name, *var_value;
        if (line[0] == '\n')
            continue;
        if (jbi_extract_variable(jbi_trim_whitespace(line), &var_name, &var_value)) {

            jbi_add_variable(&variables, var_name, var_value);

            jbi_handle_variable(var_name, var_value);
        }
        free(var_name);
        free(var_value);
    }

    fclose(file);

    while (variables) {
        Variable* temp = variables;
        variables = variables->next;
        jbi_free_variable(temp);
    }
    printf("Build configuration parsed.\n");
    return 1;
}