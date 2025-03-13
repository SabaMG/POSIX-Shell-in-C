#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"
struct lexer *lexer_new(const char *input)
{
    struct lexer *res = calloc(1, sizeof(struct lexer));
    res->pos = 0;
    res->input = input;
    struct token tok = { ERROR, "0" };
    res->current_tok = tok;
    return res;
}

void lexer_free(struct lexer *lexer)
{
    free(lexer);
}

static struct token ProcessTok(char *str)
{
    if (strcmp(str, "if") == 0)
    {
        struct token res = { IF, str };
        return res;
    }
    if (strcmp(str, "while") == 0)
    {
        struct token res = { WHILE, str };
        return res;
    }
    if (strcmp(str, "until") == 0)
    {
        struct token res = { UNTIL, str };
        return res;
    }
    if (strcmp(str, "do") == 0)
    {
        struct token res = { DO, str };
        return res;
    }
    if (strcmp(str, "done") == 0)
    {
        struct token res = { DONE, str };
        return res;
    }
    if (strcmp(str, "fi") == 0)
    {
        struct token res = { FI, str };
        return res;
    }
    if (strcmp(str, "then") == 0)
    {
        struct token res = { THEN, str };
        return res;
    }
    if (strcmp(str, "elif") == 0)
    {
        struct token res = { ELIF, str };
        return res;
    }
    if (strcmp(str, "else") == 0)
    {
        struct token res = { ELSE, str };
        return res;
    }
    if (strcmp(str, "\n") == 0)
    {
        struct token res = { EOL, str };
        return res;
    }
    if (strcmp(str, "for") == 0)
    {
        struct token res = { FOR, str };
        return res;
    }
    struct token res = { WORD, str };
    return res;
}

static struct token ProcessSeparator(char c)
{
    char *str = calloc(1, 2);
    str[0] = c;
    str[1] = '\0';
    if (c == '\n')
    {
        struct token res = { EOL, str };
        return res;
    }
    if (c == '\'')
    {
        struct token res = { SINGLEQUOTE, str };
        return res;
    }
    if (c == ';')
    {
        struct token res = { FULLSTOP, str };
        return res;
    }
    if (c == '!')
    {
        struct token res = { NOT, str };
        return res;
    }
    struct token res = { ERROR, str };
    return res;
}

static struct token redirdroite(struct lexer *lexer)
{
    if (lexer->input[lexer->pos + 1] == '>')
    {
        struct token res = { REDIR, ">>" };
        lexer->pos += 2;
        return res;
    }
    if (lexer->input[lexer->pos + 1] == '&')
    {
        struct token res = { REDIR, ">&" };
        lexer->pos += 2;
        return res;
    }
    if (lexer->input[lexer->pos + 1] == '|')
    {
        struct token res = { REDIR, ">|" };
        lexer->pos += 2;
        return res;
    }
    struct token res = { REDIR, ">" };
    lexer->pos++;
    return res;
}

struct token Redirs(struct lexer *lexer)
{
    char c = lexer->input[lexer->pos];
    if (c == '|')
    {
        if (lexer->input[lexer->pos + 1] == '|')
        {
            struct token res = { ANDOR, "||" };
            lexer->pos += 2;
            return res;
        }
        struct token res = { PIPE, "|" };
        lexer->pos++;
        return res;
    }
    if (c == '&')
    {
        if (lexer->input[lexer->pos + 1] == '&')
        {
            struct token res = { ANDOR, "&&" };
            lexer->pos += 2;
            return res;
        }
    }
    if (c == '<')
    {
        if (lexer->input[lexer->pos + 1] == '&')
        {
            struct token res = { REDIR, "<&" };
            lexer->pos += 2;
            return res;
        }
        if (lexer->input[lexer->pos + 1] == '>')
        {
            struct token res = { REDIR, "<>" };
            lexer->pos += 2;
            return res;
        }
        struct token res = { REDIR, "<" };
        lexer->pos++;
        return res;
    }
    if (c == '>')
    {
        return redirdroite(lexer);
    }

    errx(2, "unkown redir");
}
static struct token jsp(char c, char *word)
{
    if (c == '<' || c == '>')
    {
        size_t j = 0;
        while (isdigit(*(word + j)))
        {
            j++;
        }
        if (*(word + j) == '\0')
        {
            struct token tok = { IONUMBER, word };
            return tok;
        }
    }
    return ProcessTok(word);
}

static bool dq(struct lexer *lexer, bool dquoted)
{
    lexer->pos++;
    return (!dquoted);
}

static bool clangcontrol(char c)
{
    return (c != '\0' && c != ';' && c != ' ' && c != '\n' && c != '<'
            && c != '>');
}

struct token lexer_next_token(struct lexer *lexer, bool dquoted, bool quoted)
{
    while (lexer->input[lexer->pos] == ' ')
        lexer->pos++;

    if (lexer->input[lexer->pos] == '\0' || lexer->input[lexer->pos] == EOF)
    {
        struct token tok = { TEOF, 0 };
        return tok;
    }
    char c = lexer->input[lexer->pos];

    if (c == '<' || c == '>' || c == '|' || c == '&')
    {
        struct token res = Redirs(lexer);
        return res;
    }
    if (c == '\n' || c == '\t' || c == ';' || c == '!')
    {
        struct token res = ProcessSeparator(c);
        lexer->pos++;
        return res;
    }

    if (c == '#')
    {
        while (c != '\n' && c != '\0')
        {
            lexer->pos++;
            c = lexer->input[lexer->pos];
        }
        if (lexer->input[lexer->pos] == '\0')
        {
            struct token tok = { EOF, 0 };
            return tok;
        }
        lexer->pos++;
        c = lexer->input[lexer->pos];
    }
    char *word = calloc(1, 1);
    size_t i = 0;
    while (clangcontrol(c) || quoted || dquoted)
    {
        if (c == '\0' && (quoted || dquoted))
        {
            errx(2, "missing quote");
        }
        else if (c == '"' && !quoted)
        {
            dquoted = dq(lexer, dquoted);
        }
        else if (c == '\'' && !dquoted)
        {
            quoted = dq(lexer, quoted);
        }
        else
        {
            word = realloc(word, i + 2);
            *(word + i) = c;
            lexer->pos++;
            i++;
        }
        c = lexer->input[lexer->pos];
    }
    *(word + i) = '\0';
    return jsp(c, word);
}

struct token lexer_peek(struct lexer *lexer)
{
    size_t old_pos = lexer->pos;
    struct token tok = lexer_next_token(lexer, false, false);
    lexer->pos = old_pos;
    if (tok.type != REDIR && tok.type != ANDOR && tok.type != PIPE)
        free(tok.value);
    return tok;
}

struct token lexer_peek_keep(struct lexer *lexer)
{
    size_t old_pos = lexer->pos;
    struct token tok = lexer_next_token(lexer, false, false);
    lexer->pos = old_pos;
    return tok;
}

struct token lexer_pop(struct lexer *lexer)
{
    return lexer_next_token(lexer, false, false);
}
