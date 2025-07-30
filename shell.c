#include "format.h"
#include "shell.h"
#include "vector.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

static vector * recent_history;
static const char * directory;


int shell (int argc, char *argv[]) {
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
    const char * space = " ";
    char ** argv_sh = strsplit(user_input, space, &argc_int);
    if (argc_sh == 1) { // The command is a single word
        find_cmd(argc_sh, argv_sh, user_input);
        free(user_input);
        for (int i = 0; i < argc_sh; i++) {
            free(argv_sh[i]);
        }
        free(line);
        free(argv_sh);
        return;
    }
    find_cmd(argc_sh, argv_sh, user_input);

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
    } 
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

char * args_to_cmd(int argc_sh, char * argv_sh[]) {
    int cmd_length = 0;
    
    for (int i = 0; i < argc_sh; i++) {
        cmd_length += strlen(argv_sh[i]);
    }
    cmd_length += argc_sh - 1; // spaces
    cmd_length++; // null

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

void exit_handler() {
    exit(0);
}