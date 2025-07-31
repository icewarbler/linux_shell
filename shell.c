#include "format.h"
#include "shell.h"
#include "vector.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <signal.h>
#include <sys/wait.h>

static vector * recent_history;
static const char * directory;
static pid_t fg_pid = -1;
static char * fg_cmd;
const char * space = " ";


int shell (int argc, char *argv[]) {
    signal(SIGINT, cctrl_handler);

    recent_history = vector_create(&string_copy_constructor, &string_destructor, &string_default_constructor);

    while(1){ parse_command(argc, argv); }
    return 0;
}

void parse_command(int argc, char* argv[]) {
    char cwd[1024];
    directory = getcwd(cwd, sizeof(cwd));
    pid_t pid = getpid();

    print_prompt(directory, pid);

    char * line = NULL;
    size_t buffer = 0;
    
    int line_read = getline(&line, &buffer, stdin);
    if (line_read == -1 || feof(stdin)) {
        free(line);
        exit_handler();
    } else if (!strcmp(line, "\n")) {
        free(line);
        return;
    }

    trim_newline(line);

    // Counts the number of words in the command
    int argc_sh =  1;
    for (size_t i = 0; i < strlen(line); i++) { if (line[i] == ' ') { argc_sh++; } }
    size_t argc_int = (size_t) argc_sh;

    char * user_input = malloc(strlen(line) + 1);
    user_input = strcpy(user_input, line);
    char ** argv_sh = strsplit(user_input, space, &argc_int);
    if (argc_sh == 1) { // The command is a single word
        find_cmd(argc_sh, argv_sh, line);
        free(user_input);
        for (int i = 0; i < argc_sh; i++) {
            free(argv_sh[i]);
        }
        free(line);
        free(argv_sh);
        return;
    }
    find_cmd(argc_sh, argv_sh, line);

    free(user_input);
    for(int i=0; i < argc_sh; i++) {
        free(argv_sh[i]);
    }
    free(line);
    free(argv_sh);
    return;
}

void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; 
    }
}

// Some of the following code is machine generated
char ** strsplit(const char * str, const char * delim, size_t * num_tokens) {
    char *input_copy = strdup(str);

    char *token;

    size_t count = 0;
    token = strtok(input_copy, delim);
    while (token != NULL) {
        count++;
        token = strtok(NULL, delim);
    }

    // Allocate memory for the argument vector (argv_sh)
    char **argv_sh = malloc((count + 1) * sizeof(char *));
    
    strcpy(input_copy, str);
    size_t index = 0;
    token = strtok(input_copy, delim);
    while (token != NULL) {
        argv_sh[index] = strdup(token);  
        index++;
        token = strtok(NULL, delim);
    }
    argv_sh[index] = NULL;

    free(input_copy);

    return argv_sh;
}

void find_cmd(int argc_sh, char *argv_sh[], char * line) {
    char * cmd = argv_sh[0];

    if (!strcmp(argv_sh[0], "cd")) {    // cd
        run_cd(argc_sh, argv_sh);
        vector_push_back(recent_history, line);
    } else if (!strcmp(cmd, "!history")) {  // !history
        print_history(argc_sh, argv_sh);
    } else if (!strncmp(cmd, "#", 1)) { // #[INT]
        int input_int;
        if (sscanf(cmd+1, "%d", &input_int) == 1) {
            exec_history(argc_sh, argv_sh, input_int);
        } else { print_invalid_command(line); }
    } else if (!strcmp(cmd, "exit")) {  // exit
        exit_handler();
    } else if (check_external(cmd, argv_sh)) {
        run_external(argc_sh, argv_sh, line);
    }
}

int check_external(char * cmd, char* args[]) {
    char * path = getenv("PATH");
    char * path_copy = strdup(path);
    char * dir = strtok(path_copy, ":");

    while (dir != NULL) {
        char path_buffer[1024];
        snprintf(path_buffer, sizeof(path_buffer), "%s/%s", dir, cmd); // iterates through 4 different possible directories
        if (access(path_buffer, X_OK) == 0) {   // this means the cmd was found
            free(path_copy);
            return 1;
        }
        dir = strtok(NULL, ":");
    }
 //   free(dir);
    free(path_copy);
    return 0;
}

