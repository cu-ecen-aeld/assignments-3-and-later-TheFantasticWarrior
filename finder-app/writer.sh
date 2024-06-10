if [[ ! $1 || ! $2 ]] ; then
    exit 1
fi
echo $1
dir=`sed '1{s/[^/]*$//}' <<<"$1"`
echo $dir
mkdir -p $dir
touch $1
echo $2>$1
