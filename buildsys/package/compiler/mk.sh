#!/bin/bash
pkg_name=android-ndk
pkg_ver=r21d-linux-x86_64
cur_archive_type=zip
pkg_source_url=https://dl.google.com/android/repository
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
dst_path=$PRO_HOST_PATH/$pkg_all_name

function pkg_build(){
	[ -e $dst_path/$build_sfile ]  && return 0
	func_info $0 $LINENO $FUNCNAME
}
function pkg_install(){
	[ -e $dst_path/$install_sfile ]  && return 0
	func_info $0 $LINENO $FUNCNAME
	if [ ! -L $PRO_STAG_PATH ];then
		echo ln -sT $TOOLCHAIN/sysroot $PRO_STAG_PATH
		ln -sT $TOOLCHAIN/sysroot $PRO_STAG_PATH
	fi
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}

source $TOP_DIR/include/make_com_script.sh