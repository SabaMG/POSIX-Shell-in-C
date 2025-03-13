#ifndef PARSER_H
#define PARSER_H

#include "ast/ast.h"
#include "lexer/lexer.h"

enum parser_status
{
    PARSER,
    PARSER_IF,
    PARSER_THEN,
    PARSER_ELSE,
    PARSER_ELIF,
    PARSER_ERROR
};

/**
 * \brief Parses an expression or nothing.
 *
 * input =     EOF
 *          |  exp EOF ;
 */
struct ast_list *parse(enum parser_status *status, struct lexer *lexer);

/**
 * \brief Parses sexp expressions separated by + tand -.
 *
 * exp =       sexp  { ( '+' | '-' ) sexp } ;
 */
struct ast *parse_exp(struct lexer *lexer);

void ast_push(struct ast_list *list, struct ast_list *ast);

void addnode(struct ast_list *ast, struct ast *obj);

/*
static struct ast *parse_simplecmd(enum parser_status *status, struct lexer
*lexer);

static struct ast *parse_cmplist(enum parser_status *status, struct lexer
*lexer);

static struct ast *parse_shellcmd(enum parser_status *status, struct lexer
*lexer);

static struct ast *parse_command(enum parser_status *status, struct lexer
*lexer);

static struct ast *parse_pipe(enum parser_status *status, struct lexer *lexer);

static struct ast *parse_andor(enum parser_status *status, struct lexer *lexer);
*/
struct ast *parse_list(enum parser_status *status, struct lexer *lexer);

char *readfile(int fd);

void prettyprint(struct ast *node);

#endif /* ! PARSER_H */
