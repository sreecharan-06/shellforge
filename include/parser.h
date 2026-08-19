#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_ARGS 64
#define MAX_COMMANDS 16

typedef struct
{
    char *argv[MAX_ARGS];

    char *input;
    char *output;

    int append;
    int background;
} Command;

typedef struct
{
    Command commands[MAX_COMMANDS];
    int command_count;
} Pipeline;

void command_init(Command *cmd);

int parse(Token **tokens, Pipeline *pipeline);

void pipeline_print(Pipeline *pipeline);

void pipeline_free(Pipeline *pipeline);

#endif
