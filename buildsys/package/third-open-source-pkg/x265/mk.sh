#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=3.2
cur_archive_type=tar.gz
pkg_source_url=http://get.videolan.org/$pkg_name
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url/"$pkg_name"_"$pkg_ver"."$cur_archive_type"
dst_path=$dst_path/build
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake ../source \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib
		res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh