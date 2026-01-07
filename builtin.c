#include "jsh.h"

int jsh_cd(char **args)
{
  if(args[1] == NULL)
  {
    fprintf(stderr, "%sjsh: please enter a path to cd%s\n", YELLOW, RESET);
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

