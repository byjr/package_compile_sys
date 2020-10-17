#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=plugins-1.7.0
cur_archive_type=tar.gz
pkg_source_url=https://ftp.osuosl.org/pub/blfs/conglomeration/$pkg_name
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./autogen.sh && \
		./configure \
			--prefix=$cur_prefix \
			--enable-shared 
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}

source $TOP_DIR/include/make_com_script.sh