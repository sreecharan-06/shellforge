#ifndef PARSER_H
#define PARSER_H

#include "token.h"

#define MAX_ARGS 64
#define MAX_COMMANDS 16

typedef struct {
    char *argv[MAX_ARGS + 1];
    int argc;
    char input[256];
    char output[256];
    int append;
    int background;
} command_t;

typedef struct {
    command_t commands[MAX_COMMANDS];
    int command_count;
} pipeline_t;

int parser(token_list_t *tokens, pipeline_t *pipeline);
void command_init(command_t *cmd);
void pipeline_print(const pipeline_t *pipeline);

#endif