int run_external(int argc_sh, char * argv_sh[], char * line) {
    fflush(stdout);
    fflush(stderr);

    pid_t pid = fork();

    if (pid == -1) {
        print_fork_failed();
        exit(1);
    }
    if (!pid) { // child
        signal(SIGINT, cctrl_handler);    // reset child to handle default ctrl+c
        fg_pid = getpid();
        fg_cmd = malloc(strlen(argv_sh[0])+1);
        fg_cmd = strdup(argv_sh[0]);

        int bg_group = setpgid(pid, 0); // set as bg process group
        if (bg_group == -1) { print_setpgid_failed(); }

        
        print_command_executed(getpid());
        execvp(argv_sh[0], argv_sh);
        print_exec_failed(argv_sh[0]);
        exit(1);
    } else if (pid > 0) {   // parent
        int wstatus;
        pid_t child_pid = waitpid(pid, &wstatus, 0);
        if (  child_pid == -1 ) { // process was killed by sig
            print_wait_failed();
            exit(1);
        } else if ( WIFCONTINUED(wstatus) ) { 
            print_continued_process(child_pid, argv_sh[0]); 
            return 0;
        } else if ( WIFSIGNALED(wstatus) ) {
            print_stopped_process(child_pid, argv_sh[0]);
            return 0;
        }   else if ( WIFEXITED(wstatus) ) {    // child returned successfully
            if (line != NULL) {
                vector_push_back(recent_history, line);
            }
            fg_pid = -1;
            free(fg_cmd);
            int success = kill(child_pid, SIGKILL);
            if (!success) { print_killed_process(child_pid, argv_sh[0]); }
            return 1;
        }
    }
    return 0;
}

char * args_to_cmd(int argc_sh, char * argv_sh[]) {
    int cmd_length = 0;
    for (int i = 0; i < argc_sh; i++) {
        cmd_length += strlen(argv_sh[i]);
    }
    cmd_length += argc_sh - 1; //spaces
    cmd_length++;   //null

    char * cmd = malloc(cmd_length);
    for (int i = 0; i < argc_sh; i++) {
        if (i == 0) {
            cmd = strcpy(cmd, argv_sh[i]);
        } else {
            cmd = strcat(cmd, argv_sh[i]);
        }
        if (i+1 < argc_sh) {
            cmd = strcat(cmd, " ");
        }
    }
    return cmd;
}

int run_cd(int argc_sh, char * argv_sh[]) {
    if (argc_sh > 2) { // Input is too long -- throw an error
        char * cmd = args_to_cmd(argc_sh, argv_sh);
        print_invalid_command(cmd);
        free(cmd);
        return 0;
    }
    if (chdir(argv_sh[1]) == -1) {
        print_no_directory(argv_sh[1]);
        return 0;
    } 
    return 1;
}

void print_history(int argc_sh, char * argv_sh[]) {
    if (argc_sh > 1) {  // input is too long
        char * cmd = args_to_cmd(argc_sh, argv_sh);
        print_invalid_command(cmd);
        free(cmd);
        return;
    }
    
    for (size_t i = 0; i < vector_size(recent_history); i++) {
        print_history_line(i, (char *) vector_get(recent_history, i));
    }
}

void exec_history(int argc_sh, char * argv_sh[], int input_int) {
    size_t history_length = vector_size(recent_history);
    if (argc_sh > 1) {  // Input is too long
        char * cmd = args_to_cmd(argc_sh, argv_sh);
        print_invalid_command(cmd);
        free(cmd);
        return;
    }
    if (input_int >= 0 && input_int < (int) history_length) {
        char * line = (char*) vector_get(recent_history, input_int); // Overrides whatever is stored in 'line'
        
        // Counts the number of arguments in the command from history
        int cmd_args = 1;
        for (size_t i = 0; i < strlen(line); i++) { if (line[i] == ' ') { cmd_args++; } }
        size_t args_sizet = (size_t) cmd_args;
        char ** argv_sh = strsplit(line, space, &args_sizet);

        find_cmd(cmd_args, argv_sh, line);

        for (size_t i=0; i < args_sizet; i++) {
            free(argv_sh[i]);
            argv_sh[i] = NULL;
        }
        free(argv_sh);
        argv_sh = NULL;
    } else {
        print_invalid_index();
    }
}

void cctrl_handler(int sig) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFSIGNALED(status)) {
            print_killed_process(fg_pid, fg_cmd);
        } else if (WIFSTOPPED(status)) {
            print_stopped_process(fg_pid, fg_cmd);
        }
    }
    return;
}

void exit_handler() {
    exit(0);
}