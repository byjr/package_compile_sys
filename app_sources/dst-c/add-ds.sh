#!/bin/bash
work_dir=`dirname $(readlink -m $0)`
new_ds_name=$1
new_hit_name=$2

read -d '' usage_info <<-_ACEOF
### ${0##*/} help:

    \$1:数据结构的名字，必须指定
    \S2:命中执行该数据结构的key,若未指定则同上
    eg:${0##*/} DoubleQueue dque	
_ACEOF

if [ x"$new_ds_name" == x"" ];then
	printf "\n%s\n\n" "$usage_info"
	exit 1
fi

add_new_line_ctt='int '$new_ds_name'_main(int argc,char* argv[]);'
add_contex_ctt='\\tmainMap["'$new_hit_name'"] = '$new_ds_name'_main;'

[ x"$new_hit_name" == x"" ] && new_hit_name="$new_ds_name"

function add_main_ctt(){
	cd $work_dir
	[ $? != 0 ] && return 1
	grep "$new_ds_name" main.cpp
	[ $? == 0 ] && return 1
	sed -i  "/<unordered_map>/ a$add_new_line_ctt" main.cpp && \
	sed -i "/mainMap;/ a$add_contex_ctt" main.cpp
}
function make_dst_archive(){
	cd $work_dir
	rm -rf $new_ds_name
	cp -r LinkStack $new_ds_name
	sub_files=`find $new_ds_name -type f`
	new_file_name=""
	for i in $sub_files
	do
		sed -i "s/LinkStack/$new_ds_name/g" $i
		[ $? != 0 ] && return 1
		new_file_name=`echo $i | sed "s/LinkStack/$new_ds_name/g"`
		mv $i $new_file_name
	done	
}
add_main_ctt &&\
make_dst_archive


