#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=0.3.1
cur_archive_type=tar.xz
pkg_source_url=http://freedesktop.org/software/pulseaudio/$pkg_name
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	./autogen.sh && \
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--enable-shared
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
# function pkg_build(){
	# def_build $@
	# [ $? != 0 ] && exit 1
	# rm -rf $cur_prefix/include/webrtc
	# mv $cur_prefix/include/webrtc_audio_processing/webrtc $cur_prefix/include/ && \
	# rm -rf $cur_prefix/include/webrtc_audio_processing
# }
source $TOP_DIR/include/make_com_script.sh