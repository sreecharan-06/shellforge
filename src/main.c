#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "lexer.h"

static const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case TOKEN_WORD:
            return "WORD";
        case TOKEN_PIPE:
            return "PIPE";
        case TOKEN_INPUT:
            return "INPUT";
        case TOKEN_OUTPUT:
            return "OUTPUT";
        case TOKEN_APPEND:
            return "APPEND";
        case TOKEN_ERROR_OUTPUT:
            return "ERROR_OUTPUT";
        case TOKEN_BACKGROUND:
            return "BACKGROUND";
        case TOKEN_EOF:
            return "EOF";
        default:
            return "UNKNOWN";
    }
}

int main(void)
{
    char *input;

    printf("ShellForge - Milestone 2\n");
    printf("Tokenizer and Lexer\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        input = readline("shellforge> ");

        if (input == NULL)
        {
            break;
        }

        if (strlen(input) == 0)
        {
            free(input);
            continue;
        }

        add_history(input);

        if (strcmp(input, "history") == 0)
        {
            HIST_ENTRY **entries = history_list();

            if (entries != NULL)
            {
                for (int i = 0; entries[i] != NULL; i++)
                {
                    printf("%d  %s\n", i + 1, entries[i]->line);
                }
            }

            free(input);
            continue;
        }

        if (strcmp(input, "exit") == 0)
        {
            free(input);
            break;
        }

        int token_count = 0;

        Token **tokens = lex_line(input, &token_count);

        if (tokens == NULL)
        {
            fprintf(stderr, "Error: unable to tokenize input.\n");
            free(input);
            continue;
        }

        printf("\nTokens:\n");

        for (int i = 0; i < token_count; i++)
        {
            if (tokens[i] == NULL)
            {
                continue;
            }

            printf("  %-16s : \"%s\"\n",
                   token_type_name(tokens[i]->type),
                   tokens[i]->value);

            if (tokens[i]->type == TOKEN_EOF)
            {
                break;
            }
        }

        printf("\n");

        free_tokens(tokens, token_count);
        free(input);
    }

    printf("Exiting ShellForge.\n");

    return 0;
}
