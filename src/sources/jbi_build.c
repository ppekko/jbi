#include "jbi_build.h"
#include "jbi_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define LINK_CMD_SIZE 8192

void jbi_free_string_array(char** array, size_t num_elements) {
    if (!array)
        return;

    for (size_t i = 0; i < num_elements; ++i) {
        free(array[i]);
        array[i] = NULL; // avoid double freeing
    }
    free(array);
}

void jbi_to_lower(char* str) {
    for (size_t i = 0; i < strlen(str); ++i) {
        str[i] = tolower(str[i]);
    }
}

int jbi_is_object_file_up_to_date(const char* src_file, const char* obj_file, const char* includes) {
    struct stat src_stat, obj_stat;
    if (stat(src_file, &src_stat) == -1) {
        perror("Failed to get source file stat");
        return 0;
    }
    if (stat(obj_file, &obj_stat) == -1)
        return 0;

    if (src_stat.st_mtime > obj_stat.st_mtime) {
        return 0;
    }

    // Check if any header file is newer than the object file
    char command[4096];
    snprintf(command, sizeof(command), "find %s -type f -name '*.h' -newer %s -print -quit", includes, obj_file);
    FILE* file = popen(command, "r");
    if (file) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            pclose(file);
            return 0;
        }
        pclose(file);
    }

    printf("(skipped)...");
    return 1; 
}

// Function to concatenate strings safely with size checks
void jbi_safe_strcat(char* dest, size_t dest_size, const char* src) {
    strncat(dest, src, dest_size - strlen(dest) - 1);
}

int jbi_execute_command(const char* command) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork error");
        return 0;
    } else if (pid == 0) {
        // Child process
        char command_copy[BUFFER_SIZE];
        snprintf(command_copy, sizeof(command_copy), "%s", command);
        char* argv[] = {"/bin/sh", "-c", command_copy, NULL};
        execv("/bin/sh", argv);
        _exit(EXIT_FAILURE); // Exec failed, exit the child process
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        return !(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    }
}

// Function to recursively scan a directory and collect all files
void jbi_scan_directory(const char* dir_path, char*** files, size_t* num_files) {
    DIR* dir = opendir(dir_path);
    if (!dir)
        return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            *files = (char**)realloc(*files, (*num_files + 1) * sizeof(char*));
            (*files)[*num_files] = strdup(entry->d_name);
            (*num_files)++;
        } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subdir_path[PATH_MAX];
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", dir_path, entry->d_name);
            jbi_scan_directory(subdir_path, files, num_files);
        }
    }
    closedir(dir);
}

int jbi_compile_file(const char* cc, const char* src_file, const char* obj_file, const char* includes, const char* cflags, const char* libs) {
    char compile_cmd[BUFFER_SIZE] = {0};
    jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), cc);

    if (includes) {
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), " -I");
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), includes);
    }

    if (libs && strcmp(cc, "tcc")) { // Ensure tcc compatibility
        char* libs_lowercase = strdup(libs);
        jbi_to_lower(libs_lowercase);

        char* rest1 = strdup(libs_lowercase);
        char* token;
        char* rest = rest1;

        while ((token = strtok_r(rest, " ", &rest))) {
            char lib_include_cmd[BUFFER_SIZE];
            snprintf(lib_include_cmd, sizeof(lib_include_cmd), " $(pkg-config --cflags %s)", token);
            jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), lib_include_cmd);
        }
        free(libs_lowercase);
        free(rest1);
    }

    if (cflags) {
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), " ");
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), cflags);
    }

    if (strstr(cc, "tcc") == NULL) { // Ensure tcc compatibility
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), " -MMD -MF \"");
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), obj_file);
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), ".d\" ");
    } else {
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), " -MD -MF \"");
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), obj_file);
        jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), ".d\" ");
    }

    jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), "-c -o \"");
    jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), obj_file);
    jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), "\" \"");
    jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), src_file);
    jbi_safe_strcat(compile_cmd, sizeof(compile_cmd), "\"");

    return jbi_execute_command(compile_cmd) == 0;
}

