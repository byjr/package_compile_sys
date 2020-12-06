#!/bin/sh
cur_dir=`pwd`
src_name=DataStruct
dst_name=TestPkg

[ x"$3" != x"" ] && src_name=$3
[ x"$2" != x"" ] && dst_name=$2

src_source_path=`find $cur_dir/app_sources -type d -name "$src_name"`
src_package_path=`find $cur_dir/buildsys/package -type d -name "$src_name"`
dst_source_path=`find $cur_dir/app_sources -type d -name "$dst_name"`
dst_package_path=`find $cur_dir/buildsys/package -type d -name "$dst_name"`
function mk_source_part(){
	if [ "$src_source_path" == "" ];then
		echo "[${0##*/}:$LINENO:$FUNCNAME]:""WAR:source src_name:$src_name is't exist!"
		return 1		
	fi	
	if [ "$dst_source_path" != "" ];then
		echo "[${0##*/}:$LINENO:$FUNCNAME]:""WAR:source dst_source_path:$dst_source_path is exist in $dst_source_path !"
		return 1
	fi
	dst_source_path="${src_source_path%/*}/$dst_name"
	cp -rf "$src_source_path" "$dst_source_path"
}

function mk_pkg_part(){
	if [ "$src_package_path" == "" ];then
		echo "[${0##*/}:$LINENO:$FUNCNAME]:""ERR:package src_name:$src_name is exist!"
		return 1		
	fi
	
	if [ "$dst_package_path" != "" ];then
		echo "[${0##*/}:$LINENO:$FUNCNAME]:""ERR:package dst_name:$dst_name is exist in $dst_package_path !"
		return 1
	fi
	dst_package_path="${src_package_path%/*}/$dst_name"
	cp -rf $src_package_path $dst_package_path && \
	sed -i "s/$src_name/$dst_name/g" $dst_package_path/Config.in
	echo sed -i "s/$src_name/$dst_name/g" $dst_package_path/Config.in
}

function rm_pkg(){
	rm -rf $dst_source_path
	rm -rf $dst_package_path
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

