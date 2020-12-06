#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=0.2.4
cur_archive_type=tar.bz2
pkg_source_url=http://libdlna.geexbox.org/releases
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--with-ffmpeg-dir=$PRO_STAG_USR_PATH \
			--cross-prefix=
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	def_build &&\
		install_path  $cur_prefix/bin ;\
		cp -raf $dst_path/test-libdlna $cur_prefix/bin/
}
source $TOP_DIR/include/make_com_script.sh