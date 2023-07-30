#include "jbi_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Variable {
    char* name;
    char* value;
    struct Variable* next;
} Variable;

// Global linked list to store the variables
static Variable* variables = NULL;

static void jbi_add_variable(const char* name, const char* value) {
    Variable* new_var = (Variable*)malloc(sizeof(Variable));
    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->next = variables;
    variables = new_var;

}

const char* jbi_get_variable(const char* var_name) {
    Variable* current = variables;
    while (current) {
        if (strcmp(var_name, current->name) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

void jbi_handle_variable(const char* var_name, const char* var_value) {
    Variable* current = variables;
    while (current) {
        if (strcmp(var_name, current->name) == 0) {
            free(current->value);
            current->value = strdup(var_value);
            return;
        }
        current = current->next;
    }

    // If the variable does not exist in the linked list, add it
    jbi_add_variable(var_name, var_value);
}


void jbi_free_variables() {
    while (variables) {
        Variable* temp = variables;
        variables = variables->next;
        free(temp->name);
        free(temp->value);
        free(temp);
    }
}
