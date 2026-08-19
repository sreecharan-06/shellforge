#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "expand.h"

static char *expand_string(const char *str)
{
    size_t capacity = 128;
    size_t length = 0;

    char *result = malloc(capacity);

    if (result == NULL)
        return NULL;

    result[0] = '\0';

    for (size_t i = 0; str[i] != '\0'; i++)
    {
        /* Variable expansion */
        if (str[i] == '$')
        {
            i++;

            if (str[i] == '\0')
            {
                if (length + 2 >= capacity)
                {
                    capacity *= 2;
                    char *temp = realloc(result, capacity);

                    if (temp == NULL)
                    {
                        free(result);
                        return NULL;
                    }
                    result = temp;
                }

                result[length++] = '$';
                result[length] = '\0';
                break;
            }

            char variable[128];
            int j = 0;
            int is_braced = 0;

            /* ${VARIABLE} */
            if (str[i] == '{')
            {
                is_braced = 1;
                i++;

                while (str[i] != '\0' &&
                       str[i] != '}' &&
                       j < 127)
                {
                    variable[j++] = str[i++];
                }

                variable[j] = '\0';

                if (str[i] == '}')
                {
                    /* Move past } */
                }
                else
                {
                    i--;
                }
            }
            else
            {
                /* $VARIABLE */
                while (str[i] != '\0' &&
                       (isalnum((unsigned char)str[i]) ||
                        str[i] == '_') &&
                       j < 127)
                {
                    variable[j++] = str[i++];
                }

                variable[j] = '\0';

                i--;
            }

            if (j > 0)
            {
                const char *value = getenv(variable);

                if (value != NULL)
                {
                    size_t value_len = strlen(value);

                    while (length + value_len + 1 >= capacity)
                    {
                        capacity *= 2;

                        char *temp = realloc(result, capacity);

                        if (temp == NULL)
                        {
                            free(result);
                            return NULL;
                        }

                        result = temp;
                    }

                    strcpy(result + length, value);
                    length += value_len;
                }
            }
            else
            {
                // No valid variable name followed '$'
                // If it was '${}', we can choose to ignore or keep it.
                // If it was just '$' followed by space/etc (like '$ '), we keep the '$'
                if (!is_braced)
                {
                    if (length + 2 >= capacity)
                    {
                        capacity *= 2;
                        char *temp = realloc(result, capacity);

                        if (temp == NULL)
                        {
                            free(result);
                            return NULL;
                        }
                        result = temp;
                    }
                    result[length++] = '$';
                    result[length] = '\0';
                }
            }
        }
        else
        {
            /* Normal character */
            if (length + 2 >= capacity)
            {
                capacity *= 2;

                char *temp = realloc(result, capacity);

                if (temp == NULL)
                {
                    free(result);
                    return NULL;
                }

                result = temp;
            }

            result[length++] = str[i];
            result[length] = '\0';
        }
    }

    return result;
}

void expand_variables(Pipeline *pipeline)
{
    if (pipeline == NULL)
        return;

    for (int i = 0;
         i < pipeline->command_count;
         i++)
    {
        Command *cmd = &pipeline->commands[i];

        /* Expand command arguments */
        for (int j = 0;
             j < MAX_ARGS && cmd->argv[j] != NULL;
             j++)
        {
            char *expanded =
                expand_string(cmd->argv[j]);

            if (expanded != NULL)
            {
                free(cmd->argv[j]);
                cmd->argv[j] = expanded;
            }
        }

        /* Expand input filename */
        if (cmd->input != NULL)
        {
            char *expanded =
                expand_string(cmd->input);

            if (expanded != NULL)
            {
                free(cmd->input);
                cmd->input = expanded;
            }
        }

        /* Expand output filename */
        if (cmd->output != NULL)
        {
            char *expanded =
                expand_string(cmd->output);

            if (expanded != NULL)
            {
                free(cmd->output);
                cmd->output = expanded;
            }
        }
    }
}
