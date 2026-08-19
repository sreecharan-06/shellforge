#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

void command_init(command_t *cmd)
{
    cmd->argc = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = 0;
    cmd->background = 0;

    for (int i = 0; i < MAX_ARGS; i++)
        cmd->argv[i] = NULL;
}

static void pipeline_init(pipeline_t *pipeline)
{
    pipeline->command_count = 0;

    for (int i = 0; i < MAX_COMMANDS; i++)
        command_init(&pipeline->commands[i]);
}

static int add_argument(command_t *cmd, const char *value)
{
    if (cmd->argc >= MAX_ARGS - 1)
    {
        fprintf(stderr, "Error: too many arguments\n");
        return 0;
    }

    cmd->argv[cmd->argc] = strdup(value);

    if (cmd->argv[cmd->argc] == NULL)
    {
        perror("strdup");
        return 0;
    }

    cmd->argc++;
    cmd->argv[cmd->argc] = NULL;

    return 1;
}

int parser(token_list_t *tokens, pipeline_t *pipeline)
{
    pipeline_init(pipeline);

    if (tokens == NULL || tokens->count == 0)
        return 0;

    pipeline->command_count = 1;

    command_t *current = &pipeline->commands[0];

    for (int i = 0; i < tokens->count; i++)
    {
        Token *token = tokens->tokens[i];
        if (token == NULL)
            continue;

        switch (token->type)
        {
            case TOKEN_WORD:

                if (!add_argument(current, token->value))
                    return 0;

                break;

            case TOKEN_INPUT:

                if (i + 1 >= tokens->count ||
                    tokens->tokens[i + 1] == NULL ||
                    tokens->tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after <\n");
                    return 0;
                }

                current->input = strdup(tokens->tokens[++i]->value);

                if (current->input == NULL)
                    return 0;

                break;

            case TOKEN_OUTPUT:

                if (i + 1 >= tokens->count ||
                    tokens->tokens[i + 1] == NULL ||
                    tokens->tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after >\n");
                    return 0;
                }

                current->output = strdup(tokens->tokens[++i]->value);
                current->append = 0;

                if (current->output == NULL)
                    return 0;

                break;

            case TOKEN_APPEND:

                if (i + 1 >= tokens->count ||
                    tokens->tokens[i + 1] == NULL ||
                    tokens->tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after >>\n");
                    return 0;
                }

                current->output = strdup(tokens->tokens[++i]->value);
                current->append = 1;

                if (current->output == NULL)
                    return 0;

                break;

            case TOKEN_ERROR_OUTPUT:

                if (i + 1 >= tokens->count ||
                    tokens->tokens[i + 1] == NULL ||
                    tokens->tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after 2>\n");
                    return 0;
                }

                current->output = strdup(tokens->tokens[++i]->value);
                current->append = 0;

                if (current->output == NULL)
                    return 0;

                break;

            case TOKEN_PIPE:

                if (current->argc == 0)
                {
                    fprintf(stderr,
                            "Error: empty command before pipe\n");
                    return 0;
                }

                if (pipeline->command_count >= MAX_COMMANDS)
                {
                    fprintf(stderr,
                            "Error: too many commands in pipeline\n");
                    return 0;
                }

                current =
                    &pipeline->commands[pipeline->command_count];

                command_init(current);

                pipeline->command_count++;

                break;

            case TOKEN_BACKGROUND:

                current->background = 1;
                break;

            case TOKEN_EOF:

                goto finished;

            default:

                fprintf(stderr, "Error: unknown token\n");
                return 0;
        }
    }

finished:

    if (current->argc == 0)
    {
        fprintf(stderr, "Error: empty command\n");
        return 0;
    }

    return 1;
}

void pipeline_print(pipeline_t *pipeline)
{
    printf("\n");
    printf("========== PIPELINE ==========\n");

    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        printf("\nCommand %d\n", i + 1);
        printf("--------------------------------\n");

        printf("Arguments\n");

        for (int j = 0;
             j < cmd->argc;
             j++)
        {
            printf("argv[%d] = %s\n", j, cmd->argv[j]);
        }

        printf("Input      : %s\n",
               cmd->input ? cmd->input : "None");

        printf("Output     : %s\n",
               cmd->output ? cmd->output : "None");

        printf("Append     : %s\n",
               cmd->append ? "Yes" : "No");

        printf("Background : %s\n",
               cmd->background ? "Yes" : "No");
    }

    printf("================================\n");
}

void pipeline_free(pipeline_t *pipeline)
{
    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        for (int j = 0; j < cmd->argc; j++)
        {
            free(cmd->argv[j]);
            cmd->argv[j] = NULL;
        }

        free(cmd->input);
        free(cmd->output);

        cmd->input = NULL;
        cmd->output = NULL;
        cmd->argc = 0;
    }

    pipeline->command_count = 0;
}
