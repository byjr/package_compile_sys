#!/bin/bash
work_dir=`dirname $(readlink -m $0)`/src

function help_exit(){
	echo -e "
### ${0##*/} help:
    \$1:源代码模块的名字，默认值：MyTmplate
    \$2:目标代码模块的名字，必须指定
    \S3:命中执行该代码模块的key,若未指定则同上
    eg:${0##*/} MyTmplate DoubleQueue dque
"
	exit 1
}


src_name="MyTmplate"
[ "$1" != "" ] && src_name="$1"
dst_name="$2"
[ "$3" != "" ] && hit_key="$3"
[ "$dst_name" == "" ] && help_exit

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
	cp -rf $src_name $dst_name
	local sub_files=`find $dst_name -type f`
	local new_file_name=""
	for i in $sub_files
	do
		sed -i "s/$src_name/$dst_name/g" $i
		[ $? != 0 ] && return 1
		new_file_name=`echo $i | sed "s/$src_name/$dst_name/g"`
		mv $i $new_file_name
	done	
}
add_main_ctt &&\
make_dst_archive