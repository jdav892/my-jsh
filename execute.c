#include "jsh.h"

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
