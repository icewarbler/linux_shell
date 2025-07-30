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
 * Helper for when the user inputs an invalid [PATH]
 * Parses entry into a printable command
 */
char * args_to_cmd(int argc_sh, char * argv_sh[]);

/**
 * Implementation of strtok
 */
char** strsplit(const char * string, const char * delim, size_t * num_tokens);