#!/bin/bash
pkg_name=ffmpeg
pkg_ver=snapshot
cur_archive_type=tar.bz2
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh
package_source_url=http://ffmpeg.org/releases

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME	
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--cc=$CC \
			--nm=$NM \
			--ar=$AR \
			--enable-small \
			--disable-programs \
			--disable-avdevice \
			--disable-encoders \
			--disable-muxers \
			--disable-filters \
			--cross-prefix=$PRO_CROSS_PREFIX \
			--target-os=android \
			--arch=$ARCH \
			--enable-shared \
			--disable-static \
			--enable-cross-compile			
}

source $TOP_DIR/build/make_com_script.sh