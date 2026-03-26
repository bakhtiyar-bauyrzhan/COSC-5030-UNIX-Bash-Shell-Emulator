#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glob.h>
#include <fcntl.h>

void print_prompt()
{
    printf("my_shell> ");
    fflush(stdout);
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

char *redirection_handler(char *line, char *cpy_line)
{
    char *dst = cpy_line;

    int single_quote = 0;
    int double_quote = 0;

    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == '\'')
            single_quote = !single_quote;
        else if (line[i] == '"')
            double_quote = !double_quote;

        if (!single_quote && !double_quote)
        {
            if (line[i] == '2' && line[i + 1] == '>')
            {
                if (line[i + 2] == '&' && (line[i + 3] == '1'))
                {
                    *dst++ = ' ';
                    *dst++ = line[i];
                    *dst++ = line[++i];
                    *dst++ = line[++i];
                    *dst++ = line[++i];
                    *dst++ = ' ';
                    continue;
                }
                *dst++ = ' ';
                *dst++ = line[i];
                *dst++ = line[++i];
                *dst++ = ' ';
                continue;
            }
            if (line[i] == '>' || line[i] == '<')
            {
                *dst++ = ' ';
                *dst++ = line[i];
                if (line[i + 1] == '>')
                {
                    *dst++ = line[++i];
                }
                *dst++ = ' ';
                continue;
            }
        }
        *dst++ = line[i];
    }
    *dst = '\0';
    return cpy_line;
}

char **parse_line(char *line)
{
    int redirect_count = 0;
    for (int i = 0; line[i] != '\0'; i++)
    {
        if (line[i] == '>' || line[i] == '<')
        {
            redirect_count++;
        }
    }
    size_t line_redirect = strlen(line) + (redirect_count * 2) + 1;
    char *cpy_line = calloc(line_redirect, sizeof(char));
    if (cpy_line == NULL)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    char *src = cpy_line;
    char *dst = cpy_line;
    int single_quote = 0;
    int double_quote = 0;
    int backslash = 0;

    redirection_handler(line, cpy_line);

    // Process the line character by character
    while (*src != '\0')
    {
        // If we encounter a newline, src reached the end of the line, so we can stop processing
        if (*src == '\n')
        {
            // Replace newline with null terminator and break
            while (*dst != '\n')
            {
                *dst++ = '\0';
            }
            *dst = '\0';
            break;
        }
        if ((*src == ' ' || *src == '\t'))
        {
            // If we are in quotes, just copy the space/tab
            if (single_quote || double_quote)
            {
                *dst++ = *src++;
                continue;
            }
            // If we are not in quotes, replace the space/tab with a null terminator to split tokens
            else
            {
                while (*src == ' ' || *src == '\t')
                {
                    src++;
                }
                *dst++ = '\0';
                continue;
            }
        }
        else
        {
            // If we see a single quote, toggle the single_quote flag
            if (*src == '\'')
            {
                single_quote = !single_quote;
                //*dst++ = *src++;
                src++;
                continue;
            }
            // If we see a double quote, toggle the double_quote flag
            else if (*src == '"')
            {
                double_quote = !double_quote;
                //*dst++ = *src++;
                src++;
                continue;
            }

            // If we see a backslash and we are in quotes, set the backslash flag
            if (*src == '\\' && (single_quote || double_quote))
            {
                backslash = 1;
                src++;
                continue;
            }
            // If we see a backslash and we are not in quotes, just skip it
            // else if (*src == '\\' && (!single_quote && !double_quote))
            // {
            //     src++;
            //     continue;
            // }

            // If we see a backslash and the backslash flag is set, copy the next character as is
            if (backslash)
            {
                // if the next character is 'n', combine the backslash and 'n' into a newline character
                if (*src == 'n' || *src == 't')
                {
                    if (*src == 'n')
                    {
                        *dst++ = '\n';
                    }
                    else if (*src == 't')
                    {
                        *dst++ = '\t';
                    }
                    src++;
                    backslash = 0;
                    continue;
                }
                // Otherwise, just copy the backslash and the next character
                *dst++ = '\\';
                *dst++ = *src++;
                backslash = 0;
                continue;
            }

            // Otherwise, just copy the character
            *dst++ = *src++;
        }
    }
    // Worst case: every other character is a space, plus one for the command and one for the NULL terminator
    int bufsize = (strlen(line) / 2) + 2;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));

    // Split the processed line into tokens based on null terminators
    for (int i = 0; position < bufsize;)
    {
        // Get the length of the next token
        int token_length = strlen(cpy_line + i);
        // No more tokens
        if (token_length == 0) {
            break;
        }
        // Duplicate the token string and store it in the tokens array
        tokens[position++] = strdup((cpy_line + i));
        // Move to the start of the next token (current position + token length + 1 for the null terminator)
        i += token_length + 1;
    }

    // Null-terminate the tokens array
    tokens[position] = NULL;

    return tokens;
}

