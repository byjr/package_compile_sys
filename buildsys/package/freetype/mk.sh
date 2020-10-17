#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=2.9
cur_archive_type=tar.bz2
pkg_source_url=https://download-mirror.savannah.gnu.org/releases/$pkg_name
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--disable-cli \
			--enable-shared \
			--enable-pic \
			--disable-asm
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh