#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/token.h"

Token *create_token(TokenType type, const char *value)
{
    Token *token = malloc(sizeof(Token));

    if (token == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    token->type = type;

    if (value != NULL)
    {
        token->value = malloc(strlen(value) + 1);

        if (token->value == NULL)
        {
            perror("malloc");
            free(token);
            exit(EXIT_FAILURE);
        }

        strcpy(token->value, value);
    }
    else
    {
        token->value = NULL;
    }

    return token;
}

void free_token(Token *token)
{
    if (token == NULL)
        return;

    free(token->value);
    free(token);
}
