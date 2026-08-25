#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

#include "executor.h"
#include "builtin.h"

/*
 * Execute one command.
 *
 * Returns:
 *      0  -> success
 *     -1  -> error
 */
int execute_command(command_t *cmd)
{
    pid_t pid;
    int status;

    /*
     * Check whether the command is valid.
     */
    if (cmd == NULL || cmd->argc == 0)
    {
        return -1;
    }

    /*
     * ------------------------------------------------
     * Check for built-in command
     * ------------------------------------------------
     *
     * Built-ins such as cd, pwd, echo and exit
     * are handled by the shell itself.
     * Support redirection for built-ins by saving and
     * restoring stdin/stdout in the parent process.
     */
    if (is_builtin(cmd))
    {
        // Flush before duping to ensure buffers are clear
        fflush(stdout);
        fflush(stderr);

        int saved_stdin = dup(STDIN_FILENO);
        int saved_stdout = dup(STDOUT_FILENO);
        int err = 0;

        // Redirect input
        if (cmd->input[0] != '\0')
        {
            int in_fd = open(cmd->input, O_RDONLY);
            if (in_fd < 0)
            {
                perror("Shellforge: input redirection");
                err = 1;
            }
            else
            {
                if (dup2(in_fd, STDIN_FILENO) < 0)
                {
                    perror("Shellforge: dup2 input");
                    err = 1;
                }
                close(in_fd);
            }
        }

        // Redirect output
        if (!err && cmd->output[0] != '\0')
        {
            int out_fd;
            if (cmd->append)
            {
                out_fd = open(cmd->output, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            }
            else
            {
                out_fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            }

            if (out_fd < 0)
            {
                perror("Shellforge: output redirection");
                err = 1;
            }
            else
            {
                if (dup2(out_fd, STDOUT_FILENO) < 0)
                {
                    perror("Shellforge: dup2 output");
                    err = 1;
                }
                close(out_fd);
            }
        }

        int result = -1;
        if (!err)
        {
            result = execute_builtin(cmd);
            // Flush again so that any output from the built-in is written to the redirected destination
            fflush(stdout);
            fflush(stderr);
        }

        // Restore stdin/stdout
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdin);
        close(saved_stdout);

        return result;
    }

    /*
     * ------------------------------------------------
     * Create a child process
     * ------------------------------------------------
     */
    fflush(stdout);
    fflush(stderr);
    pid = fork();

    /*
     * fork() failed
     */
    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    /*
     * ------------------------------------------------
     * CHILD PROCESS
     * ------------------------------------------------
     */
    if (pid == 0)
    {
        /*
         * Handle Input Redirection
         */
        if (cmd->input[0] != '\0')
        {
            int in_fd = open(cmd->input, O_RDONLY);
            if (in_fd < 0)
            {
                perror("Shellforge: input redirection");
                exit(EXIT_FAILURE);
            }
            if (dup2(in_fd, STDIN_FILENO) < 0)
            {
                perror("Shellforge: dup2 input");
                close(in_fd);
                exit(EXIT_FAILURE);
            }
            close(in_fd);
        }

        /*
         * Handle Output Redirection (Output or Append)
         */
        if (cmd->output[0] != '\0')
        {
            int out_fd;
            if (cmd->append)
            {
                out_fd = open(cmd->output, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            }
            else
            {
                out_fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            }

            if (out_fd < 0)
            {
                perror("Shellforge: output redirection");
                exit(EXIT_FAILURE);
            }
            if (dup2(out_fd, STDOUT_FILENO) < 0)
            {
                perror("Shellforge: dup2 output");
                close(out_fd);
                exit(EXIT_FAILURE);
            }
            close(out_fd);
        }

        /*
         * execvp() expects:
         *
         * argv[0] = command
         * argv[1] = argument
         * ...
         * argv[argc] = NULL
         */
        char *args[MAX_ARGS + 1];

        /*
         * Convert our 2D character array into
         * an array of pointers.
         */
        for (int i = 0; i < cmd->argc; i++)
        {
            args[i] = cmd->argv[i];
        }

        /*
         * execvp() requires NULL termination.
         */
        args[cmd->argc] = NULL;

        /*
         * Replace the child process with
         * the requested external command.
         */
        execvp(args[0], args);

        /*
         * If execvp() returns, execution failed.
         */
        perror(args[0]);
        exit(EXIT_FAILURE);
    }

    /*
     * ------------------------------------------------
     * PARENT PROCESS
     * ------------------------------------------------
     */

    /*
     * If it is a background command, we do not wait.
     */
    if (cmd->background)
    {
        printf("[Process run in background: PID %d]\n", pid);
        return 0;
    }

    /*
     * Wait for the child process to finish.
     */
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return -1;
    }

    /*
     * Check how the child terminated.
     */
    if (WIFEXITED(status))
    {
        /*
         * Return child's exit status.
         */
        return WEXITSTATUS(status);
    }

    /*
     * Child terminated abnormally.
     */
    if (WIFSIGNALED(status))
    {
        fprintf(stderr, "Process terminated by signal %d\n", WTERMSIG(status));
        return -1;
    }

    return 0;
}
