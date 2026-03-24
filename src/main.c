#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>
#include "shell.h"

# Cole was here

int main()
{
    char *raw_line = NULL;
    char **args = NULL;

    while (1)
    {
        // Show the prompt
        print_prompt();
        // Read a line of input
        raw_line = read_line(raw_line);

        // If EOF or error, clear the error state and restart the loop
        if (raw_line == NULL)
        {
            clearerr(stdin);
            printf("\nCatched EOF or error. Restarting loop\n");
            continue;
        }

        // If the user just pressed Enter, show the prompt again
        if (raw_line[0] == '\n')
        {
            free(raw_line);
            raw_line = NULL;
            continue;
        }

        // Parse the line into arguments
        args = parse_line(raw_line);

        // If no command was entered, show the prompt again
        if (args[0] != NULL) {
            if (is_builtin(args[0]))
                execute_builtin(args[0], args);
            else execute_external(args[0], args);
        }
        
        free_args(args);
        free(raw_line);
        args = NULL;
        raw_line = NULL;
    }
    return 0;
}
