#ifndef AST_H
#define AST_H

#include <unistd.h>

enum ast_type
{
    AST_INPUT,
    AST_LIST,
    AST_AND_OR,
    AST_PIPELINE,
    AST_COMMAND,
    AST_SIMPLE_COMMAND,
    AST_IF,
    AST_REDIR,
    AST_ELIF,
    AST_WHILE,
    AST_UNTIL,
    AST_ELEMENT,
    AST_NOT
};

struct ast
{
    enum ast_type type;
};

struct ast_and_or
{
    struct ast base;
    struct ast_and_or *next;
    struct ast *child;
    size_t oper; // 0 no operator ; 1 and ; 2 or
};

struct ast_while
{
    struct ast base;
    struct ast *next;
    struct ast *child;
};

struct ast_command
{
    struct ast base;
    struct ast *child;
    struct ast_redir *redir;
};

struct ast_redir
{
    struct ast base;
    char **content;
    size_t len;
    struct ast *next;
};

struct ast_cmd // simple command
{
    struct ast base;
    char **words; // NULL terminated char* list
    size_t len;
    struct ast_redir *redir;
};

struct ast_list
{
    struct ast base;
    struct ast_list *next;
    struct ast *child;
};

struct ast_elif
{
    struct ast base;
    struct ast *condition;
    struct ast *then;
    struct ast *next;
};

struct ast_if
{
    struct ast base;
    struct ast *condition;
    struct ast *then_body;
    struct ast *else_body;
    struct ast_elif *elif;
};

struct ast_not
{
    struct ast base;
    struct ast *child;
};

struct ast_cmd *initcommand(void);
struct ast_if *initif(void);
struct ast_list *initlist(void);
struct ast_list *initpipe(void);
struct ast_and_or *initandor(void);
struct ast_not *initnot(void);
void free_ast(struct ast *ast);

#endif /* ! AST_H */
