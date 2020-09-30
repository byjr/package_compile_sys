#!/bin/bash
pkg_name=mingw-w64
pkg_ver=v5.3.1
cur_archive_type=zip
pkg_source_url=https://iweb.dl.sourceforge.net/project/mingw-w64/mingw-w64/mingw-w64-release
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
dst_path=$PRO_HOST_PATH/$pkg_all_name

function pkg_ex_make(){
	if [ ! -d $PRO_STAG_USR_PATH ];then
		install_path $PRO_STAG_USR_PATH
		func_info $0 $LINENO $FUNCNAME
	fi	
}
function pkg_ex_clean(){
	func_info $0 $LINENO $FUNCNAME
}
source $TOP_DIR/include/make_com_script.sh