#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>

void print_prompt()
{
    printf("my_shell> ");
    fflush(stdout); // This will block until input is received or EOF is encountered
}

char *read_line(char *line)
{
    size_t len = 0;
    ssize_t char_read_count;

    // char_read_count returns the number of characters read, or -1 on EOF/error
    char_read_count = getline(&line, &len, stdin);

    if (char_read_count == -1)
    {
        return NULL; // EOF or error
                     // if (errno == EINTR)
                     // {
                     //     // CTRL+C was pressed; clear error and restart the loop
                     //     // No need unitl Phase 5
                     //     printf("\nCatched CTRL+C. Restarting LOOP\n");
                     //     clearerr(stdin);
                     // }
                     // else
                     // {
                     //     // This was an actual CTRL+D or a real error
                     //     // No need unitl Phase 5
                     //     printf("\nCatched CTRL+D. Sending EOF\n");
                     //     exit(EXIT_SUCCESS);
                     // }
    }
    return line;
}

char **parse_line(char *line)
{
    char *src = line;
    char *dst = line;
    int single_quote = 0;
    int double_quote = 0;
    while (*src != '\0')
    {
        if (*src == '\n')
        {
            // Replace newline with null terminator and break
            *dst = '\0';
            break;
        }
        if ((*src == ' ' || *src == '\t'))
        {
            if (single_quote || double_quote)
            {
                // If we are in quotes, just copy the space/tab
                *dst++ = *src++;
                continue;
            }
            else
            {
                // Skip over multiple spaces/tabs
                while (*src == ' ' || *src == '\t')
                    src++;
                // If we are not at the end, add a single space
                if (*src != '\0')
                    *dst++ = ' ';
            }
        }
        else
        {
            // If we see a single quote, toggle the single_quote flag
            if (*src == '\'')
                single_quote = !single_quote;
            // If we see a double quote, toggle the double_quote flag
            else if (*src == '"')
                double_quote = !double_quote;
            if (*src == '\\' && (single_quote || double_quote))
            {
                if (*(src+1) == 'n') {
                        *dst++ = '\n';
                        src = src + 2;
                        continue;
                }
                else {
                    *dst++ = *src++;
                    continue;
                }
            }
            else if (*src == '\\' && (!single_quote && !double_quote))
            {
                src++;
                continue;
            }
            *dst++ = *src++;
        }
    }
    char *cpy_line = strdup(line);
    int bufsize = 64;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token = strtok(cpy_line, " \t\n'\"");
    while (token != NULL)
    {
        tokens[position++] = strdup(token);

        // If we exceed the buffer, reallocate more memory
        if (position >= bufsize)
        {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char *));
        }

        // Next token
        token = strtok(NULL, " \t\n'\"");
    }

    tokens[position] = NULL;
    free(cpy_line);
    return tokens;
}

int is_builtin(char *command)
{
    return (strcmp(command, "exit") == 0 ||
            strcmp(command, "cd") == 0 ||
            strcmp(command, "pwd") == 0);
            //strcmp(command, "echo") == 0;
}

int execute_builtin(char *command, char **args)
{
    if (strcmp(command, "exit") == 0)
    {
        printf("Goodbye!\n");
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(command, "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
            return 1;
        }
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        char *path = args[1];
        if (path == NULL)
        {
            path = getenv("HOME");
        }
        if (chdir(path) != 0)
        {
            return -1;
        }
        return 1; // Built-in handled
    }
    // else if (strcmp(args[0], "echo") == 0)
    // {
    //     for (int i = 1; args[i] != NULL; i++)
    //     {
    //         printf("%s", args[i]);
    //         printf("\n\n\n");
    //         printf("%s", args[i][0]);
    //         printf("\n\n\n");
    //     }
    //     printf("\n");
    //     return 1;
    // }
    return 0;
}

int execute_external(char *command, char **args)
{
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // 1. Expand the arguments
    for (int i = 0; args[i] != NULL; i++)
    {
        int flags = GLOB_NOCHECK; // Keep original string if no wildcard match
        if (i > 0)
        {
            flags |= GLOB_APPEND; // Append to the array for subsequent args
        }
        int result = glob(args[i], flags, NULL, &glob_result);
        if (result != 0 && result != GLOB_NOMATCH) {
            fprintf(stderr, "Glob error processing: %s\n", args[i]);
            if (i > 0) globfree(&glob_result); // Clean up partial allocations
            return -1;
        }
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(glob_result.gl_pathv[0], glob_result.gl_pathv) == -1)
        {
            fprintf(stderr, "Error executing command: %s\n", strerror(errno));
            globfree(&glob_result);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0)
    {
        // Forking error
        fprintf(stderr, "Error forking: %s\n", strerror(errno));
    }
    else
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }

    globfree(&glob_result);
    
    return 1;
}

void free_args(char **args)
{
    if (args == NULL) return;

    // Free each dynamically allocated token string
    for (int i = 0; args[i] != NULL; i++) free(args[i]);

    // Free the array of pointers itself
    free(args);
}




#ifdef PHASE_5_SIGNALS
No need unitl Phase 5 void handle_sigint(int sig)
{
    printf("\nCatched CTRL+C. Exitting\n");
    fflush(stdout);
}
    // Register the handler for SIGINT (CTRL+C)
    signal(SIGINT, handle_sigint);
#endif