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
