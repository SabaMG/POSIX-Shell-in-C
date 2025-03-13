#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser.h"

static struct ast *parse_simplecmd(enum parser_status *status,
                                   struct lexer *lexer);

static struct ast *parse_cmplist(enum parser_status *status,
                                 struct lexer *lexer);

static struct ast *parse_shellcmd(enum parser_status *status,
                                  struct lexer *lexer);

static struct ast *parse_command(enum parser_status *status,
                                 struct lexer *lexer);

static struct ast *parse_pipe(enum parser_status *status, struct lexer *lexer);

static struct ast *parse_andor(enum parser_status *status, struct lexer *lexer);

static int IsWord(struct token tok)
{
    return (tok.type == WORD || tok.type == NOT || tok.type == IF
            || tok.type == ELSE || tok.type == ELIF || tok.type == FI
            || tok.type == WHILE || tok.type == DO || tok.type == DONE
            || tok.type == THEN || tok.type == UNTIL || tok.type == FOR
            || tok.type == IN);
}

static int isshell(struct token tok)
{
    return (tok.type == IF || tok.type == WHILE || tok.type == UNTIL
            || tok.type == FOR);
}

static int IsRedir(struct token tok)
{
    return (tok.type == REDIR || tok.type == IONUMBER);
}

int IsKeyword(struct token tok)
{
    return (IsWord(tok) && (tok.type != WORD));
}

static int IsAndor(struct token tok)
{
    return (tok.type == WORD || tok.type == IF || tok.type == WHILE
            || tok.type == UNTIL || tok.type == FOR || tok.type == NOT
            || IsRedir(tok));
}

static int IsPrefix(struct token tok)
{
    return IsRedir(tok);
}

static int IsCommand(struct token tok)
{
    return isshell(tok) || tok.type == WORD || IsPrefix(tok);
}

static int IsPipe(struct token tok)
{
    return IsCommand(tok) || tok.type == NOT;
}

static void push_redir(struct ast *src, struct ast_redir *dest)
{
    while (dest->next)
    {
        dest = (struct ast_redir *)(dest->next);
    }
    dest->next = src;
}

static struct ast *parse_redir(enum parser_status *status, struct lexer *lexer)
{
    struct ast_redir *ast = calloc(1, sizeof(struct ast_redir));
    struct token tok = lexer_pop(lexer);
    int i = 0;
    ast->base.type = AST_REDIR;
    ast->content = calloc(4, sizeof(char *));
    if (tok.type == IONUMBER)
    {
        ast->content[i] = tok.value;
        tok = lexer_pop(lexer);
        i++;
    }
    if (tok.type != REDIR)
    {
        errx(2, "Expected REDIR");
    }
    ast->content[i] = tok.value;
    tok = lexer_pop(lexer);
    i++;
    if (!IsWord(tok))
    {
        errx(2, "Expected Word");
    }
    ast->content[i] = tok.value;

    if (lexer_peek(lexer).type == IONUMBER || lexer_peek(lexer).type == REDIR)
    {
        ast->next = (parse_redir(status, lexer));
    }
    return &ast->base;
}

static struct ast *parse_element(enum parser_status *status,
                                 struct lexer *lexer, struct ast_cmd *ast)
{
    if (IsWord(lexer_peek(lexer)))
    {
        ast->words[ast->len] = lexer_pop(lexer).value;
        ast->len++;
        ast->words = realloc(ast->words, (ast->len + 2) * sizeof(char *));
    }
    else
    {
        if (ast->redir)
            push_redir(parse_redir(status, lexer), ast->redir);
        else
            ast->redir = (struct ast_redir *)parse_redir(status, lexer);
    }
    return &ast->base;
}

static struct ast *parse_prefix(enum parser_status *status, struct lexer *lexer)
{
    return parse_redir(status, lexer);
}

static struct ast *parse_simplecmd(enum parser_status *status,
                                   struct lexer *lexer)
{
    struct token token = lexer_peek(lexer);

    struct ast_cmd *ast = initcommand();
    if (IsPrefix(token))
    {
        ast->redir = (struct ast_redir *)(parse_prefix(status, lexer));
    }

    token = lexer_peek(lexer);

    if (!IsWord(token))
    {
        if (ast->redir)
        {
            return &(ast->base);
        }
        *status = PARSER_ERROR;
        errx(2, "Expected WORD");
    }

    token = lexer_pop(lexer);
    ast->words = realloc(ast->words, (ast->len + 2) * sizeof(char *));
    *(ast->words + ast->len) = token.value;
    ast->len += 1;
    token = lexer_peek(lexer);

    while (IsWord(token) || IsRedir(token))
    {
        ast = (struct ast_cmd *)(parse_element(status, lexer, ast));
        token = lexer_peek(lexer);
    }
    *(ast->words + ast->len) = NULL;
    return &(ast->base);
}

