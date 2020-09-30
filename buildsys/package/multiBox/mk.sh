#!/bin/bash
pkg_name=multiBox
pkg_ver=1.2.0
cur_archive_type=src
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
src_path=$PRO_APP_PATH/$pkg_name

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake $src_path \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_SYSTEM_NAME="Windows" \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib	\
			-DMisc_ENABLE=1 \
			-DBase64Tool_ENABLE=1 \
			-DCrcTool_ENABLE=1
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}

source $TOP_DIR/include/make_com_script.sh