#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>

void print_prompt();
char *read_line(char *line);
char **parse_line(char *line);
int is_builtin(char *command);
int execute_builtin(char *command, char **args);
int execute_external(char *command, char **args);
void free_args(char **args);

#endif