static struct ast *parse_cmplist(enum parser_status *status,
                                 struct lexer *lexer)
{
    struct token t = lexer_peek(lexer);
    // free(t.value);
    while (lexer_peek(lexer).type == EOL)
    {
        t = lexer_pop(lexer);
        free(t.value);
    }
    struct ast_list *ast = calloc(1, sizeof(struct ast_list));
    ast->base.type = AST_LIST;
    ast->child = parse_andor(status, lexer);
    if (lexer_peek(lexer).type == FULLSTOP || lexer_peek(lexer).type == EOL)
    {
        t = lexer_pop(lexer);
        free(t.value);
        while (lexer_peek(lexer).type == EOL)
        {
            t = lexer_pop(lexer);
            free(t.value);
        }
        struct token tok = lexer_peek(lexer);
        if (IsAndor(tok))
        {
            ast->next = (struct ast_list *)(parse_cmplist(status, lexer));
        }
    }
    return &(ast->base);
}

static struct ast *elif_clause(enum parser_status *status, struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type == ELIF)
    {
        tok = lexer_pop(lexer);
        free(tok.value);
        struct ast_elif *ast = calloc(1, sizeof(struct ast_elif));
        ast->base.type = AST_ELIF;
        ast->condition = parse_cmplist(status, lexer);
        tok = lexer_peek(lexer);
        if (tok.type != THEN)
        {
            *status = PARSER_ERROR;
            errx(2, "Expected THEN");
        }

        tok = lexer_pop(lexer);
        free(tok.value);
        ast->then = parse_cmplist(status, lexer);
        tok = lexer_peek(lexer);
        if (tok.type == ELIF)
        {
            ast->next = (elif_clause(status, lexer));
        }
        return &(ast->base);
    }

    return NULL;
}

static struct ast *parse_if(enum parser_status *status, struct lexer *lexer)
{
    struct token token = lexer_pop(lexer);
    free(token.value);
    struct ast_if *ast_if = initif();
    ast_if->base.type = AST_IF;
    ast_if->condition = parse_cmplist(status, lexer);
    token = lexer_peek(lexer);
    if (token.type != THEN)
    {
        *status = PARSER_ERROR;
        errx(2, "Expected THEN");
    }
    token = lexer_pop(lexer);
    free(token.value);
    ast_if->then_body = parse_cmplist(status, lexer);
    if (lexer_peek(lexer).type == ELIF)
    {
        struct ast *el = elif_clause(status, lexer);
        ast_if->elif = (struct ast_elif *)el;
    }
    if (lexer_peek(lexer).type == ELSE)
    {
        token = lexer_pop(lexer);
        free(token.value);
        ast_if->else_body = parse_cmplist(status, lexer);
    }

    token = lexer_peek(lexer);
    if (token.type != FI)
    {
        *status = PARSER_ERROR;
        errx(2, "Expected FI");
    }

    token = lexer_pop(lexer);
    free(token.value);
    return &(ast_if->base);
}

static struct ast *parse_while(enum parser_status *status, struct lexer *lexer)
{
    struct token token = lexer_pop(lexer);
    struct ast_if *ast_if = initif();
    if (token.type == WHILE)
        ast_if->base.type = AST_WHILE;
    else
        ast_if->base.type = AST_UNTIL;
    ast_if->condition = parse_cmplist(status, lexer);
    free(token.value);
    token = lexer_peek(lexer);
    if (token.type != DO)
    {
        *status = PARSER_ERROR;
        errx(2, "Expected DO");
    }
    token = lexer_pop(lexer);
    free(token.value);
    ast_if->then_body = parse_cmplist(status, lexer);

    token = lexer_peek(lexer);
    if (token.type != DONE)
    {
        *status = PARSER_ERROR;
        errx(2, "Expected DONE");
    }
    token = lexer_pop(lexer);
    free(token.value);
    return &(ast_if->base);
}

static struct ast *parse_shellcmd(enum parser_status *status,
                                  struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type == IF)
    {
        return parse_if(status, lexer);
    }
    if (tok.type == FOR)
    {
        errx(2, "FOR, NIGHT?");
    }
    return parse_while(status, lexer);
}
static struct ast *parse_command(enum parser_status *status,
                                 struct lexer *lexer)
{
    struct token tok = lexer_peek(lexer);
    if (tok.type == WORD || IsPrefix(tok))
    {
        return parse_simplecmd(status, lexer);
    }
    else if (isshell(tok))
    {
        struct ast *ast = parse_shellcmd(status, lexer);
        tok = lexer_peek(lexer);
        if (tok.type == IONUMBER || tok.type == REDIR)
        {
            struct ast_command *command = calloc(1, sizeof(struct ast_command));
            command->base.type = AST_COMMAND;
            command->redir = (struct ast_redir *)parse_redir(status, lexer);
            command->child = ast;
            tok = lexer_peek(lexer);
            while (IsRedir(tok))
            {
                push_redir(parse_redir(status, lexer), command->redir);
            }
            return &command->base;
        }

        else
        {
            return ast;
        }
    }
    else
    {
        errx(2, "Wrong Synthax");
        return NULL;
    }
}

