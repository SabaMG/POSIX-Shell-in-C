( toto=12; export toto );
echo $toto
unset toto

toto=13;
export toto
echo $toto
unset $toto
echo $toto;
