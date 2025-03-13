#!/bin/sh

# This is a comment and should not be printed

echo 12 > test ;
cat < test ;

# Append
echo "3" >> test
cat < test

echo 2 2> test
echo 3 2>> test

cat test

if echo 12; then echo 14; fi > test

echo "echo 12" > input
cat < input > test
cat < test | cat

echo "toto" >| test
cat test

rm test input
