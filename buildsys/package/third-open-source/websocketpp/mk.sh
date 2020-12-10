#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=0.8.2
cur_archive_type=tar.gz
pkg_source_url=https://github.com/zaphoyd/websocketpp/archive/$pkg_ver.tar.gz
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url
# PROCESS_NUM=1
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake ./ \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib \
			-DBUILD_EXAMPLES=TRUE \
			-DBUILD_TESTS=ON \
			-DCMAKE_COMPILER_IS_GNUCXX=TRUE
		res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh