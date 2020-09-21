#!/bin/bash
pkg_name=cppUtils
pkg_ver=1.2.0
cur_archive_type=src
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh
src_path=$PRO_APP_PATH/$pkg_name

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake -DCMAKE_INSTALL_PREFIX=$cur_prefix \
			$src_path
}

source $TOP_DIR/build/make_com_script.sh