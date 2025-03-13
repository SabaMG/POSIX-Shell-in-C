#ifndef REDIR_H
#define REDIR_H

#include <stddef.h>

#include "ast/ast.h"

bool redir(char **words, char **content, enum ast_type type, size_t len);

#endif /* ! REDIR_H */
