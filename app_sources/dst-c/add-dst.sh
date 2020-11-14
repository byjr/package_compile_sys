#!/bin/bash
work_dir=`dirname $(readlink -m $0)`
dst_name=$1
hit_key=$2

read -d '' usage_info <<-_ACEOF
### ${0##*/} help:

    \$1:数据结构的名字，必须指定
    \S2:命中执行该数据结构的key,若未指定则同上
    eg:${0##*/} DoubleQueue dque	
_ACEOF

if [ x"$dst_name" == x"" ];then
	printf "\n%s\n\n" "$usage_info"
	exit 1
fi

[ x"$hit_key" == x"" ] && hit_key="$dst_name"
add_new_line_ctt='int '$dst_name'_main(int argc,char* argv[]);'
add_contex_ctt='\\tmainMap["'$hit_key'"] = '$dst_name'_main;'



function add_main_ctt(){
	cd $work_dir
	[ $? != 0 ] && return 1
	grep "$dst_name" main.cpp
	[ $? == 0 ] && return 1
	sed -i  "/<unordered_map>/ a$add_new_line_ctt" main.cpp && \
	sed -i "/mainMap;/ a$add_contex_ctt" main.cpp
}
function make_dst_archive(){
	cd $work_dir
	rm -rf $dst_name
	cp -r LinkStack $dst_name
	local sub_files=`find $dst_name -type f`
	local new_file_name=""
	for i in $sub_files
	do
		sed -i "s/LinkStack/$dst_name/g" $i
		[ $? != 0 ] && return 1
		new_file_name=`echo $i | sed "s/LinkStack/$dst_name/g"`
		mv $i $new_file_name
	done	
}
add_main_ctt &&\
make_dst_archive


