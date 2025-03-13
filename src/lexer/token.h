#ifndef TOKEN_H
#define TOKEN_H

#include <unistd.h>

enum token_type
{
    IF,
    THEN,
    FULLSTOP,
    ELIF,
    ELSE,
    FI,
    EOL,
    SINGLEQUOTE,
    DOUBLEQUOTE,
    WORD,
    NOT,
    REDIR,
    DO,
    DONE,
    WHILE,
    ERROR,
    IONUMBER,
    PIPE,
    ANDOR,
    UNTIL,
    FOR,
    IN,
    TEOF
};

struct token
{
    enum token_type type;
    char *value;
};

#endif /* ! TOKEN_H */
