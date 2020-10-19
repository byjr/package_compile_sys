#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=4.5.0
cur_archive_type=tar.gz
pkg_source_url=https://github.com/opencv/opencv/archive
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url/$pkg_ver.$cur_archive_type
function pkg_sync(){
	[ -e $dst_path/$sync_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	def_download && \
	cd $dst_path && \
		tar xf $src_path && \
		mv * src
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	install_path $dst_path/build
	cd $dst_path/build && \
		cmake $dst_path/src \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib
		res_info $? "[$0:$LINENO]:$FUNCNAME"
}

source $TOP_DIR/include/make_com_script.sh