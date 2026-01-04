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
#include <errno.h>

#define RL_BUFF_SIZE 1024
#define TOK_DELIM " \t\r\n"
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
    p = strtok(NULL, "|");
  }
  s[i] = NULL;
  i = 0;
  while(s[i] != NULL)
  {
    printf("%s\n", s[i]);
    i++;
  }
  return s;
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
 * Starts by copying the current stdin and stdout to tempin and tempout respectively
 * then loops to check for input redirection if any
 * fdin is set to current stdin (current stdin or input redirection)
 * next, the for loop iterates over each command in the array returned by split_pipes()
 * the dup2(fdin, 0) call duplicates fdin over 0 as in it sets the stdin for current session
 * and subsequent call closes fdin because it is no longer required to be open
 *
 * subsequent if-elseif-else mechanism checks for output redirection and set fdout accordingly
 * if the command is in the last one, we would like to see the o/p on the terminal
 * so the original tempout is restorted and if neither of the two conditions are met, then the stdout
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

char *get_history_file_path()
{
  static char file_path[128];
  strncpy(file_path, getenv("HOME"), sizeof(file_path) - 1);
  file_path[sizeof(file_path) - 1] = '\0';
  strcat(file_path, "/.jsh_history");
  return file_path;
}

int jsh_history()
{
  FILE *fp = fopen(get_history_file_path(), "r");
  int ch, c, line_num = 1;
  char line[128];
  char prev_comm[128];
  char **args = NULL;
  if(!fp)
    fprintf(stderr, RED "jsh: file not found" RESET "\n");
  else
  {
    putchar('\n');
    while((c=getc(fp)) != EOF)
    {
      putchar(c);
    }
  }

  printf("\n" INVERT "<0>: QUIT   <#line> Execute command <-1> clear history" RESET"\n\n: ");
  scanf("%d", &ch);
  getchar();
  fseek(fp, 0, SEEK_SET);
  if(isdigit(ch) != 0)
  {
    printf("Please enter a number\n");
  }
  else if(ch == 0)
  {
    fclose(fp);
    return 1;
  }
  else if(ch == -1)
  {
    fclose(fp);
    fp = fopen(get_history_file_path(), "w");
    fclose(fp);
    return jsh_execute(clr);
  }
  else 
    {
      while((fgets(line, 128, fp)) != NULL)
      {
        if(line_num == ch)
        {
          strcpy(prev_comm, &line[3]);
          int p = 0, flag = 0;
          fclose(fp);
          while(prev_comm[p] != '\0')
          {
            if(prev_comm[p] == '|')
            {
              flag  = 1;
              break;
            }
            p++;
          }
          if(!flag)
          {
            args = split_line(prev_comm);
            return jsh_launch(args);
          }
          else
          {
            args = split_pipes(prev_comm);
            return jsh_pipe(args);
          }
        }
        else
          line_num++; 
      }
    }
  return 1;
}

//return the current number of lines in history file
//for appending new items in file
int history_line_count()
{
  FILE *fp = fopen(get_history_file_path(), "r");
  int c;
  int numberOfLines = 1;
  do
  {
    c = getc(fp);
    if(c == '\n')
    {
      numberOfLines++;
    }
  }while(c != EOF);
  return numberOfLines;
}

// handles input of Ctrl-c and not exit until the exit command is entered
void signalHandler()
{
  signal(SIGINT, signalHandler);
  getchar();
}

int jsh_launch(char **args)
{
  int i = 0;
  if(args[0] == NULL)
  {
    return 1;
  }
  else if(strcmp(args[0], "history") != 0 && strcmp(args[0], "exit") != 0 && strcmp(args[0], "clear") != 0)
  {
    history_input(args, " ");
  }
  for(i = 0;  i < builtin_funcs_count(); i++)
  {
    if(strcmp(args[0], builtin_str[i]) == 0)
    {
      return(*builtin_funcs[i])(args);
    }
  }
  return jsh_execute(args);
}

