lib_LIBRARIES = libeval.a

libeval_a_SOURCES =  \
  exec_tree.c \
	builtins.c \
	pipe.c \
	redir.c \
	builtins.h \
  exec_tree.h \
  redir.h  
  

libeval_a_CPPFLAGS = -I$(top_srcdir)/src

libeval_a_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic


noinst_LIBRARIES = libeval.a
