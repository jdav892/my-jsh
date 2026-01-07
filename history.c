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
  
  if(!fp) return 1;

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

void history_input(char **args, char *d)
{
  FILE *history_file = fopen(get_history_file_path(), "a+");
  int j = 0;
  fprintf(history_file, "%d, ", history_line_count());
  while(args[j] != NULL)
  {
    if(j > 0)
      fputs(d, history_file);
    fputs(args[j], history_file);
    j++;
  }
  fputs("\n", history_file);
  fclose(history_file);
}

void pipe_history_input(char * line)
{
  FILE *history_file = fopen(get_history_file_path(), "a+");
  fprintf(history_file, "%d, %s\n", history_line_count(), line);
  fclose(history_file);
}
