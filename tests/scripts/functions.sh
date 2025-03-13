foo() {
    export toto=12;
}
foo
echo $toto

bar() {
    unset toto;
}
bar
echo $toto
bar () {
    to() {
        echo 14;
    }
    echo $1
    echo $2
    echo $-1
}
bar 1
to
unset -f to
to
