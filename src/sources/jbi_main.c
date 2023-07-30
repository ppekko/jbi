#include "jbi_config.h"
#include "jbi_parser.h"
#include "jbi_build.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <ftw.h>

static void jbi_print_help() {
    printf("Usage: jbi [OPTIONS] <path to a folder with justbuild.it>\n");
    printf("OPTIONS:\n");
    printf("  -h  Print this help message\n");
    printf("  -c  Clean project\n");
}

static int jbi_find_argument(int argc, char* argv[], const char* argument) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], argument) == 0) {
            return 1;
        }
    }
    return 0;
}

// Helper function to parse inputted paths
static char* jbi_correct_input(const char* path, int jbiswitch) {
    // Create a copy of the path to ensure we don't modify the original
    char* input_path = strdup(path);

    // Handle special cases: ".", ".."
    if (strcmp(input_path, ".") == 0) {
        free(input_path);
        input_path = getcwd(NULL, 0);
    } else if (strcmp(input_path, "..") == 0) {
        char* cwd = getcwd(NULL, 0);
        chdir("..");
        free(input_path);
        input_path = getcwd(NULL, 0);
        chdir(cwd);
        free(cwd);
    }

    if (jbiswitch) {
        // Calculate the length required for the new string
        size_t newLen = snprintf(NULL, 0, "%s%s", input_path, "/justbuild.it") + 1;
        char* newString = (char*)malloc(newLen);

        if (newString != NULL) {
            // Concatenate the path with "/justbuild.it"
            snprintf(newString, newLen, "%s%s", input_path, "/justbuild.it");
        } else {
            printf("Memory allocation failed.\n");
            free(input_path);
            return NULL;
        }

        free(input_path);
        return newString;
    } else {
        char* newString = (char*)malloc(strlen(input_path) + 1); // +1 for the null terminator
        if (newString != NULL) {
            strcpy(newString, input_path);
        } else {
            printf("Memory allocation failed.\n");
            free(input_path);
            return NULL;
        }

        free(input_path);
        return newString;
    }
}

static unsigned int jbi_file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    } else {
        return 0;
    }
}


int main(int argc, char* argv[]) {
    printf("jbi - just build it                              ver 1.00\n");
    printf("small and simple build system\n");
    printf("by pipe                         https://github.com/ppekko\n\n");

    if (jbi_find_argument(argc, argv, "-h")) {
        jbi_print_help();
        return 0;
    }

    char* jbipath;
    char* bpath;
    if (argc == 1  || jbi_find_argument(argc, argv, "-c")) {
        jbipath = jbi_correct_input(".", 1);
        bpath = jbi_correct_input(".", 0);
    } else {
        jbipath = jbi_correct_input(argv[argc - 1], 1);
        bpath = jbi_correct_input(argv[argc - 1], 0);
    }

    if (!jbi_file_exists(jbipath)) {
        printf("ERROR: %s does not exist.\n", jbipath);
        printf("\nNo arguments specified, displaying help text\n");
        jbi_print_help();
        free(jbipath);
        free(bpath);
        return 1;
    }

    jbi_parse_file(jbipath);
    free(jbipath);

    if (jbi_find_argument(argc, argv, "-c")) {
        char clean_command[200];
        sprintf(clean_command, "rm -R \"%s/build/\" && rm -R \"%s/%s\"", bpath, bpath, jbi_get_variable("exe"));

        if (!jbi_execute_command(clean_command)) {
            printf("Object files cleaned.\n");
        }
        free(bpath);
        return 0;
    }

    
    if (jbi_build_project(bpath)) {
        printf("Build Complete!\n");
    } else {
        printf("Build Failed\n");
        free(bpath);
        return 1;
    }

    free(bpath);
    return 0;
}
