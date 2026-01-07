#include "jsh.h"

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
