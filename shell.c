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
int is_sep;
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

    if(!check_conjunction(argc_sh, argv_sh, line)  && !check_separator(argc_sh, argv_sh, line)) {
        find_cmd(argc_sh, argv_sh, user_input);
    }

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

int check_conjunction(int argc_sh, char * argv_sh[], char * line) {
    int delim_index = -1;
    int is_and = 0;
    is_sep = 0;
    for (int i = 0; i < argc_sh; i++) {
        if(!strcmp(argv_sh[i], "||") || !strcmp(argv_sh[i], "&&")) {     // found op
            delim_index = i;
            if (!strcmp(argv_sh[i], "&&")) { is_and = 1; }
        }
    }
    if (delim_index == -1) {   // not op
        return 0;
    }
    int cmd1_argc = 0;
    int cmd2_argc = 0;
    char ** cmd1_argv = make_first_command(argc_sh, argv_sh, delim_index, is_sep);
    char ** cmd2_argv = make_second_command(argc_sh, argv_sh, delim_index); 
    for (int j = 0; j < delim_index; j++) {
        cmd1_argc++;
    }
    for (int j = delim_index + 1; j < argc_sh; j++) {
        cmd2_argc++;
    }
    vector_push_back(recent_history, line);
        
    int status1 = cd_or_external(cmd1_argc, cmd1_argv, NULL);
    if (!is_and && !status1) {
        cd_or_external(cmd2_argc, cmd2_argv, NULL);
    } else if (is_and && status1) {
        cd_or_external(cmd2_argc, cmd2_argv, NULL);
    }
    for (int i = 0; i < cmd1_argc; i++) {
        free(cmd1_argv[i]);
        cmd1_argv[i] = NULL;
    }
    for(int i = 0; i < cmd2_argc; i++) {
        free(cmd2_argv[i]);
        cmd2_argv[i] = NULL;
    }
    free(cmd1_argv);
    free(cmd2_argv);
    cmd1_argv = NULL;
    cmd2_argv = NULL;
    return 1; 
}


int check_separator(int argc_sh, char * argv_sh[], char * line) {
    is_sep = 1;
    int delim_index = -1;
    for (int i = 0; i < argc_sh; i++) {
        for (int j = 0; j < (int) strlen(argv_sh[i]); j++) {
            if(argv_sh[i][j] == ';') {     // found separator
                delim_index = i;
            }
        }
    }
    if (delim_index == -1) {
        return 0;
    }

    int cmd1_argc = 0;
    int cmd2_argc = 0;

    char ** cmd1_argv = make_first_command(argc_sh, argv_sh, delim_index, is_sep);
    char ** cmd2_argv = make_second_command(argc_sh, argv_sh, delim_index);
    for (int j = 0; j <= delim_index; j++) {
        cmd1_argc++;
    }
    for (int j = delim_index + 1; j < argc_sh; j++) {
        cmd2_argc++;
    }
    
    char * str = cmd1_argv[delim_index];
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == ';') {
        str[len - 1] = '\0'; 
    }

    vector_push_back(recent_history, line);

    cd_or_external(cmd1_argc, cmd1_argv, NULL);
    cd_or_external(cmd2_argc, cmd2_argv, NULL);
    
    for (int i = 0; i < cmd1_argc; i++) {
        free(cmd1_argv[i]);
        cmd1_argv[i] = NULL;
    }
    for(int i = 0; i < cmd2_argc; i++) {
        free(cmd2_argv[i]);
        cmd2_argv[i] = NULL;
    }
    free(cmd1_argv);
    free(cmd2_argv);
        
    return 1;   
}

char ** make_first_command(int argc_sh, char * argv_sh[], int delim_index, int is_sep) {
    size_t cmd1_num = 0;
    int cmd1_length = 0;
    if (!is_sep) {
        for (int i = 0; i < delim_index; i++) {
            cmd1_length += strlen(argv_sh[i]);
            cmd1_num++;
        }
    } else {
        for (int i = 0; i <= delim_index; i++) {
            cmd1_length += strlen(argv_sh[i]);
            cmd1_num++;
        }
    }
    cmd1_length += (int) cmd1_num -1; //spaces
    cmd1_length++;       //null terminator
    char * cmd1 = (char*) malloc(cmd1_length);
    if (!is_sep) {
        for (int i = 0; i < delim_index; i++) {
            char * cmd1_part = argv_sh[i];
            if (i ==0) {
                cmd1 = strcpy(cmd1, cmd1_part);
            } else {
                cmd1 = strcat(cmd1, cmd1_part);
            }
            if (i+1 < delim_index) {
                cmd1 = strcat(cmd1, " ");
            }
        }
    } else {
        for (int i = 0; i <= delim_index; i++) {
            char * cmd1_part = argv_sh[i];
            if (i ==0) {
                cmd1 = strcpy(cmd1, cmd1_part);
            } else {
                cmd1 = strcat(cmd1, cmd1_part);
            }
            if (i + 1 < delim_index + 1) {
                cmd1 = strcat(cmd1, " ");
            }
        }
    }
    
    const char * space = " ";
    char ** cmd1_argv = strsplit(cmd1, space, &cmd1_num); 
    free(cmd1);
    return cmd1_argv;
}

