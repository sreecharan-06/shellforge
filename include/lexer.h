#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#define MAX_TOKENS 100

typedef struct {
    Token *tokens[MAX_TOKENS];
    int count;
} token_list_t;

void lexer(const char *line, token_list_t *tokens);
void free_tokens(token_list_t *tokens);

#endif
