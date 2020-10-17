#!/bin/bash
pkg_name=ffmpeg
pkg_ver=4.3
cur_archive_type=tar.bz2
pkg_source_url=http://ffmpeg.org/releases
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

rt_func_config="\
--enable-ffmpeg \
--enable-ffplay \
--enable-ffprobe \
"

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--cc=$CC \
			--nm=$NM \
			--ar=$AR \
			$rt_func_config \
			--cross-prefix=$PRO_CROSS_PREFIX \
			--target-os=linux \
			--arch=$ARCH \
			--enable-shared \
			--disable-static \
			--disable-x86asm \
			--enable-cross-compile
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh