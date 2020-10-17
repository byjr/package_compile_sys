#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=STLport-5.2.1
cur_archive_type=tar.gz
pkg_source_url=https://github.com/ObiWahn/stlport/archive
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url/$pkg_ver.$cur_archive_type

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	[ -e $dst_path/$build_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	dst_path=$dst_path/build/lib
	cd $dst_path && \
	make -j $PROCESS_NUM -f gcc.mak depend && \
	make -f gcc.mak install
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh