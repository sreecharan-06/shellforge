#ifndef TOKEN_H
#define TOKEN_H

typedef enum
{
    TOKEN_WORD,
    TOKEN_PIPE,
    TOKEN_INPUT,
    TOKEN_OUTPUT,
    TOKEN_APPEND,
    TOKEN_ERROR_OUTPUT,
    TOKEN_BACKGROUND,
    TOKEN_EOF
} TokenType;

typedef struct Token
{
    TokenType type;
    char *value;
} Token;

Token *create_token(TokenType type, const char *value);
void free_token(Token *token);

#endif
