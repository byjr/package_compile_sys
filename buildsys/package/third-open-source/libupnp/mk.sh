#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=1.14.0
cur_archive_type=tar.bz2
pkg_source_url=https://sourceforge.net/projects/pupnp/files/pupnp/libupnp-$pkg_ver/libupnp-$pkg_ver.tar.bz2/download
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url
CC=$CC" -g"
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