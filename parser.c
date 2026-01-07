#include "jsh.h"

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
