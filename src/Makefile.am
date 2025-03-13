# define the subdirectories
SUBDIRS = ast \
      parser \
			lexer \
			eval

bin_PROGRAMS = 42sh

42sh_SOURCES = 42sh.c

42sh_CPPFLAGS = -I%D%

42sh_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic

42sh_LDADD =  \
        eval/libeval.a  \
       parser/libpars.a \
       lexer/liblex.a \
       ast/libast.a