int is_builtin(char *command)
{
    return (strcmp(command, "exit") == 0 ||
            strcmp(command, "cd") == 0 ||
            strcmp(command, "pwd") == 0);
}

int execute_builtin(char *command, char **args)
{
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);
    int saved_stderr = dup(STDERR_FILENO);

    char *in_file = NULL;
    char *out_file = NULL;
    char *err_file = NULL;
    int append_mode = 0;
    int j = 0;

    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "<") == 0)
        {
            in_file = args[++i];
        }
        else if (strcmp(args[i], ">") == 0)
        {
            out_file = args[++i];
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            out_file = args[++i];
            append_mode = 1;
        }
        else if (strcmp(args[i], "2>") == 0)
        {
            err_file = args[++i];
        }
        else
        {
            args[j++] = args[i];
        }
    }
    args[j] = NULL;

    if (in_file != NULL)
    {
        int fd_in = open(in_file, O_RDONLY);
        if (fd_in >= 0)
        {
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
    }
    if (out_file != NULL)
    {
        int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
        int fd_out = open(out_file, flags, 0644);
        if (fd_out >= 0)
        {
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
    }
    if (err_file != NULL)
    {
        int fd_err = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd_err >= 0)
        {
            dup2(fd_err, STDERR_FILENO);
            close(fd_err);
        }
    }

    int ret = 0;

    if (strcmp(command, "exit") == 0)
    {
        printf("Goodbye!\n");
        exit(EXIT_SUCCESS);
    }
    // Get the current working directory and print it
    else if (strcmp(command, "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
            ret = 1;
        }
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        char *path = args[1];

        // If no path is provided, default to the HOME environment variable
        if (path == NULL)
        {
            path = getenv("HOME");
        }
        // Attempt to change the directory
        if (chdir(path) != 0)
        {
            ret = -1;
        }
        ret = 1;
    }

    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);

    close(saved_stdin);
    close(saved_stdout);
    close(saved_stderr);

    return ret;
}

int execute_external(char *command, char **args)
{
    char *in_file = NULL;
    char *out_file = NULL;
    char *err_file = NULL;
    int append_mode = 0;
    int j = 0;

    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "<") == 0)
        {
            in_file = args[++i];
        }
        else if (strcmp(args[i], ">") == 0)
        {
            out_file = args[++i];
        }
        else if (strcmp(args[i], ">>") == 0)
        {
            out_file = args[++i];
            append_mode = 1;
        }
        else if (strcmp(args[i], "2>") == 0)
        {
            err_file = args[++i];
        }
        else
        {
            args[j++] = args[i];
        }
    }
    args[j] = NULL;

    // Use glob to expand wildcards in the arguments
    glob_t glob_result;
    // Initialize the glob_result structure to zero
    memset(&glob_result, 0, sizeof(glob_result));

    // Process each argument with glob, appending results to glob_result
    for (int i = 0; args[i] != NULL; i++)
    {
        // Use GLOB_NOCHECK to keep the original string if no wildcard match is found
        int flags = GLOB_NOCHECK;
        if (i > 0)
        {
            // For subsequent arguments, we want to append to the existing glob_result
            flags |= GLOB_APPEND;
        }
        // Expand the argument with glob
        int result = glob(args[i], flags, NULL, &glob_result);

        if (result != 0 && result != GLOB_NOMATCH)
        {
            fprintf(stderr, "Glob error processing: %s\n", args[i]);
            // Clean up partial allocations
            if (i > 0)
                globfree(&glob_result);
            return -1;
        }
    }

    // Fork the process to execute the command in a child process
    pid_t pid = fork();

    // Child process
    if (pid == 0)
    {
        if (in_file != NULL)
        {
            int fd_in = open(in_file, O_RDONLY);
            if (fd_in < 0)
            {
                perror("Error opening input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (out_file != NULL)
        {
            int flags = O_WRONLY | O_CREAT;
            flags |= (append_mode) ? O_APPEND : O_TRUNC;

            int fd_out = open(out_file, flags, 0644);
            if (fd_out < 0)
            {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        if (err_file != NULL)
        {
            int fd_err = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_err < 0)
            {
                perror("Error opening error file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_err, STDERR_FILENO);
            close(fd_err);
        }

        if (execvp(glob_result.gl_pathv[0], glob_result.gl_pathv) == -1)
        {
            fprintf(stderr, "Error executing command: %s\n", strerror(errno));
            globfree(&glob_result);
            exit(EXIT_FAILURE);
        }
    }
    // Forking error
    else if (pid < 0)
    {
        fprintf(stderr, "Error forking: %s\n", strerror(errno));
    }
    // Parent process
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }

    globfree(&glob_result);

    return 1;
}

void free_args(char **args)
{
    if (args == NULL)
        return;

    // Free each dynamically allocated token string
    for (int i = 0; args[i] != NULL; i++)
        free(args[i]);

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