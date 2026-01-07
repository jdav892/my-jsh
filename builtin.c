#include "jsh.h"


char *clr[2] = {"clear", "null"};

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

int jsh_exit(char **args)
{
  (void)args;
  return 0;
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
