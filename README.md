### This is our POSIX SHELL in C.

to run the shell, you need to compile it first. The compilation process is as follows:

First make sure you have autoconf installed on your system. If not, you can install it by running the following command:

### brew install autoconf

then run the following commands in the terminal:

### autoreconf --install
### ./configure
### make

it creates you the binary in src/
you can run it with the -c flag and after under quote your command

for example

### ./src/42sh -c "echo -n Hello"

you can do more complex commands as well, it takes in account pipelines, logical opperators, lists, some redirections, etc...

### Cleanning

to clean everything, you can run the following commands: 

### make clean
### make maintainer-clean
### rm Makefile.in ar-lib aclocal.m4 compile configure configure\~ depcomp install-sh missing