static struct ast *parse_pipe(enum parser_status *status, struct lexer *lexer)
{
    struct ast_list *ast = initpipe();
    ast->child = parse_command(status, lexer);
    if (lexer_peek(lexer).type == PIPE)
    {
        struct token tok = lexer_pop(lexer);
        while (lexer_peek(lexer).type == EOL)
        {
            tok = lexer_pop(lexer);
            free(tok.value);
        }
        tok = lexer_peek(lexer);
        if (IsCommand(tok))
        {
            ast->next = (struct ast_list *)parse_pipe(status, lexer);
        }
        else
        {
            errx(2, "Missing command");
        }
    }

    return &(ast->base);
}

static struct ast *parse_pipe2(enum parser_status *status, struct lexer *lexer)
{
    if (lexer_peek(lexer).type == NOT)
    {
        struct ast_not *ast = initnot();
        struct token t = lexer_pop(lexer);
        free(t.value);
        ast->child = parse_pipe(status, lexer);
        return &(ast->base);
    }
    return parse_pipe(status, lexer);
}

static struct ast *parse_andor(enum parser_status *status, struct lexer *lexer)
{
    struct ast_and_or *ast = initandor();
    ast->child = parse_pipe2(status, lexer);
    if (lexer_peek(lexer).type == ANDOR)
    {
        if (strcmp(lexer_peek(lexer).value, "&&") == 0)
            ast->oper = 1;
        else
            ast->oper = 2;
        struct token tok = lexer_pop(lexer);
        while (lexer_peek(lexer).type == EOL)
        {
            tok = lexer_pop(lexer);
            free(tok.value);
        }
        tok = lexer_peek(lexer);
        if (IsPipe(tok))
        {
            ast->next = (struct ast_and_or *)parse_andor(status, lexer);
        }
        else
        {
            errx(2, "Missing pipeline");
        }
    }
    return &(ast->base);
}

struct ast *parse_list(enum parser_status *status, struct lexer *lexer)
{
    struct token t = lexer_peek(lexer);
    // free(t.value);
    while (lexer_peek(lexer).type == EOL)
    {
        t = lexer_pop(lexer);
        free(t.value);
    }
    struct ast_list *ast = calloc(1, sizeof(struct ast_list));
    ast->base.type = AST_LIST;
    ast->child = parse_andor(status, lexer);

    if (lexer_peek(lexer).type == EOL || lexer_peek(lexer).type == FULLSTOP)
    {
        t = lexer_pop(lexer);
        free(t.value);

        while (lexer_peek(lexer).type == EOL)
        {
            t = lexer_pop(lexer);
            free(t.value);
        }
        if (lexer_peek(lexer).type == FULLSTOP)
        {
            errx(2, "empty command");
        }
        struct token tok = lexer_peek(lexer);
        if (IsAndor(tok))
        {
            ast->next = (struct ast_list *)(parse_list(status, lexer));
        }
    }
    return &(ast->base);
    /*struct ast_list *ast = initlist();
    ast->child = parse_andor(status, lexer);
    if (lexer_peek(lexer).type == FULLSTOP) {
      struct token tok = lexer_pop(lexer);
      free(tok.value);
      tok = lexer_peek(lexer);
      if (IsAndor(tok)) {
        ast->next = (struct ast_list *)(parse_list(status, lexer));
      }
    }
    return &(ast->base);
    */
}

char *readfile(int fd)
{
    if (fd == -1)
    {
        errx(2, "incorrect file");
    }
    char *list = calloc(1, sizeof(char));
    unsigned int size = 0;
    char buf[64];
    unsigned int i;
    while ((i = read(fd, buf, 64)) > 0)
    {
        list = realloc(list, size + i + 2);
        for (unsigned int j = 0; j < i; j++)
        {
            *(list + size + j) = buf[j];
        }
        size += i;
    }
    *(list + size) = '\0';
    return list;
}

/*int main(int argc, char *argv[])
{
  enum parser_status stat;
  struct lexer *lexer;
  if (argc < 2)
  {
    //redir
    lexer = lexer_new(readfile(0));
  }
  else if (strcmp(argv[1],"-c") == 0)
  {
    lexer = lexer_new(argv[2]);
  }
  else
  {
    int fd = open(argv[1],0);
    lexer = lexer_new(readfile(fd));
  }
  //char *input = "word1 echo feur ; word3 word4 word5";
  struct ast *res = parse_list(&stat,lexer);
  prettyprint(res);
  lexer_free(lexer);
  return 0;
  }*/
