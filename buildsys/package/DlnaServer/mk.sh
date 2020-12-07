#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=1.1.0
cur_archive_type=src
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
src_path=$PRO_APP_PATH/$pkg_name
PROCESS_NUM=1
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake $src_path \
			-DPKG_NAME=$pkg_name \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib	\
			-DBUILD_DLNA=ON \
			-DSOURCE_DIR=$src_path
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh