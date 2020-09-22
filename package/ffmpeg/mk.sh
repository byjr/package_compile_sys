#!/bin/bash
pkg_name=ffmpeg
pkg_ver=snapshot
cur_archive_type=tar.bz2
pkg_source_url=http://ffmpeg.org/releases
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

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
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh