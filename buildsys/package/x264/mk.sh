#!/bin/bash
pkg_name=x264
pkg_ver=snapshot-20191217-2245
cur_archive_type=tar.bz2
pkg_source_url=http://download.videolan.org/x264/snapshots
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
			--sysroot=$TOOLCHAIN/sysroot \
			--disable-asm
			
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh