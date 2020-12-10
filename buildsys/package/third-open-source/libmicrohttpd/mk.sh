#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=0.9.71
cur_archive_type=tar.gz
pkg_source_url=https://mirrors.aliyun.com/gnu/libmicrohttpd
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
# pkg_source_uri=$pkg_source_url/"$pkg_name"_"$pkg_ver"."$cur_archive_type"
# dst_path=$dst_path/build
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh