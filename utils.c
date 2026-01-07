#include "jsh.h"

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

// handles input of Ctrl-c and not exit until the exit command is entered
void signalHandler()
{
  signal(SIGINT, signalHandler);
  getchar();
}
