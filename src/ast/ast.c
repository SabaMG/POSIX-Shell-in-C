#include "ast.h"

#include "lexer/token.h"
// #include "../lexer/token.h"
#include <assert.h>
#include <stdlib.h>

struct ast_cmd *initcommand(void)
{
    struct ast_cmd *res = calloc(1, sizeof(struct ast_cmd));
    res->words = calloc(1, 1);
    res->len = 0;
    res->base.type = AST_SIMPLE_COMMAND;
    return res;
}

struct ast_if *initif(void)
{
    struct ast_if *res = calloc(1, sizeof(struct ast_if));
    res->base.type = AST_IF;
    return res;
}

struct ast_list *initlist(void)
{
    struct ast_list *ast = calloc(1, sizeof(struct ast_list));
    ast->base.type = AST_LIST;
    return ast;
}

struct ast_list *initpipe(void)
{
    struct ast_list *ast = calloc(1, sizeof(struct ast_list));
    ast->base.type = AST_PIPELINE;
    return ast;
}

struct ast_and_or *initandor(void)
{
    struct ast_and_or *ast = calloc(1, sizeof(struct ast_and_or));
    ast->base.type = AST_AND_OR;
    return ast;
}

struct ast_not *initnot(void)
{
    struct ast_not *ast = calloc(1, sizeof(struct ast_not));
    ast->base.type = AST_NOT;
    return ast;
}

static void free_ast_redir(struct ast_redir *ast)
{
    for (size_t i = 0; ast->content[i]; i++)
    {
        if (*(ast->content[i]) != '>' && *(ast->content[i]) != '<')
            free(ast->content[i]);
    }
    free(ast->content);
    if (ast->next)
        free_ast(ast->next);
    free(ast);
}

static void free_ast_cmd(struct ast *ast)
{
    struct ast_cmd *ast_cmd = (struct ast_cmd *)ast;
    for (int i = 0; ast_cmd->words[i]; i++)
        free(ast_cmd->words[i]);
    free(ast_cmd->words);
    if (ast_cmd->redir)
        free_ast_redir(ast_cmd->redir);
    free(ast_cmd);
}

static void free_ast_command(struct ast *ast)
{
    struct ast_command *ast_command = (struct ast_command *)ast;
    free_ast(ast_command->child);
    if (ast_command->redir)
        free_ast_redir(ast_command->redir);
    free(ast_command);
}

static void free_ast_and_or(struct ast *ast)
{
    struct ast_and_or *ast_and_or = (struct ast_and_or *)ast;
    if (ast_and_or->next)
        free_ast_and_or((struct ast *)ast_and_or->next);
    if (ast_and_or->child)
        free_ast(ast_and_or->child);
    free(ast_and_or);
}

static void free_ast_not(struct ast *ast)
{
    struct ast_not *ast_not = (struct ast_not *)ast;
    if (ast_not->child)
        free_ast(ast_not->child);
    free(ast_not);
}

static void free_ast_elif(struct ast_elif *ast)
{
    free_ast(ast->condition);
    free_ast(ast->then);
    free_ast(ast->next);
    free(ast);
}

static void free_ast_if(struct ast *ast)
{
    struct ast_if *ast_if = (struct ast_if *)ast;
    free_ast(ast_if->condition);
    free_ast(ast_if->then_body);
    free_ast(ast_if->else_body);
    free(ast_if);
}

static void free_ast_list(struct ast *ast)
{
    struct ast_list *ast_list = (struct ast_list *)ast;
    free_ast((struct ast *)ast_list->next);
    free_ast(ast_list->child);
    free(ast_list);
}

void free_ast(struct ast *ast)
{
    if (ast)
    {
        if (ast->type == AST_SIMPLE_COMMAND)
            free_ast_cmd(ast);

        else if (ast->type == AST_COMMAND)
            free_ast_command(ast);

        else if (ast->type == AST_IF || ast->type == AST_UNTIL
                 || ast->type == AST_WHILE)
            free_ast_if(ast);

        else if (ast->type == AST_LIST || ast->type == AST_PIPELINE)
            free_ast_list(ast);

        else if (ast->type == AST_AND_OR)
            free_ast_and_or(ast);

        else if (ast->type == AST_NOT)
            free_ast_not(ast);

        else if (ast->type == AST_ELIF)
            free_ast_elif((struct ast_elif *)ast);

        else
            free(ast);
    }
}
