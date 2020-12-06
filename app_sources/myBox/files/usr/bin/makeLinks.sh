#!/bin/sh
function help_exit(){
	echo -e "
### ${0##*/} help:

    \$1:\$target
    \S2:\$link_path_list
    eg:${0##*/} busybox-bin grep		
"
	exit 1
}
[ "$1" == "" ] && help_exit
target="$1"
[ "$2" == "" ] && help_exit
link_path_list="$2"
dst_dir=$(readlink -m `which $target`)
dst_dir=$(dirname "$dst_dir")

cd $dst_dir 
if [ $? != 0 ];then
	echo "ERR:cant find target,exit!"
	return 1
fi
for i in $link_path_list;do
	[ ! -L $i ] && ln -sf $target $i
done