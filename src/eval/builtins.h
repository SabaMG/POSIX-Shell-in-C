#ifndef BUILTINS_H
#define BUILTINS_H

#include <stddef.h>

int echo(char **words, size_t len);

int cd(char **words, size_t len);

char *getpwd(void);

#endif /* ! BUILTINS_H */
