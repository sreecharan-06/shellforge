#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

void command_init(Command *cmd)
{
    cmd->input = NULL;
    cmd->output = NULL;
    cmd->append = 0;
    cmd->background = 0;

    for (int i = 0; i < MAX_ARGS; i++)
        cmd->argv[i] = NULL;
}

static void pipeline_init(Pipeline *pipeline)
{
    pipeline->command_count = 0;

    for (int i = 0; i < MAX_COMMANDS; i++)
        command_init(&pipeline->commands[i]);
}

static int add_argument(Command *cmd, const char *value)
{
    int i = 0;

    while (i < MAX_ARGS - 1 && cmd->argv[i] != NULL)
        i++;

    if (i >= MAX_ARGS - 1)
    {
        fprintf(stderr, "Error: too many arguments\n");
        return 0;
    }

    cmd->argv[i] = strdup(value);

    if (cmd->argv[i] == NULL)
    {
        perror("strdup");
        return 0;
    }

    return 1;
}

int parse(Token **tokens, Pipeline *pipeline)
{
    pipeline_init(pipeline);

    if (tokens == NULL || tokens[0] == NULL)
        return 0;

    pipeline->command_count = 1;

    Command *current = &pipeline->commands[0];

    for (int i = 0; tokens[i] != NULL; i++)
    {
        Token *token = tokens[i];

        switch (token->type)
        {
            case TOKEN_WORD:

                if (!add_argument(current, token->value))
                    return 0;

                break;

            case TOKEN_INPUT:

                if (tokens[i + 1] == NULL ||
                    tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after <\n");
                    return 0;
                }

                current->input = strdup(tokens[++i]->value);

                if (current->input == NULL)
                    return 0;

                break;

            case TOKEN_OUTPUT:

                if (tokens[i + 1] == NULL ||
                    tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after >\n");
                    return 0;
                }

                current->output = strdup(tokens[++i]->value);
                current->append = 0;

                if (current->output == NULL)
                    return 0;

                break;

            case TOKEN_APPEND:

                if (tokens[i + 1] == NULL ||
                    tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after >>\n");
                    return 0;
                }

                current->output = strdup(tokens[++i]->value);
                current->append = 1;

                if (current->output == NULL)
                    return 0;

                break;

            case TOKEN_ERROR_OUTPUT:

                if (tokens[i + 1] == NULL ||
                    tokens[i + 1]->type != TOKEN_WORD)
                {
                    fprintf(stderr,
                            "Error: filename expected after 2>\n");
                    return 0;
                }

                /*
                 * This milestone's Command structure does not
                 * currently contain a separate error-output field.
                 * Treat it as output for parsing/display purposes.
                 */
                current->output = strdup(tokens[++i]->value);
                current->append = 0;

                if (current->output == NULL)
                    return 0;

                break;

            case TOKEN_PIPE:

                if (current->argv[0] == NULL)
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

    if (current->argv[0] == NULL)
    {
        fprintf(stderr, "Error: empty command\n");
        return 0;
    }

    return 1;
}

void pipeline_print(Pipeline *pipeline)
{
    printf("\n");
    printf("========== PIPELINE ==========\n");

    for (int i = 0; i < pipeline->command_count; i++)
    {
        Command *cmd = &pipeline->commands[i];

        printf("\nCommand %d\n", i + 1);
        printf("--------------------------------\n");

        printf("Arguments\n");

        for (int j = 0;
             j < MAX_ARGS && cmd->argv[j] != NULL;
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

void pipeline_free(Pipeline *pipeline)
{
    for (int i = 0; i < pipeline->command_count; i++)
    {
        Command *cmd = &pipeline->commands[i];

        for (int j = 0; j < MAX_ARGS; j++)
        {
            free(cmd->argv[j]);
            cmd->argv[j] = NULL;
        }

        free(cmd->input);
        free(cmd->output);

        cmd->input = NULL;
        cmd->output = NULL;
    }

    pipeline->command_count = 0;
}
