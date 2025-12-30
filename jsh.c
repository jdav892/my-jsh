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

// strings to clear terminal
char *clr[2] = {"clear", "null"};

// ANSI color codes
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define CYAN "\033[0;36m"
#define GREEN "\033[0;32m"
#define INVERT "\033[0;7m"
#define BOLD "\e[1m"
#define ITALICS "\e[3m"
#define RESET "\e[0m"

//declared functions
char *read_line();
char **split_line(char*);
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

// array of built in function pointers
int (*builtin_funcs[])(char **) = {&jsh_cd, &jsh_help, &jsh_exit, &jsh_history, &jsh_grep, &args_length};

// string array of built in commands for strcmp() before invoking execvp()
char *builtin_str[] = {"cd", "help", "exit", "history", "grep", "sizeof"};


// return the size of built in array

int builtin_funcs_count()
{
  return sizeof(builtin_str) /sizeof(char *);
}

// function definitions
void pipe_history_input(char * line)
{
  FILE *history_file = fopen(get_history_file_path(), "a+");
  fprintf(history_file, "%d, %s\n", history_line_count(), line);
  fclose(history_file);
}

void history_input(char **args, char *d)
{
  FILE *history_file = fopen(get_history_file_path(), "a+");
  int j = 0;
  fprintf(history_file, "%d, ", history_line_count());
  while(args[j] != NULL)
  {
    if(j > 0)
      fputs(d, history_file);
    fputs(args[j], history_file);
    j++;
  }
  fputs("\n", history_file);
  fclose(history_file);
}

char *trim_whitespace(char *str)
{
  char *end;
  while(isspace((unsigned char) *str)) str++;
  if(*str == 0)
  {
    return str;
  }
  end  = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char) *end)) end--;
  *(end + 1) = 0;
  return str;
}

char **split_pipes(char *input)
{
  char *p = strtok(input, "|");
  char **s = malloc(1024*sizeof(char *));
  int i = 0;
  while(p != NULL)
  {
    s[i] = trim_whitespace(p);
    i++;
    p = strtok(NULL, "| ");
  }
  s[i] = NULL;
  i = 0;
  while(s[i] != NULL)
  {
    printf("%s\n", s[i]);
    i++;
  }
  return 0;
}

int args_length(char **args)
{
  int i = 0;
  
  while(args[i] != NULL)
  {
    i++;
  }
  return i;
}

/*
 * Starts by coping the current stdin and stdout to tempin and tempout respectively
 * then loops to check for input redirection if any
 * fdin is set to current stdin (current stdin or input redirection)
 * next, th efor loop iterates over each command in the array returned by split_pipes()
 * the dup2(fdin, 0) call duplicates fdin over 0-as in it sets as the stdin for current session
 * and subsequent call closes fdin because it is no longer required to be open
 *
 * subsequent if-elseif-else mechanism checks for output redirection and set fdout accordingly
 * if the command in the last one, we would like to see the o/p on the terminal
 * so the original tempout is resorted and if neither of the two conditions meet, then the stdout
 * is set to the output of a new pipe created over the fd[2] variable
 *
 * now fdout is set and is duplicated to the default stdin
 * commands are then executed one by one
 * */

int jsh_pipe(char **args)
{
  // saving current stdin and stdout for restoring
  int tempin = dup(0);
  int tempout = dup(1);
  int j = 0, i = 0, flag = 0;
  int fdin = 0, fdout;

  //history_input(args)
  for(j = 0; j < args_length(args); j++)
  {
    if(strcmp(args[j], "<") == 0)
    {
      fdin = open(args[j + 1], O_RDONLY);
      flag += 2;
    }
  }

  if(!fdin)
    fdin = dup(tempin);
  int pid;
  for(i = 0; i < args_length(args) - flag; i++)
  {
    char **rargs = split_line(args[i]);
    dup2(fdin, 0);
    close(fdin);
    if(i == args_length(args) - 3 && strcmp(args[i + 1], ">") == 0)
    {
      if((fdout = open(args[i + 1], O_WRONLY)))
        i++;
    }
    else if(i == args_length(args) - flag - 1)
      fdout = dup(tempout);
    else
     {
      int fd[2];
      pipe(fd);
      fdout = fd[1];
      fdin = fd[0];
    }

    dup2(fdout, 1);
    close(fdout);

    pid = fork();
    if(pid == 0)
    {
      execvp(rargs[0], rargs);
      perror("error forking\n");
      exit(EXIT_FAILURE);
    }
    wait(NULL);
  }

  dup2(tempin, 0);
  dup2(tempout, 1);
  close(tempin);
  close(tempout);

  return i;
}

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
