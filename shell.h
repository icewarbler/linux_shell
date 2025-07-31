#include <stddef.h>

// Entry point for shell
int shell(int argc, char * argv[]);

// Called when user enters input
void parse_command(int argc, char* argv[]);

// Handles an exit from the shell
void exit_handler(); 

// Trims extra white spaces from user input
void trim_newline(char *str);

// Checks if the input is one of several pre-identified commands
void find_cmd(int argc_sh, char *argv_sh[], char * line);

/**
 * Implementation of 'cd'
 * Formatted as 'cd [PATH]'
 * @return 1 on success, 0 on failure
 */
int run_cd(int argc_sh, char * argv_sh[]);

/**
 * Helper for when the user inputs an invalid command
 * Parses entry into a printable command
 * Note: absolutely necessary, as without this the #[INT] command breaks!
 */
char * args_to_cmd(int argc_sh, char * argv_sh[]);

/**
 * Implementation of strtok
 */
char** strsplit(const char * string, const char * delim, size_t * num_tokens);

/**
 * Implementation of '!history'
 * Print the contents of the recent_history vector
 */
void print_history(int argc_sh, char * argv_sh[]);

/**
 * Execute a specific command in history
 * Formatted as '#[INTEGER]'
 * i.e. #3 will print the third command in recent_history
 */
void exec_history(int argc_sh, char * argv_sh[], int input_int);

/**
 * Checks if a command exists in the path
 */
int check_external(char * cmd, char* args[]);

/**
 * Runs a command in path
 * @return 1 on success, 0 on failure
 */
int run_external(int argc_sh, char * argv_sh[], char * line);

/**
 * Handler for ctrl+c
 */
void cctrl_handler(int sig);