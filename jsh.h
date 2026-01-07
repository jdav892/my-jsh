#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define RL_BUFF_SIZE 1024
#define TOK_DELIM " \t\r\n"
#define TK_BUFF_SIZE 64

#define INVERT "\033[0;7m"
#define BOLD "\e[1m"
#define ITALICS "\e[3m"
#define RESET "\e[0m"

// declared functions
char *read_line();
char **split_line(char *);
char *trim_whitespace(char *); // for leading whitespace
char **split_pipes(char *);
char *get_history_file_path();
int jsh_exit(char **);
int jsh_execute(char **);
int jsh_cd(char **);
int jsh_help(char **);
int jsh_grep(char **);
int jsh_launch(char **);
int history_line_count();
int jsh_history();
int jsh_pipe(char **);
int args_length(char **);
void history_input(char **, char *);
void pipe_history_input(char *);
void print_tokens(char *);
void get_dir(char *);
void signalHandler();
