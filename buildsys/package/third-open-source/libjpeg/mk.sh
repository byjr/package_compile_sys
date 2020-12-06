#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=v9c
cur_archive_type=tar.gz
pkg_source_url="http://www.ijg.org/files/jpegsrc"
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url.$pkg_ver.$cur_archive_type
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--target=$TARGET \
			--host=$PRO_CC_HOST \
			--build=x86_64-pc-linux-gnu \
			--prefix=$cur_prefix
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh