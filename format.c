/**
 *  Formatting for the shell
 *  Courtesy of https://cs341.cs.illinois.edu/
 */

#include "format.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_prompt(const char *directory, pid_t pid) {
   printf("(pid=%d)%s$ ", pid, directory);
}

void print_no_directory(const char *path) {
    printf("%s: No such file or directory!\n", path);
}

void print_invalid_command(const char *command) {
    printf("Invalid command: %s\n", command);
}

void print_history_line(size_t index, const char *command) {
    printf("%zu\t%s\n", index, command);
}

void print_invalid_index() {
    printf("Invalid Index!\n");
}