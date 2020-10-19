#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=2.0.12
cur_archive_type=tar.gz
pkg_source_url=https://www.libsdl.org/release
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--enable-shared
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
https://www.libsdl.org/release/SDL2-2.0.12.tar.gz
source $TOP_DIR/include/make_com_script.sh