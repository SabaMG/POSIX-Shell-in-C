#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 500
#define PATH_MAX 1024

#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_word(char *word, int escape_flag)
{
    for (size_t j = 0; word[j]; j++)
    {
        if (escape_flag == 0)
            printf("%c", word[j]);
        else
        {
            if (word[j] == '\\')
            {
                j++;
                switch (word[j])
                {
                case '\\':
                    printf("\\");
                    break;
                case 'n':
                    printf("\n");
                    break;
                case 't':
                    printf("\t");
                    break;
                default:
                    printf("\\%c", word[j]);
                    break;
                }
            }
            else
            {
                printf("%c", word[j]);
            }
        }
    }
}

int echo(char **words, size_t len)
{
    int newline_flag = 1;
    int escape_flag = 0; // -E : \n => \n

    size_t i = 1;

    while (words[i] && words[i][0] == '-') // parsing flags
    {
        for (size_t j = 1; words[i][j]; j++)
        {
            switch (words[i][j])
            {
            case 'n':
                newline_flag = 0;
                break;
            case 'e':
                escape_flag = 1;
                break;
            case 'E':
                escape_flag = 0;
                break;
            default:
                errx(1, "echo error");
            }
        }
        i++;
    }
    if (words[i] && len > 0)
    {
        print_word(words[i], escape_flag);
        i++;
        while (words[i])
        {
            printf(" ");
            print_word(words[i], escape_flag);
            i++;
        }
    }
    if (newline_flag == 1)
        printf("\n");
    return 0;
}

char *getpwd(void)
{
    char *pwd = getenv("PWD");
    if (!pwd)
    {
        pwd = calloc(1, 100);
        getcwd(pwd, 100);
        setenv("PWD", pwd, 1);
        // printf("cwd : %s\n",pwd);
        return pwd;
    }
    return strdup(pwd);
}

int cd(char **words, size_t len)
{
    char *oldpwd = getpwd();

    if (len == 1)
    {
        char *home = getenv("HOME");
        if (!home || strcmp(home, "") == 0)
            errx(1, "cd: HOME not set");

        if (chdir(home) == -1)
            errx(1, "cd: can't go to HOME");
    }
    else
    {
        char *path = words[1];

        if (chdir(path) == -1)
            errx(1, "cd: can't go to %s", path);
        else
        {
            char *pwd = getpwd();
            char *temp = calloc(1, strlen(path) + strlen(pwd) + 2);
            strcat(temp, pwd);
            strcat(temp, "/");
            strcat(temp, path);
            setenv("PWD", temp, 1);
            free(temp);
            free(pwd);
        }
    }

    if (setenv("OLDPWD", oldpwd, 1) == -1)
    {
        free(oldpwd);
        errx(1, "getcwd failed after chdir");
    }
    char *newpwd = getpwd();

    if (setenv("PWD", newpwd, 1) == -1)
    {
        free(newpwd);
        free(oldpwd);
        errx(1, "getcwd failed after chdir");
    }
    free(oldpwd);
    free(newpwd);
    return 0;
}
