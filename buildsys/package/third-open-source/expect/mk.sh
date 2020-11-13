#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=5.43.0
cur_archive_type=tar.gz
pkg_source_url=https://ftp.osuosl.org/pub/blfs/conglomeration/expect
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
export CFLAGS="-I $PRO_STAG_USR_PATH/include"
export CPPFLAGS=$CFLAGS

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--prefix=$cur_prefix \
			--enable-shared \
			--with-tcl=$PRO_STAG_USR_PATH/bin
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}

source $TOP_DIR/include/make_com_script.sh