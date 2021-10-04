#!/bin/sh -

for a in '' `seq 0 7`
do
for b in '' `seq 0 7`
do
for c in '' `seq 0 7`
do
for z in '' 0
do
    m=$z$a$b$c &&
    test "$m" != '' &&
    m1=`printf %04o 0$m` &&
    m2=`./umask $m` &&
    if test $m1 != "$m2"
    then
        printf '%s %s %s\n' "$m" "$m1" "$m2"
    fi
done
done
done
done


for a in `seq 0 7`
do
for b in `seq 0 7`
do
for c in `seq 0 7`
do
    m=0$a$b$c &&
    m2=`./umask $m` &&
    if test $m != "$m2"
    then
        printf '%s %s\n' "$m" "$m2"
    fi
done
done
done


for a in `seq 0 7`
do
for b in `seq 0 7`
do
for c in `seq 0 7`
do
    m=0$a$b$c &&
    m2=`umask $m &&
    ./umask` &&
    if test $m != "$m2"
    then
        printf '%s %s\n' "$m" "$m2"
    fi
done
done
done
