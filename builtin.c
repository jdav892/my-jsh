#include "jsh.h"

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
