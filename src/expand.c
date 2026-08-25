#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "expand.h"

static void expand_string(const char *src, char *dest, int dest_len)
{
    int src_idx = 0;
    int dest_idx = 0;

    while (src[src_idx] != '\0' && dest_idx < dest_len - 1)
    {
        if (src[src_idx] == '$')
        {
            src_idx++; // skip '$'

            // Check for ${VAR} syntax
            if (src[src_idx] == '{')
            {
                src_idx++; // skip '{'
                char var_name[256];
                size_t var_idx = 0;

                while (src[src_idx] != '\0' && src[src_idx] != '}')
                {
                    if (var_idx < sizeof(var_name) - 1)
                    {
                        var_name[var_idx++] = src[src_idx];
                    }
                    src_idx++;
                }
                var_name[var_idx] = '\0';

                if (src[src_idx] == '}')
                {
                    src_idx++; // skip '}'
                }

                // Lookup environment variable
                const char *val = getenv(var_name);
                if (val != NULL)
                {
                    while (*val != '\0' && dest_idx < dest_len - 1)
                    {
                        dest[dest_idx++] = *val++;
                    }
                }
            }
            else
            {
                // Alphanumeric/underscore name: $VAR
                char var_name[256];
                size_t var_idx = 0;

                // Read variable name
                while (src[src_idx] != '\0' && (isalnum((unsigned char)src[src_idx]) || src[src_idx] == '_'))
                {
                    if (var_idx < sizeof(var_name) - 1)
                    {
                        var_name[var_idx++] = src[src_idx];
                    }
                    src_idx++;
                }
                var_name[var_idx] = '\0';

                if (var_idx == 0)
                {
                    // If no variable name followed the '$', treat it as a literal '$'
                    dest[dest_idx++] = '$';
                }
                else
                {
                    const char *val = getenv(var_name);
                    if (val != NULL)
                    {
                        while (*val != '\0' && dest_idx < dest_len - 1)
                        {
                            dest[dest_idx++] = *val++;
                        }
                    }
                }
            }
        }
        else
        {
            dest[dest_idx++] = src[src_idx++];
        }
    }
    dest[dest_idx] = '\0';
}

void expand_variables(pipeline_t *pipeline)
{
    if (pipeline == NULL)
    {
        return;
    }

    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];
        
        // 1. Expand command arguments
        for (int j = 0; j < cmd->argc; j++)
        {
            char expanded[4096];
            expand_string(cmd->argv[j], expanded, sizeof(expanded));
            
            // Write expanded string back in place to the token text buffer.
            // Since cmd->argv[j] points to the token's text field (size 256),
            // copy at most 255 bytes.
            strncpy(cmd->argv[j], expanded, 255);
            cmd->argv[j][255] = '\0';
        }

        // 2. Expand input redirection file path
        if (strlen(cmd->input) > 0)
        {
            char expanded[256];
            expand_string(cmd->input, expanded, sizeof(expanded));
            strncpy(cmd->input, expanded, sizeof(cmd->input) - 1);
            cmd->input[sizeof(cmd->input) - 1] = '\0';
        }

        // 3. Expand output redirection file path
        if (strlen(cmd->output) > 0)
        {
            char expanded[256];
            expand_string(cmd->output, expanded, sizeof(expanded));
            strncpy(cmd->output, expanded, sizeof(cmd->output) - 1);
            cmd->output[sizeof(cmd->output) - 1] = '\0';
        }
    }
}
