#!/bin/bash
#if you start this application,after every ctrl+x or ctrl+c,the application runs an astyle on the content of the clipboard then put it in the clipboard again.
#sot,az xselhez kimasolni se kell,eleg kijelolni.
prev=$(xsel)
while true;
do
act=$(xsel)
if [ "$act" != "$prev" ];
then
prev="$act"
echo $act >/tmp/input
astyle --style=gnu -s2 --pad=oper -C -N -K /tmp/input >output
cat /tmp/input
cat /tmp/input | xclip -in -selection c
fi
done
