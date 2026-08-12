#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lexer.h"

#define MAX_TOKENS 100

static void add_token(Token **tokens, int *count,
                      TokenType type, const char *value)
{
    if (*count >= MAX_TOKENS - 1)
        return;

    tokens[*count] = create_token(type, value);
    (*count)++;
}

Token **lex_line(const char *input, int *token_count)
{
    Token **tokens = malloc(sizeof(Token *) * MAX_TOKENS);

    if (tokens == NULL)
        return NULL;

    int count = 0;
    int i = 0;

    while (input[i] != '\0')
    {
        /* Skip spaces */
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        /* Pipe */
        if (input[i] == '|')
        {
            add_token(tokens, &count, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        /* Input redirection */
        if (input[i] == '<')
        {
            add_token(tokens, &count, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        /* Output redirection */
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                add_token(tokens, &count, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                add_token(tokens, &count, TOKEN_OUTPUT, ">");
                i++;
            }

            continue;
        }

        /* Error output redirection */
        if (input[i] == '2' && input[i + 1] == '>')
        {
            add_token(tokens, &count, TOKEN_ERROR_OUTPUT, "2>");
            i += 2;
            continue;
        }

        /* Background */
        if (input[i] == '&')
        {
            add_token(tokens, &count, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        /* Word */
        char buffer[1024];
        int j = 0;

        while (input[i] != '\0' &&
               !isspace((unsigned char)input[i]) &&
               input[i] != '|' &&
               input[i] != '<' &&
               input[i] != '>' &&
               input[i] != '&')
        {
            if (j < (int)sizeof(buffer) - 1)
                buffer[j++] = input[i];

            i++;
        }

        buffer[j] = '\0';

        if (j > 0)
            add_token(tokens, &count, TOKEN_WORD, buffer);
    }

    /* End of input */
    tokens[count] = create_token(TOKEN_EOF, NULL);
    count++;

    *token_count = count;

    return tokens;
}

void free_tokens(Token **tokens, int token_count)
{
    if (tokens == NULL)
        return;

    for (int i = 0; i < token_count; i++)
        free_token(tokens[i]);

    free(tokens);
}

