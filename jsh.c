#include <stdio.h>
#include <stdlib.h>

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

char **split_lines(char *line)
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
      tokens = realloc(tokens, buffsize * sizeofchar(char *));

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

void loop()
{
  char *line;
  char **args;
  int status = 1;

  do {
    printf("> ");
    line = read_line();
    flag = 0;
    args = split_lines(line);
    status = dash_launch(args);
    free(line);
    free(args);
  } while (status);
}

int main()
{
  return 0;

}
