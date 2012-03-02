#!/bin/bash
#egy paraméter van:a fájlnév,a többi guis
filename=$1
#cat $filename|egrep ".*::.*\(.*\)$"|xargs -I a echo -n " \"a\" \"a\" "
if [ -z "$filename"  ]
then
#filename=$(kdialog --getopenurl /home/adam/zwa/work/bap-dev-abakai-all/bap )
filename=$(kdialog --getopenurl $PWD |cut -c 8- )
fi
mode=$2
if [ -z "$mode"  ]
then
mode=$(kdialog --geometry 800x600 --menu "valassz modot" "function_enter" "function_enter" "clean" "clean" "cout_specific_word" "cout_specific_word"  )
fi
echo $mode

#echo kdialog --geometry 800x600 --checklist valassz $(cat $filename|egrep "^[^ ]*::.*\(.*\)$"|xargs -I a echo -n " \"a\" \"a\" off ")
if [ "$mode" = "clean" ]
then
# sed -i "s/^.*bakaiszkript_to_make_function_prints.*$//g" $filename
 sed -i "/^.*bakaiszkript_to_make_function_prints.*$/{D}" $filename
exit
fi


if [ "$mode" = "function_enter" ]
then
command=$(echo kdialog --geometry 800x600 --checklist valassz $(cat $filename|egrep "^[^ ]*::.*\(.*\)$"|xargs -I a echo -n " \"a\" \"a\" off ") )
echo $command

retval=$(echo $command|bash)
echo $retval|sed "s/\"/\n/g"|uniq|while read i
do
sed -i "/${i}/{/\([^ ]*\)::\(.*)\)$/{N; s#\([^ ]*\)::\(.*)\)\n{#\1::\2\n{\nstd::cout\<\<\"\1::\2\\\n\";//bakaiszkript_to_make_function_prints#}}" $filename
done
exit
fi

if [ "$mode" = "clean" ]
then
# sed -i "s/^.*bakaiszkript_to_make_function_prints.*$//g" $filename
word=$(kdialog --inputbox " " " ")
sed  "/${word}/{s/^.*$/a/}" $filename
exit
fi



