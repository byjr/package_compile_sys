#!/bin/bash
pkg_name=nghttp2
pkg_ver=1.39.2
cur_archive_type=tar.gz
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh
package_source_url=https://codeload.github.com/nghttp2

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
}

source $TOP_DIR/build/make_com_script.sh