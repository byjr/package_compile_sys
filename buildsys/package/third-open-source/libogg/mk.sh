#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=1.3.4
cur_archive_type=tar.xz
pkg_source_url=https://downloads.xiph.org/releases/ogg
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

source $TOP_DIR/include/make_com_script.sh