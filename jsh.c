#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>

#define RL_BUFF_SIZE 1024
#define TOK_DELIM "\t\r\n"
#define TK_BUFF_SIZE 64

char *clr[2] = {"clear", "null"};

// ANSI color codes
#define RED "\033[0;31m"
#define YELLOW "\003[0;33m"
#define BLUE "\003[0;34m"
#define CYAN "\003[0;36m"
#define GREEN "\003[0;32m"
#define INVERT "\003[0;7m"
#define BOLD "\e[1m"
#define ITALICS "\e[3m"
#define RESET "\e[0m"

//declared functions
char *read_line();
char **split_line(char*);
char *trim_whitespace(char *);
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
int args_lenth(char **);
void history_input(char **, char *);
void pipe_history_input(char *);
void print_tokens(char *);
void get_dir(char *);
void signalHandler();



char *read_line()
{
  int buffsize = 1024;
  int position = 0;
  char *buffer = malloc(sizeof(char) *buffsize);
  int c;

  if(!buffer)
  {
    fprintf(stderr, "%sjsh: Allocation Error%s\n", RED, RESET);
    exit(EXIT_FAILURE);
  }

  while(1)
  {
    c = getchar();
    if (c == EOF || c == '\n')
    {
      //printf("\n");
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= buffsize) 
    {
      buffsize += 1024;
      buffer = realloc(buffer, buffsize);

      if(!buffer)
      {
        fprintf(stderr, "jsh: Allocation Error \n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **split_line(char *line)
{
  int buffsize = 1024, position = 0;
  char **tokens = malloc(buffsize * sizeof(char *));
  char *token;

  if (!tokens){
    fprintf(stderr, "%sjsh: Allocation Error%s\n", RED, RESET);
    exit(EXIT_FAILURE);
  }
  token = strtok(line, TOK_DELIM);
  while (token != NULL)
  {
    tokens[position] = token;
    position++;

    if (position >= buffsize)
    {
      buffsize += TK_BUFF_SIZE;
      tokens = realloc(tokens, buffsize * sizeof(char *));

      if (!tokens)
      {
        fprintf(stderr, "%sjsh: Allocation Error%s\n", RED, RESET);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;

  return tokens;
}

int jsh_exit(char **args)
{
  return 0;
}

int jsh_execute(char **args)
{
  pid_t cpid;
  int status;

  if (strcmp(args[0], "exit") == 0)
  {
    return jsh_exit(args);
  }

  cpid = fork();

  if (cpid == 0)
  {
    if (execvp(args[0], args) < 0)
      printf("jsh: command not found: %s\n", args[0]);
    exit(EXIT_FAILURE);
  } else if (cpid < 0)
    printf(RED "Error forking"
           RESET "\n");
  else {
    waitpid(cpid, &status, WUNTRACED);
  }
  return 1;
}

void loop()
{
  char *line;
  char **args;
  int status = 1;

  do {
    printf("> ");
    line = read_line();
    args = split_line(line);
    status = jsh_execute(args);
    free(line);
    free(args);
  } while (status);
}

int main()
{
  loop();
  return 0;

}
