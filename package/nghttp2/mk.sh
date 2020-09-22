#!/bin/bash
pkg_name=nghttp2
pkg_ver=1.39.2
cur_archive_type=tar.gz
pkg_source_url=https://github.com/nghttp2/nghttp2/releases/download/v1.39.2
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--target=$TARGET \
			--host=$PRO_CC_HOST \
			--build=x86_64-pc-linux-gnu \
			--prefix=$cur_prefix \
			--disable-dependency-tracking \
			--disable-nls \
			--disable-static \
			--enable-shared \
			--enable-lib-only
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh