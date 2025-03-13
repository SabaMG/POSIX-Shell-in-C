#include "exec_tree.h"

#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../ast/ast.h"
#include "builtins.h"
#include "redir.h"

bool exec_ast(struct ast *ast)
{
    if (ast->type == AST_COMMAND)
        return exec_cmd_ast(ast);

    if (ast->type == AST_SIMPLE_COMMAND)
        return exec_simple_cmd_ast(ast);

    if (ast->type == AST_IF)
        return exec_if_ast(ast);

    if (ast->type == AST_LIST)
        return exec_list_ast(ast);

    if (ast->type == AST_PIPELINE)
        return exec_ast_pipe((struct ast_list *)ast, 0);

    if (ast->type == AST_WHILE)
        return exec_while_ast(ast);

    if (ast->type == AST_UNTIL)
        return exec_until_ast(ast);

    if (ast->type == AST_ELIF)
        return exec_elif_ast(ast);

    if (ast->type == AST_NOT)
        return exec_ast_not(ast);

    if (ast->type == AST_AND_OR)
        return exec_ast_and_or(ast);

    return false;
}

bool exec_ast_not(struct ast *ast)
{
    struct ast_not *ast_not = (struct ast_not *)ast;

    return !(exec_ast(ast_not->child));
}

bool exec_ast_and_or(struct ast *ast)
{
    struct ast_and_or *ast_and_or = (struct ast_and_or *)ast;

    if (exec_ast(ast_and_or->child))
    {
        if (ast_and_or->oper == 1)
            return exec_ast(&(ast_and_or->next->base));

        return true;
    }
    else
    {
        if (ast_and_or->oper == 2)
            return exec_ast(&(ast_and_or->next->base));

        return false;
    }
}

bool exec_elif_ast(struct ast *ast)
{
    struct ast_elif *ast_elif = (struct ast_elif *)ast;
    bool res = false;
    if (exec_ast(ast_elif->condition))
    {
        res = true;
        exec_ast(ast_elif->then);
    }
    return res;
}

bool exec_cmd_ast(struct ast *ast)
{
    struct ast_command *ast_command = (struct ast_command *)ast;

    return exec_ast(ast_command->child);
}

bool run_command(char **words, size_t len)
{
    if (strcmp(words[0], "false") == 0)
        return false;
    if (strcmp(words[0], "true") == 0)
        return true;
    if (strcmp(words[0], "echo") == 0)
        return echo(words, len) == 0 ? true : false;
    if (strcmp(words[0], "cd") == 0)
        return cd(words, len) == 0 ? true : false;

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        execvp(words[0], words);
        perror("execvp");
        exit(errno == ENOENT ? 127 : 1);
    }
    else
    {
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 0)
                return true;
            errx(WEXITSTATUS(status), "commande introuvable");
        }
        else if (WIFSIGNALED(status))
        {
            printf("Commande terminée par le signal : %d\n", WTERMSIG(status));
        }
        else
        {
            printf("Commande terminée de manière inattendue\n");
        }
        return false;
    }
}

bool exec_simple_cmd_ast(struct ast *ast)
{
    struct ast_cmd *ast_cmd = (struct ast_cmd *)ast;
    if (ast_cmd->redir)
    {
        bool res = true;
        for (struct ast_redir *ast_redir = ast_cmd->redir; ast_redir;
             ast_redir = (struct ast_redir *)ast_redir->next)
            res = res
                && redir(ast_cmd->words, ast_redir->content, ast->type,
                         ast_cmd->len);
        return res;
    }
    return run_command(ast_cmd->words, ast_cmd->len);
}

bool exec_if_ast(struct ast *ast)
{
    struct ast_if *if_ast = (struct ast_if *)ast;

    bool condition = exec_ast(if_ast->condition);

    if (condition)
        return exec_ast(if_ast->then_body);
    else if (if_ast->elif)
        return exec_list_ast(&(if_ast->elif->base));
    else
        return exec_ast(if_ast->else_body);
}

bool exec_list_ast(struct ast *ast)
{
    if (ast->type == AST_LIST)
    {
        struct ast_list *list_ast = (struct ast_list *)ast;

        if (list_ast->next == NULL)
            return exec_ast(list_ast->child);
        else
        {
            exec_ast(list_ast->child);
            return exec_ast(&(list_ast->next->base));
        }
    }

    else
    {
        struct ast_list *list_ast = (struct ast_list *)ast;

        if (list_ast->next == NULL)
            return exec_ast(list_ast->child);
        else
        {
            exec_ast(list_ast->child); // il faut modifier cette parti car ici
                                       // on doit stocker la valeurs et pas la
                                       // renvoyer tout de suite
            return exec_ast(&(list_ast->next->base));
        }
    }
}
