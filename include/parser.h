#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"

#define MAX_ARGS 64
#define MAX_COMMANDS 16

typedef struct
{
    char *argv[MAX_ARGS];
    int argc;

    char *input;
    char *output;

    int append;
    int background;
} command_t;

typedef struct
{
    command_t commands[MAX_COMMANDS];
    int command_count;
} pipeline_t;

void command_init(command_t *cmd);

int parser(token_list_t *tokens, pipeline_t *pipeline);

void pipeline_print(pipeline_t *pipeline);

void pipeline_free(pipeline_t *pipeline);

#endif
