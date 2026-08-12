#ifndef LEXER_H
#define LEXER_H

#include "token.h"

Token **lex_line(const char *input, int *token_count);
void free_tokens(Token **tokens, int token_count);

#endif
