#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "eval/exec_tree.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main(int argc, char *argv[])
{
    enum parser_status stat;
    char *rf = NULL;
    struct lexer *lexer;
    if (argc < 2)
    {
        // redir
        rf = readfile(0);
        lexer = lexer_new(rf);
    }
    else if (strcmp(argv[1], "-c") == 0)
    {
        lexer = lexer_new(argv[2]);
    }
    else
    {
        int fd = open(argv[1], 0);
        rf = readfile(fd);
        lexer = lexer_new(rf);
    }

    struct token t;
    while (lexer_peek(lexer).type == EOL)
    {
        t = lexer_pop(lexer);
        free(t.value);
    }

    if (lexer_peek(lexer).type == TEOF)
    {
        free(rf);
        free(lexer);
        return 0;
    }
    struct ast *res = parse_list(&stat, lexer);
    // prettyprint(res);

    // ----TESTS A LA MANO
    /*
    struct ast_list * res = initlist();

    struct ast_and_or * andor = initandor();
    andor->oper = 2;//and
    struct ast_cmd * cmd1 = initcommand();
    cmd1->base.type = AST_COMMAND;


    free(cmd1->words);
    cmd1->words = malloc(3 * sizeof(char *));
     cmd1->words[0] = "echo";
    cmd1->words[1] = "hello";
    cmd1->words[2] = NULL;

    andor->child = &(cmd1->base);

    //-- cmd2
    struct ast_cmd * cmd2 = initcommand();
    cmd2->base.type = AST_COMMAND;


    free(cmd2->words);
    cmd2->words = malloc(3 * sizeof(char *));
    cmd2->words[0] = "echo";
    cmd2->words[1] = "world";
    cmd2->words[2] = NULL;

    andor->next = &(cmd2->base);

    res->child = &(andor->base);
  */

    int rep = exec_ast(res);
    free_ast(res);
    // free(res);
    lexer_free(lexer);

    if (rf)
        free(rf);
    return rep ? 0 : 1;
}
