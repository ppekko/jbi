#ifndef JBI_BUILD_H
#define JBI_BUILD_H

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

#ifdef __cplusplus
extern "C" {
#endif

// Function to recursively scan a directory and collect all .c files
void jbi_scan_directory(const char* dir_path, char*** files, size_t* num_files);

// Function to execute a command and wait for it to finish
int jbi_execute_command(const char* command);

// Function to check if the object file is up-to-date with the source file and headers
int jbi_is_object_file_up_to_date(const char* src_file, const char* obj_file, const char* includes);

// Function to convert a string to lowercase
void jbi_to_lower(char* str);

// Function to compile a C file into an object file
int jbi_compile_c_file(const char* cc, const char* src_file, const char* obj_file, const char* includes, const char* cflags, const char* libs);

// Function to link the object files into a relocatable object file
int jbi_link_files(const char* cc, char** object_files, size_t num_object_files, const char* reloc_obj_file, const char* includes, const char* ldflags, const char* libs);

// Function to build a project
int jbi_build_project(char* buildpath);

#ifdef __cplusplus
}
#endif

#endif // JBI_BUILD_H
