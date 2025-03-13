#ifndef EXEC_TREE_H
#define EXEC_TREE_H
#include <stdbool.h>
#include <stddef.h>

#include "ast/ast.h"

bool exec_cmd_ast(struct ast *ast);

bool exec_simple_cmd_ast(struct ast *ast);

bool exec_if_ast(struct ast *ast);

bool exec_ast(struct ast *ast);

bool exec_list_ast(struct ast *ast);

bool exec_elif_ast(struct ast *ast);

bool exec_while_ast(struct ast *ast);

bool exec_until_ast(struct ast *ast);

bool exec_ast_not(struct ast *ast);

bool exec_ast_and_or(struct ast *ast);

bool run_command(char **words, size_t len);

bool exec_ast_pipe(struct ast_list *node, int input_fd);

#endif /* ! EXEC_TREE_H */
