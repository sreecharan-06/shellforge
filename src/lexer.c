#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

void lexer(const char *input, token_list_t *list)
{
    if (input == NULL || list == NULL)
    {
        return;
    }

    token_list_init(list);
    int i = 0;

    while (input[i] != '\0')
    {
        // Skip whitespace
        if (isspace((unsigned char)input[i]))
        {
            i++;
            continue;
        }

        // Delimiter: PIPE
        if (input[i] == '|')
        {
            token_add(list, TOKEN_PIPE, "|");
            i++;
            continue;
        }

        // Delimiter: INPUT redirection
        if (input[i] == '<')
        {
            token_add(list, TOKEN_INPUT, "<");
            i++;
            continue;
        }

        // Delimiter: OUTPUT or APPEND redirection
        if (input[i] == '>')
        {
            if (input[i + 1] == '>')
            {
                token_add(list, TOKEN_APPEND, ">>");
                i += 2;
            }
            else
            {
                token_add(list, TOKEN_OUTPUT, ">");
                i++;
            }
            continue;
        }

        // Delimiter: BACKGROUND execution
        if (input[i] == '&')
        {
            token_add(list, TOKEN_BACKGROUND, "&");
            i++;
            continue;
        }

        // Start building a WORD token
        char word[MAX_TOKEN_LEN];
        int j = 0;
        int err = 0;

        // Loop to build the word until end of input or a delimiter is found
        while (input[i] != '\0' && !isspace((unsigned char)input[i]) &&
               input[i] != '|' && input[i] != '<' && input[i] != '>' && input[i] != '&')
        {
            char c = input[i];

            if (c == '\'')
            {
                // Single Quote
                i++; // skip opening single quote
                while (input[i] != '\0' && input[i] != '\'')
                {
                    if (j < MAX_TOKEN_LEN - 1)
                    {
                        word[j++] = input[i];
                    }
                    i++;
                }

                if (input[i] == '\'')
                {
                    i++; // skip closing single quote
                }
                else
                {
                    fprintf(stderr, "Lexer Error : Unterminated single quote\n");
                    err = 1;
                    break;
                }
            }
            else if (c == '"')
            {
                // Double Quote
                i++; // skip opening double quote
                while (input[i] != '\0' && input[i] != '"')
                {
                    if (input[i] == '\\' && input[i + 1] != '\0')
                    {
                        // Escape character inside double quotes
                        i++; // skip backslash
                        if (j < MAX_TOKEN_LEN - 1)
                        {
                            word[j++] = input[i];
                        }
                        i++;
                    }
                    else
                    {
                        if (j < MAX_TOKEN_LEN - 1)
                        {
                            word[j++] = input[i];
                        }
                        i++;
                    }
                }

                if (input[i] == '"')
                {
                    i++; // skip closing double quote
                }
                else
                {
                    fprintf(stderr, "Lexer Error : Unterminated double quote\n");
                    err = 1;
                    break;
                }
            }
            else if (c == '\\')
            {
                // Escape character
                i++; // skip backslash
                if (input[i] != '\0')
                {
                    if (j < MAX_TOKEN_LEN - 1)
                    {
                        word[j++] = input[i];
                    }
                    i++;
                }
                else
                {
                    // Backslash at the very end of input
                    break;
                }
            }
            else
            {
                // Normal character
                if (j < MAX_TOKEN_LEN - 1)
                {
                    word[j++] = c;
                }
                i++;
            }
        }

        if (err)
        {
            // Abort token list if lexing error occurs
            list->count = 0;
            return;
        }

        word[j] = '\0';
        token_add(list, TOKEN_WORD, word);
    }

    // Add TOKEN_END at the end of the list
    token_add(list, TOKEN_END, "END");
}