char ** make_second_command(int argc_sh, char * argv_sh[], int delim_index) {
    const char * space = " ";

    int cmd2_length = 0;
    size_t cmd2_num =0;
    for (int i = delim_index + 1; i < argc_sh; i++) {
        cmd2_length += strlen(argv_sh[i]);
        cmd2_num++;
    }
    cmd2_length += (int) cmd2_num - 1; //spaces
    cmd2_length++;  // null terminator
    char * cmd2 = (char*) malloc(cmd2_length);
    for (int i = delim_index+1; i < argc_sh; i++) {
        char * cmd2_part = argv_sh[i];
        if (i == delim_index +1) {
            cmd2 = strcpy(cmd2, cmd2_part);
        } else {
            cmd2 = strcat(cmd2, cmd2_part);
        }
        if (i+1 < argc_sh) {
            cmd2 = strcat(cmd2, " ");
        }
    }
    char ** cmd2_argv = strsplit(cmd2, space, &cmd2_num);
    free(cmd2);
    return cmd2_argv;
}

int cd_or_external(int argc, char * argv_sh[], char * line) {
    if (!strcmp(argv_sh[0], "cd")) {
            int res = run_cd(argc, argv_sh);
            return res;
    } else if (check_external(argv_sh[0], argv_sh)) {
        int ret = run_external(argc, argv_sh, line);
        if (ret == 0) {
            char * cmd = args_to_cmd(argc, argv_sh);
            print_invalid_command(cmd);
            free(cmd);
            return 0;
        } else {
            return 1;
        }
    } else {
        char * cmd = args_to_cmd(argc, argv_sh);
        print_invalid_command(cmd);
        free(cmd);
        return 0;
    }
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
    } else if (!strncmp(line, "!", 1)) {
        char prefix[100];
        if (sscanf(cmd+1, "%s ", (char *) &prefix) == 1) {
            exec_prefix(argc_sh, argv_sh, prefix);
        } else { print_invalid_command(line); }
    } else if (!strcmp(cmd, "exit")) {  // exit
        exit_handler();
    } else if (check_external(cmd, argv_sh)) {
        run_external(argc_sh, argv_sh, line);
    } else {
        vector_push_back(recent_history, line);
        print_invalid_command(line);
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
    size_t cmd_length = 0;
    for (int i = 0; i < argc_sh; i++) {
        cmd_length += strlen(argv_sh[i]);
    }
    cmd_length += argc_sh - 1; //spaces
    cmd_length++;   //null

    char * cmd = (char *) malloc(cmd_length);
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

void exec_prefix(int argc_sh, char * argv_sh[], char * prefix) {
    if (!vector_size(recent_history)) {
        print_no_history_match();
        return;
    }
    if (argc_sh > 1) {
        char * cmd = args_to_cmd(argc_sh, argv_sh);
        print_invalid_command(cmd);
        free(cmd);
        return;
    }
    int found = 0;
    for (size_t i = vector_size(recent_history); i-- > 0; ) {
        char * line = (char*) vector_get(recent_history, i);
        if (!strncmp(line, prefix, strlen(prefix))) {
            found = 1;
            const char * space = " ";
            size_t args_sizet = 0;
            int cmd_args = 1;
            for (size_t i=0; i < strlen(line); i++) {  if (line[i] == ' ') { cmd_args++; } }
            args_sizet = (size_t) cmd_args;
            char ** argv_sh = strsplit(line, space, &args_sizet); 
            int has_conj = check_conjunction(cmd_args, argv_sh, line);
            int has_sep = check_separator(cmd_args, argv_sh, line);
            if (!has_conj && !has_sep) {
                find_cmd(cmd_args, argv_sh, line);
            }
            for (int i=0; i < cmd_args; i++) {
                free(argv_sh[i]);
                argv_sh[i] = NULL;
            }
            free(argv_sh);
            argv_sh = NULL;
            
            return;
        }
    }
    if (found == 0) {
        print_no_history_match();
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