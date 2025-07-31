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

void print_fork_failed() {
    printf("Fork Failed!\n");
}

void print_no_directory(const char *path) {
    printf("%s: No such file or directory!\n", path);
}

void print_command_executed(pid_t pid) {
    printf("Command executed by pid=%d\n", pid);
}

void print_exec_failed(const char *command) {
    printf("%s: not found\n", command);
}

void print_wait_failed() {
    printf("Failed to wait on child!\n");
}

void print_setpgid_failed() {
    printf("Failed to start new process group!\n");
}

void print_invalid_command(const char *command) {
    printf("Invalid command: %s\n", command);
}

void print_stopped_process(int pid, char *command) {
    printf("%d suspended\t%s\n", pid, command);
}

void print_killed_process(int pid, char *command) {
    printf("%d killed\t%s\n", pid, command);
}

void print_continued_process(int pid, char *command) {
    printf("%d continued\t%s\n", pid, command);
}

void print_history_line(size_t index, const char *command) {
    printf("%zu\t%s\n", index, command);
}

void print_invalid_index() {
    printf("Invalid Index!\n");
}

void print_no_history_match() {
    printf("No Match!\n");
}