#include <err.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ast/ast.h"
#include "exec_tree.h"

static pid_t exec_fork(struct ast *child, int input_fd, int output_fd)
{
    pid_t pid = fork();
    if (pid < 0)
        errx(1, "Fork failed");

    if (pid == 0) // Processus fils
    {
        if (input_fd != STDIN_FILENO)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        exit(exec_ast(child));
    }

    return pid;
}

bool exec_ast_pipe2(struct ast_list *node, int input_fd)
{
    if (!node)
        return 0;

    int fds[2];
    if (node->next)
    {
        if (pipe(fds) < 0)
            errx(1, "Couldn't create pipe");
    }

    pid_t pid =
        exec_fork(node->child, input_fd, node->next ? fds[1] : STDOUT_FILENO);

    if (input_fd != STDIN_FILENO)
        close(input_fd);
    if (node->next)
        close(fds[1]);

    int status = 0;
    if (node->next)
        status = exec_ast_pipe2(node->next, fds[0]);

    waitpid(pid, &status, 0);

    return WEXITSTATUS(status);
}

bool exec_ast_pipe(struct ast_list *node, int input_fd)
{
    if (!node)
        return 0;

    if (node->next)
        return exec_ast_pipe2(node, input_fd);
    return exec_list_ast(&(node->base));
}

bool exec_until_ast(struct ast *ast)
{
    struct ast_if *ast_until = (struct ast_if *)ast;

    bool res = false;
    while (!exec_ast(ast_until->condition))
    {
        res = true;
        exec_ast(ast_until->then_body);
    }

    return res;
}

bool exec_while_ast(struct ast *ast)
{
    struct ast_if *ast_while = (struct ast_if *)ast;

    bool res = false;
    while (exec_ast(ast_while->condition))
    {
        res = true;
        exec_ast(ast_while->then_body);
    }

    return res;
}
