#include "jsh.h"

char *get_history_file_path()
{
  static char file_path[128];
  strncpy(file_path, getenv("HOME"), sizeof(file_path) - 1);
  file_path[sizeof(file_path) - 1] = '\0';
  strcat(file_path, "/.jsh_history");
  return file_path;
}

int history_line_count()
{
  FILE *fp = fopen(get_history_file_path(), "r");
  int c;
  int numberOfLines = 1;
  do
  {
    c = getc(fp);
    if(c == '\n')
    {
      numberOfLines++;
    }
  }while(c != EOF);
  return numberOfLines;
}
