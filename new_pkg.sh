#!/bin/sh
cur_dir=`pwd`
src_name=DataStruct
dst_name=TestPkg

[ x"$3" != x"" ] && src_name=$3
[ x"$2" != x"" ] && dst_name=$2

function mk_source_part(){
	cd $cur_dir/app_sources
	[ $? != 0 ] && return 1
	if [ -d $dst_name ];then
		echo "ERR:source dst_name:$dst_name is exist!"
		return 1
	fi
	if [ ! -d $src_name ];then
		echo "ERR:source src_name:$src_name is't exist!"
		return 1		
	fi	
	cp -rf $src_name $dst_name
}

function mk_pkg_part(){
	cd $cur_dir/buildsys/package
	[ $? != 0 ] && return 1
	if [ -d $dst_name ];then
		echo "ERR:source dst_name:$dst_name is exist!"
		return 1
	fi
	if [ ! -d $src_name ];then
		echo "ERR:source src_name:$src_name is't exist!"
		return 1		
	fi	
	cp -rf $src_name $dst_name && \
	sed -i "s/$src_name/$dst_name/g" $dst_name/Config.in
	echo sed -i "s/$src_name/$dst_name/g" $dst_name/Config.in
}

function rm_pkg(){
	rm -rf $cur_dir/app_sources/$dst_name
	rm -rf $cur_dir/buildsys/package/$dst_name
}

case $1 in
	rmk|remake)
		echo "INF:try to remake pkg $dst_name base on $src_name ..."
		rm_pkg
		mk_pkg_part
		mk_source_part
		;;		
	mk|make)
		echo "INF:try to make pkg $dst_name base on $src_name ..."
		mk_pkg_part
		mk_source_part
		;;
	rm|remove)
		echo "INF:try to remove pkg $dst_name ..."
		rm_pkg
		;;
	*)
		echo "ERR:unsurport cmd!!"
		;;
esac		

