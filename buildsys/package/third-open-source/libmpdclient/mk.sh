#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=2.19
cur_archive_type=tar.xz
pkg_source_url=https://www.musicpd.org/download/libmpdclient/2
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
# pkg_source_uri=$pkg_source_url/"$pkg_name"_"$pkg_ver"."$cur_archive_type"
# dst_path=$dst_path/build
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	install_path $cur_prefix	
	cd $dst_path && \
		meson . $cur_prefix
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	[ -e $dst_path/$build_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
	ninja -j $PROCESS_NUM -C $cur_prefix && \
	ninja -C $cur_prefix install
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh