#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "lexer.h"
#include "parser.h"
#include "expand.h"

static const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:
            return "WORD";

        case TOKEN_PIPE:
            return "PIPE";

        case TOKEN_INPUT:
            return "INPUT";

        case TOKEN_OUTPUT:
            return "OUTPUT";

        case TOKEN_APPEND:
            return "APPEND";

        case TOKEN_ERROR_OUTPUT:
            return "ERROR_OUTPUT";

        case TOKEN_BACKGROUND:
            return "BACKGROUND";

        case TOKEN_EOF:
            return "EOF";

        default:
            return "UNKNOWN";
    }
}

static int is_builtin(const char *cmd)
{
    if (cmd == NULL)
        return 0;

    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "pwd") == 0 ||
            strcmp(cmd, "echo") == 0 ||
            strcmp(cmd, "history") == 0 ||
            strcmp(cmd, "exit") == 0);
}

static int execute_builtin(Command *cmd)
{
    if (cmd == NULL || cmd->argv[0] == NULL)
        return 0;

    if (strcmp(cmd->argv[0], "cd") == 0)
    {
        char *dir = cmd->argv[1];

        if (dir == NULL)
        {
            dir = getenv("HOME");
            if (dir == NULL)
            {
                fprintf(stderr, "cd: HOME not set\n");
                return 1;
            }
        }

        if (chdir(dir) != 0)
        {
            perror("cd");
            return 1;
        }

        return 0;
    }
    else if (strcmp(cmd->argv[0], "pwd") == 0)
    {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
            return 0;
        }
        else
        {
            perror("pwd");
            return 1;
        }
    }
    else if (strcmp(cmd->argv[0], "echo") == 0)
    {
        for (int i = 1; cmd->argv[i] != NULL; i++)
        {
            printf("%s", cmd->argv[i]);
            if (cmd->argv[i + 1] != NULL)
            {
                printf(" ");
            }
        }
        printf("\n");
        return 0;
    }
    else if (strcmp(cmd->argv[0], "history") == 0)
    {
        HIST_ENTRY **entries = history_list();

        if (entries != NULL)
        {
            for (int i = 0; entries[i] != NULL; i++)
            {
                printf("%d  %s\n", i + 1, entries[i]->line);
            }
        }

        return 0;
    }
    else if (strcmp(cmd->argv[0], "exit") == 0)
    {
        return 0;
    }

    return 1;
}

static void execute_pipeline(Pipeline *pipeline)
{
    int num_cmds = pipeline->command_count;
    if (num_cmds == 0)
        return;

    int pipes[MAX_COMMANDS - 1][2];

    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            return;
        }
    }

    pid_t pids[MAX_COMMANDS];

    for (int i = 0; i < num_cmds; i++)
    {
        Command *cmd = &pipeline->commands[i];

        fflush(stdout);
        fflush(stderr);
        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("fork");
            return;
        }

        if (pids[i] == 0)
        {
            // Child process

            // Pipe input
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0)
                {
                    perror("dup2 pipe input");
                    exit(EXIT_FAILURE);
                }
            }

            // Pipe output
            if (i < num_cmds - 1)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0)
                {
                    perror("dup2 pipe output");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipes in child
            for (int j = 0; j < num_cmds - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Redirect input
            if (cmd->input != NULL)
            {
                int in_fd = open(cmd->input, O_RDONLY);
                if (in_fd < 0)
                {
                    perror("open input file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(in_fd, STDIN_FILENO) < 0)
                {
                    perror("dup2 input file");
                    close(in_fd);
                    exit(EXIT_FAILURE);
                }
                close(in_fd);
            }

            // Redirect output
            if (cmd->output != NULL)
            {
                int flags = O_WRONLY | O_CREAT;
                if (cmd->append)
                    flags |= O_APPEND;
                else
                    flags |= O_TRUNC;

                int out_fd = open(cmd->output, flags, 0644);
                if (out_fd < 0)
                {
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(out_fd, STDOUT_FILENO) < 0)
                {
                    perror("dup2 output file");
                    close(out_fd);
                    exit(EXIT_FAILURE);
                }
                close(out_fd);
            }

            // Execute
            if (is_builtin(cmd->argv[0]))
            {
                execute_builtin(cmd);
                exit(EXIT_SUCCESS);
            }
            else
            {
                execvp(cmd->argv[0], cmd->argv);
                perror(cmd->argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Parent closes all pipes
    for (int i = 0; i < num_cmds - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children unless backgrounded
    int wait_for_pipeline = 1;
    if (pipeline->commands[num_cmds - 1].background)
    {
        wait_for_pipeline = 0;
        printf("[Process running in background]\n");
    }

    if (wait_for_pipeline)
    {
        for (int i = 0; i < num_cmds; i++)
        {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
}

int main(void)
{
    char *input;

    printf("ShellForge - Milestone 2\n");
    printf("Tokenizer and Lexer\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        input = readline("shellforge> ");

        if (input == NULL)
        {
            break;
        }

        if (strlen(input) == 0)
        {
            free(input);
            continue;
        }

        add_history(input);

        /*
         * Lexical analysis
         */
        int token_count = 0;
        Token **tokens = lex_line(input, &token_count);

        if (tokens == NULL)
        {
            fprintf(stderr, "Error: unable to tokenize input.\n");
            free(input);
            continue;
        }

        /*
         * Display tokens
         */
        printf("\nTokens:\n");
        for (int i = 0; i < token_count; i++)
        {
            if (tokens[i] == NULL)
            {
                continue;
            }

            printf("  %-16s : \"%s\"\n",
                   token_type_name(tokens[i]->type),
                   tokens[i]->value);

            if (tokens[i]->type == TOKEN_EOF)
            {
                break;
            }
        }

        /*
         * Parsing
         */
        Pipeline pipeline;

        if (parse(tokens, &pipeline))
        {
            /*
             * Variable expansion
             */
            expand_variables(&pipeline);

            /*
             * Display parsed and expanded pipeline
             */
            pipeline_print(&pipeline);

            /*
             * Built-in handling / command execution
             */
            if (pipeline.command_count == 1 && pipeline.commands[0].argv[0] != NULL)
            {
                if (strcmp(pipeline.commands[0].argv[0], "exit") == 0)
                {
                    pipeline_free(&pipeline);
                    free_tokens(tokens, token_count);
                    free(input);
                    break;
                }
                else if (strcmp(pipeline.commands[0].argv[0], "cd") == 0)
                {
                    execute_builtin(&pipeline.commands[0]);
                }
                else
                {
                    execute_pipeline(&pipeline);
                }
            }
            else if (pipeline.command_count > 0)
            {
                execute_pipeline(&pipeline);
            }

            /*
             * Free pipeline memory
             */
            pipeline_free(&pipeline);
        }

        /*
         * Free tokens and input
         */
        free_tokens(tokens, token_count);
        free(input);

        printf("\n");
    }

    printf("Exiting ShellForge.\n");

    return 0;
}
