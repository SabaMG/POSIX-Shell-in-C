#!/bin/sh

toto=$(echo 12)
echo $toto

toto=13; echo $toto
echo $@ $* $? $1 $#

toto=titi echo $toto
echo ${toto}:${toto}
echo \$toto
echo '$toto'
dolar="echo $"
${dolar}
specials="\\;!123/$"; toto="value"
echo $specials $toto
echo $RANDOM
echo $$
echo $UID