int jsh_grep(char **args)
{
  FILE *fp = NULL;
  int flag = 0;
  char temp[512];
  int line_num = 1;
  if(args[0] != NULL && strcmp(args[0], "grep") == 0)
  {
    if(args[1] != NULL && args[2] != NULL)
    {
      // local struct variable
      struct stat path_stat;
      
      if(stat(args[2], &path_stat) == 0)
      {
        if(S_ISDIR(path_stat.st_mode))
        {
          fprintf(stderr, RED "jsh: grep: '%s' is a directory not a file" RESET "\n", args[2]);
          return 1;
        }
      }
      fp = fopen(args[2], "r");
      
      if(fp == NULL)
      {
        switch(errno)
        {
          case ENOENT:
            fprintf(stderr, RED "jsh: grep: file '%s' does not exist" RESET "\n", args[2]);
            break;
          
          case EACCES:
            fprintf(stderr, RED "jsh: grep: permission denied for file '%s'" RESET "\n", args[2]);
            break;

          case EISDIR:
            fprintf(stderr, RED "jsh: grep: '%s' is a directory" RESET "\n", args[2]);
            break;

          default:
            fprintf(stderr, RED "jsh: grep: cannot open file '%s': %s" RESET "\n", args[2], strerror(errno));
            break;
        }
        return 1;
      }

      while((fgets(temp, 512, fp)) != NULL)
      {
        if(strstr(temp, args[1]))
        {
          printf("%d, %s", line_num, temp);
          flag = 1;
        }
        line_num++;
      }
      fclose(fp);
    }
    else
    {
      fprintf(stderr, RED "jsh: grep requires two parameters, " ITALICS "PATTERN" RESET RED " and " RED ITALICS "FILE" RESET "\n");
    }
  }
  if(flag == 0)
    printf("No matches were found \n");
  return 1;
}

/*
 * Displays a description of builtin commands on - 'help'
 * */

int jsh_help(char **args)
{
  if(args[0] != NULL && strcmp(args[0], "help") == 0)
  {
    fprintf(stderr, "\n~~~~~~~\n"
            BOLD "\njsh " RESET "is a basic unix terminal shell\n"
            "\nSupported Commands:\n1. cd\n2. exit\n3. help\n4. touch\n5. cat"
            "\n\n~~~~~~~\n\n");
  }
  return 1;
}

/*
 * Gives current working directory for prompt
 * */
void get_dir(char *state)
{
  char cwd[1024];
  if(getcwd(cwd, sizeof(cwd)) != NULL)
  {
    if(strcmp(state, "loop") == 0)
      printf(RED " [ " RESET GREEN "%s" RESET RED " ] " RESET, cwd);
    else if(strcmp(state, "pwd") == 0)
        printf("%s\n", cwd);
  }
  else 
  {
    printf("%sgetcwd() error%s", RED, RESET);
  }
}

int jsh_cd(char **args)
{
  if(args[1] == NULL)
  {
    fprintf(stderr, "%sjsh: Please enter a path to cd%s\n", YELLOW, RESET);
    return 1;
  }
  else
  {
    if(chdir(args[1]) < 0)
    {
      perror("jsh");
      return 1;
    }
  }
  return 1;
}

/*
 * Test function
 * */
void print_token(char **tokens)
{
  int i = 0;
  while(tokens[i] != NULL)
  {
    printf("%s\n", tokens[i]);
    i++;
  }
}

char *read_line()
{
  int buffsize = RL_BUFF_SIZE;
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
    } 
    else 
    {
      buffer[position] = c;
    }
    position++;

    if (position >= buffsize) 
    {
      printf("Buffer Overflow.... allocating more memory");// ?? LOL
      buffsize += RL_BUFF_SIZE;
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
  int buffsize = TK_BUFF_SIZE, position = 0;
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
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;

  return tokens;
}

int jsh_exit(char **args)
{
  (void)args;
  return 0;
}

//executes system call execvp with the tokenized user input as argument
//the execvp() call takes place in a child process
//the parent waits until the child has finished processing
int jsh_execute(char **args)
{
  pid_t cpid;
  int status;
  cpid = fork();

  if (cpid == 0)
  {
    if (execvp(args[0], args) < 0)
      printf("jsh: command not found: %s\n", args[0]);
    exit(EXIT_FAILURE);
  } else if (cpid < 0)
    printf(RED "Error forking" RESET "\n");
  else {
    waitpid(cpid, &status, WUNTRACED);
  }
  return 1;
}

/*
 * Driver Func
 *
 * status var controlled while loop
 * every iteration first prints the prompt(cwd)
 * then proceeds to read user input using the function read_line()
 * the returned stream of tokens are then passed onto the launch function
 * which returns 0 or 1 depending on the execution
 * loop exits accordingly
 * */

void loop()
{
  char *line;
  char **args;
  int status = 1, i = 0, flag = 0;

  do {
    get_dir("loop");
    printf(BLUE "> " RESET);
    line = read_line();
    flag = 0;
    i = 0;
    while(line[i] != '\0')
    {
      if(line[i] == '|')
      {
        flag = 1;
        break;
      }
      i++;
    }
    if(flag)
    {
      pipe_history_input(line);
      args = split_pipes(line);
      status = jsh_pipe(args);
    }
    else 
    {
      args = split_line(line);
      status = jsh_launch(args);
    }
    free(line);
    free(args);
  } while (status);
}

int main()
{
  loop();
  return EXIT_SUCCESS;

}
