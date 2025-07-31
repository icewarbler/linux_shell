/**
 *  Formatting for the shell
 *  Courtesy of https://cs341.cs.illinois.edu/
 */

#pragma once
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

/**
 * Print when the user tries to change directories but `path` does not exist.
 */
void print_no_directory(const char *path);

/**
 * Print when executing an external command with a process id `pid`.
 */
void print_command_executed(pid_t pid);

/**
 * Print when fork fails.
 */
void print_fork_failed();

/**
 * Print when exec('command') fails.
 */
void print_exec_failed(const char *command);

/**
 * Print when wait fails.
 */
void print_wait_failed();

/**
 * Print when setpgid fails.
 */
void print_setpgid_failed();

/**
 * Print when use of built-in 'command' is invalid,
 * such as `kill` without pid.
 */
void print_invalid_command(const char *command);

/**
 * Print when a process was successfully killed.
 *
 * pid      process id of killed process
 * command  command process was running
 */
void print_killed_process(int pid, char *command);

/**
 * Print when process with process ID `pid` has been stopped.
 */
void print_stopped_process(int pid, char *command);

/**
 * Print when process with process ID `pid` was sent the `SIGCONT` signal.
 */
void print_continued_process(int pid, char *command);

/**
 * Print a single line of history. Both `index` and `command` are the same as
 * in the history log.
 */
void print_history_line(size_t index, const char *command);

/**
 * Print when the user specifies an out-of-bounds history command.
 */
void print_invalid_index();

/**
 * Print when the history search fails (no command has the prefix).
 */
void print_no_history_match();