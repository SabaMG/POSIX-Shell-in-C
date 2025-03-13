#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "builtins.h"
#include "exec_tree.h"

int redirect_right(char **cmd, char *file, int append, size_t len)
{
    int file_fd = -1;
    if (append == 0)
        file_fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    else
        file_fd = open(file, O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (file_fd == -1)
    {
        printf("\n---\npwd : %s\n", getenv("PWD"));
        perror("open");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        close(file_fd);
        return 1;
    }

    int save = dup(STDOUT_FILENO);

    if (pid == 0)
    {
        if (dup2(file_fd, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(file_fd);
            exit(1);
        }
        if (strcmp(cmd[0], "echo") == 0)
            return echo(cmd, len) == 0 ? true : false;

        execvp(cmd[0], cmd);
        perror(cmd[0]);
        exit(127);

        exit(1);
    }
    else
    {
        if (dup2(save, STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(save);
            exit(1);
        }
        close(file_fd);

        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
        {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 127)
            {
                return 1;
            }
        }
    }
    return 0;
}

bool redir(char **words, char **content, enum ast_type type, size_t len)
{
    if (type == AST_SIMPLE_COMMAND)
    {
        char *pwd = getpwd();

        char *temp =
            calloc(1, strlen(pwd) + strlen(content[1]) + 2); // add pwd to path
        strcat(temp, pwd);
        if (pwd[strlen(pwd) - 1] != '/')
            strcat(temp, "/");
        strcat(temp, content[1]);

        if (strcmp(">", content[0]) == 0 || strcmp(">|", content[0]) == 0)
        {
            bool res = redirect_right(words, temp, 0, len);
            free(temp);
            free(pwd);
            return res;
        }

        if (strcmp(">>", content[0]) == 0)
        {
            bool res = redirect_right(words, temp, 1, len);
            free(temp);
            free(pwd);
            return res;
        }
    }
    return false;
}
