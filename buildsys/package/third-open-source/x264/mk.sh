#!/bin/bash
pkg_name=x264
pkg_ver=20200819
cur_archive_type=tar.xz
pkg_source_url=https://ftp.osuosl.org/pub/blfs/conglomeration/$pkg_name
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
			--cross-prefix=$PRO_CROSS_PREFIX \
			--disable-asm
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}

source $TOP_DIR/include/make_com_script.sh