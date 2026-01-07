#include "jsh.h"

// array of built in function pointers
int (*builtin_funcs[])(char **) = {&jsh_cd,      &jsh_help, &jsh_exit,
                                   &jsh_history, &jsh_grep, &args_length};

// string array of built in commands for strcmp() before invoking execvp()
char *builtin_str[] = {"cd", "help", "exit", "history", "grep", "sizeof"};

// return the size of built in array
char *clr[2] = { "clear", "null"};

int builtin_funcs_count() { return sizeof(builtin_str) / sizeof(char *); }


int jsh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "%sjsh: please enter a path to cd%s\n", YELLOW, RESET);
    return 1;
  } else {
    if (chdir(args[1]) < 0) {
      perror("jsh");
      return 1;
    }
  }
  return 1;
}

/*
 * Displays a description of builtin commands on - 'help'
 * */

int jsh_help(char **args) {
  if (args[0] != NULL && strcmp(args[0], "help") == 0) {
    fprintf(stderr,
            "\n~~~~~~~\n" BOLD "\njsh " RESET "is a basic unix terminal shell\n"
            "\nSupported Commands:\n1. cd\n2. exit\n3. help\n4. touch\n5. cat"
            "\n\n~~~~~~~\n\n");
  }
  return 1;
}

int jsh_exit(char **args) {
  (void)args;
  return 0;
}

int jsh_history(char **args) {
  FILE *fp = fopen(get_history_file_path(), "r");
  int ch, c, line_num = 1;
  char line[128];
  char prev_comm[128];
  if (!fp)
    fprintf(stderr, RED "jsh: file not found" RESET "\n");
  else {
    putchar('\n');
    while ((c = getc(fp)) != EOF) {
      putchar(c);
    }
  }

  printf("\n" INVERT
         "<0>: QUIT   <#line> Execute command <-1> clear history" RESET
         "\n\n: ");
  scanf("%d", &ch);
  getchar();
  fseek(fp, 0, SEEK_SET);
  if (isdigit(ch) != 0) {
    printf("Please enter a number\n");
  } else if (ch == 0) {
    fclose(fp);
    return 1;
  } else if (ch == -1) {
    fclose(fp);
    fp = fopen(get_history_file_path(), "w");
    fclose(fp);
    return jsh_execute(clr);
  } else {
    while ((fgets(line, 128, fp)) != NULL) {
      if (line_num == ch) {
        strcpy(prev_comm, &line[3]);
        int p = 0, flag = 0;
        fclose(fp);
        while (prev_comm[p] != '\0') {
          if (prev_comm[p] == '|') {
            flag = 1;
            break;
          }
          p++;
        }
        if (!flag) {
          args = split_line(prev_comm);
          return jsh_launch(args);
        } else {
          args = split_pipes(prev_comm);
          return jsh_pipe(args);
        }
      } else
        line_num++;
    }
  }
  return 1;
}

int jsh_grep(char **args) {
  FILE *fp = NULL;
  int flag = 0;
  char temp[512];
  int line_num = 1;
  if (args[0] != NULL && strcmp(args[0], "grep") == 0) {
    if (args[1] != NULL && args[2] != NULL) {
      // local struct variable
      struct stat path_stat;

      if (stat(args[2], &path_stat) == 0) {
        if (S_ISDIR(path_stat.st_mode)) {
          fprintf(stderr,
                  RED "jsh: grep: '%s' is a directory not a file" RESET "\n",
                  args[2]);
          return 1;
        }
      }
      fp = fopen(args[2], "r");

      if (fp == NULL) {
        switch (errno) {
        case ENOENT:
          fprintf(stderr, RED "jsh: grep: file '%s' does not exist" RESET "\n",
                  args[2]);
          break;

        case EACCES:
          fprintf(stderr,
                  RED "jsh: grep: permission denied for file '%s'" RESET "\n",
                  args[2]);
          break;

        case EISDIR:
          fprintf(stderr, RED "jsh: grep: '%s' is a directory" RESET "\n",
                  args[2]);
          break;

        default:
          fprintf(stderr, RED "jsh: grep: cannot open file '%s': %s" RESET "\n",
                  args[2], strerror(errno));
          break;
        }
        return 1;
      }

      while ((fgets(temp, 512, fp)) != NULL) {
        if (strstr(temp, args[1])) {
          printf("%d, %s", line_num, temp);
          flag = 1;
        }
        line_num++;
      }
      fclose(fp);
    } else {
      fprintf(stderr,
              RED "jsh: grep requires two parameters, " ITALICS
                  "PATTERN" RESET RED " and " RED ITALICS "FILE" RESET "\n");
    }
  }
  if (flag == 0)
    printf("No matches were found \n");
  return 1;
}

int args_length(char **args) {
  int i = 0;

  while (args[i] != NULL) {
    i++;
  }
  return i;
}
