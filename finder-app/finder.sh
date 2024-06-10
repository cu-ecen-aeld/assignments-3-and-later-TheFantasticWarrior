if [[ ! -d $1 || ! $2 ]] ; then
    exit 1
fi
outputs=$(grep -hRc $1 -e $2)
sum=($(awk '{n+=$0}{if ($0!=0){f++}} END {print n,f}' <<<"$outputs"))
echo "The number of files are ${sum[1]} and the number of matching lines are ${sum[0]}"
