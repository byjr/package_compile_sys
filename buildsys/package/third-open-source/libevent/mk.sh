#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=2.1.12
cur_archive_type=tar.gz
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=https://github.com/libevent/libevent/archive/release-$pkg_ver-stable.tar.gz
# dst_path=$dst_path/build
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib
		res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh