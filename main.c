#include "jsh.h"

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
