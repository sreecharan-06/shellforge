#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

static void add_token(token_list_t *tokens, TokenType type, const char *value)
{
    if (tokens->count >= MAX_TOKENS - 1)
        return;

    tokens->tokens[tokens->count] = create_token(type, value);
    tokens->count++;
}

void lexer(const char *line, token_list_t *tokens)
{
    tokens->count = 0;
    for (int i = 0; i < MAX_TOKENS; i++)
    {
        tokens->tokens[i] = NULL;
    }

    int i = 0;

    while (line[i] != '\0')
    {
        /* Skip spaces */
        if (isspace((unsigned char)line[i]))
        {
            i++;
            continue;
        }

        /* Pipe */
        if (line[i] == '|')
        {
            add_token(tokens, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Input redirection */
        if (line[i] == '<')
        {
            add_token(tokens, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        /* Output redirection */
        if (line[i] == '>')
        {
            if (line[i + 1] == '>')
            {
                add_token(tokens, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                add_token(tokens, TOKEN_OUTPUT, ">");
                i++;
            }

            continue;
        }

        /* Error output redirection */
        if (line[i] == '2' && line[i + 1] == '>')
        {
            add_token(tokens, TOKEN_ERROR_OUTPUT, "2>");
            i += 2;
            continue;
        }

        /* Background */
        if (line[i] == '&')
        {
            add_token(tokens, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        /* Word */
        char buffer[1024];
        int j = 0;

        while (line[i] != '\0' &&
               !isspace((unsigned char)line[i]) &&
               line[i] != '|' &&
               line[i] != '<' &&
               line[i] != '>' &&
               line[i] != '&')
        {
            if (j < (int)sizeof(buffer) - 1)
                buffer[j++] = line[i];

            i++;
        }

        buffer[j] = '\0';

        if (j > 0)
            add_token(tokens, TOKEN_WORD, buffer);
    }

    /* End of input */
    tokens->tokens[tokens->count] = create_token(TOKEN_EOF, NULL);
    tokens->count++;
}

void free_tokens(token_list_t *tokens)
{
    if (tokens == NULL)
        return;

    for (int i = 0; i < tokens->count; i++)
    {
        free_token(tokens->tokens[i]);
        tokens->tokens[i] = NULL;
    }
    tokens->count = 0;
}
