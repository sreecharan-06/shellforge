#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include "history.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "expand.h"
#include "builtin.h"
#include "executor.h"

int main(void)
{
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Display a welcome banner when the shell starts
    printf("=====================================\n");
    printf("      Shellforge \n");
    printf(" A Unix Style Shell written in C\n");
    printf("=====================================\n");

    token_list_t tokens;
    pipeline_t pipeline;
    char *line;

    while (1)
    {
        line = readline("shellforge$ ");
        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        if (strcmp(line, "history") == 0)
        {
            print_history();
            add_history(line);
            free(line);
            continue;
        }

        add_history(line);

        lexer(line, &tokens);
        token_print(&tokens);

        if (parser(&tokens, &pipeline))
        {
            expand_variables(&pipeline);
            pipeline_print(&pipeline);
            
            for (int i = 0; i < pipeline.command_count; i++)
            {
                int result = execute_command(&pipeline.commands[i]);

                // Only exit the shell if the 'exit' built-in was explicitly called and returned 1 (success)
                if (is_builtin(&pipeline.commands[i]) && strcmp(pipeline.commands[i].argv[0], "exit") == 0)
                {
                    if (result == 1)
                    {
                        free(line);
                        return 0;
                    }
                }
            }
        }

        free(line);
    }
    return 0;
}
