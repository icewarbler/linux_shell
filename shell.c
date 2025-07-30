#include "format.h"
#include "shell.h"
#include <unistd.h>
#include <stdlib.h>
#include<string.h>

static const char * directory;

int main(int argc, char *argv[]) {
    return shell(argc, argv);
}

/**
*   Main method -- called when user starts shell 
*/
int shell (int argc, char *argv[]) {
    while(1){ parse_command(argc, argv); }
    return 0;
}

/**
 * Helper function called when user presses "enter"
 * Trims whitespace and searches for a valid command
 */
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

    return;
}

void exit_handler() {
    exit(0);
}