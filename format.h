/**
 *  Formatting for the shell
 *  Courtesy of https://cs341.cs.illinois.edu/
 */
#pragma once
// #include "linux_libs/unistdwind.h"
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

 /**
 * Struct with information about a process
 *
 * pid      process id
 * nthreads number of threads for process
 * vsize    size of program (virtual) memory in kb
 * state    process state/status
 * start_str process start time (in HH:MM format)
 * time_str process execution time (in M:SS format)
 * command  command process is executing
 */
typedef struct process_info {
    int pid;
    long int nthreads;
    unsigned long int vsize;
    char state;
    char *start_str;
    char *time_str;
    char *command;
} process_info;

/**
 * Print the shell prompt with current directory
 * `directory` and current process id `pid`.
 */
void print_prompt(const char *directory, pid_t pid);