int jbi_link_files(const char* cc, char** object_files, size_t num_object_files, const char* reloc_obj_file, const char* includes, const char* ldflags, const char* libs) {
    char link_cmd[LINK_CMD_SIZE] = {0};
    jbi_safe_strcat(link_cmd, sizeof(link_cmd), cc);

    if (includes) {
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), " -I");
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), includes);
    }

    for (size_t i = 0; i < num_object_files; ++i) {
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), " \"build/");
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), object_files[i]);
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), "\"");
    }

    if (libs) {
        char* libs_copy = strdup(libs);
        char* token;
        char* rest = libs_copy;
        while ((token = strtok_r(rest, " ", &rest))) {
            char lib_flag[BUFFER_SIZE];
            snprintf(lib_flag, sizeof(lib_flag), " -l%s", token);
            jbi_safe_strcat(link_cmd, sizeof(link_cmd), lib_flag);
        }
        free(libs_copy);
    }

    if (ldflags) {
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), " ");
        jbi_safe_strcat(link_cmd, sizeof(link_cmd), ldflags);
    }

    jbi_safe_strcat(link_cmd, sizeof(link_cmd), " -o \"");
    jbi_safe_strcat(link_cmd, sizeof(link_cmd), reloc_obj_file);
    jbi_safe_strcat(link_cmd, sizeof(link_cmd), "\"");

    return jbi_execute_command(link_cmd) == 0;
}

int jbi_build_project(char* buildpath) {
    char* cwd = getcwd(NULL, 0);
    chdir(buildpath); // Build in this path and copy the file to the path above when done
    const char* exe = jbi_get_variable("exe");
    const char* cc = jbi_get_variable("cc");
    const char* src = jbi_get_variable("src");

    if (!exe || !cc || !src) {
        printf("ERROR: exe, cc, or src was not defined in justbuild.it. Stopping build...\n");
        return 0;
    }

    const char* include = jbi_get_variable("include");
    const char* libs = jbi_get_variable("libs");
    const char* cflags = jbi_get_variable("cflags");
    const char* ldflags = jbi_get_variable("ldflags");
    const char* pre = jbi_get_variable("pre");
    const char* post = jbi_get_variable("post");

    if (pre && jbi_execute_command(pre) != 0) {
        return 0;
    }

    // Create a build folder if it doesn't exist
    if (access("build", F_OK) == -1) {
        mkdir("build", 0777);
    }

    // Scan the src directory for .c files
    char** c_files = NULL;
    size_t num_c_files = 0;
    jbi_scan_directory(src, &c_files, &num_c_files);

    for (size_t i = 0; i < num_c_files; ++i) {
        printf("[%zu/%zu] Compiling %s...", i + 1, num_c_files, c_files[i]);
        char src_file[256];
        snprintf(src_file, sizeof(src_file), "%s/%s", src, c_files[i]);

        char obj_file[256];
        snprintf(obj_file, sizeof(obj_file), "build/%s.o", c_files[i]);
        fflush(NULL);

        if (jbi_is_object_file_up_to_date(src_file, obj_file, include) == 0) {
            int compile_result = jbi_compile_file(cc, src_file, obj_file, include, cflags, libs);
            if (compile_result == 0) {
                printf("\nCompilation of %s failed.\n", c_files[i]);

                // Free allocated memory before returning
                jbi_free_string_array(c_files, num_c_files);
                return 0;
            }
            printf("Done!\n");
        } else {
            printf("Done!\n");
        }
    }

    printf("Linking...");
    // Scan the build folder for .o files
    char** object_files = NULL;
    size_t num_object_files = 0;
    DIR* build_dir = opendir("build");
    if (build_dir) {
        struct dirent* entry;
        while ((entry = readdir(build_dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                const char* file_extension = strrchr(entry->d_name, '.');
                if (file_extension && (strcmp(file_extension, ".o") == 0)) {
                    object_files = (char**)realloc(object_files, (num_object_files + 1) * sizeof(char*));
                    object_files[num_object_files] = strdup(entry->d_name);
                    num_object_files++;
                }
            }
        }
        closedir(build_dir);
    }

    // Build the link command and link
    char exe_path[256];
    snprintf(exe_path, sizeof(exe_path), "%s/%s", cwd, exe);

    int link_result = jbi_link_files(cc, object_files, num_object_files, exe_path, include, ldflags, libs);
    if (link_result == 0) {
        printf("Linking failed.\n");
        jbi_free_string_array(c_files, num_c_files);
        jbi_free_string_array(object_files, num_object_files);
        return 0;
    }

    if (post && jbi_execute_command(post) != 0) {
        return 0;
    }

    printf("Done!\n");

    jbi_free_string_array(c_files, num_c_files);
    jbi_free_string_array(object_files, num_object_files);

    // Return to the original working directory
    chdir(cwd);
    free(cwd);

    return 1;
}
