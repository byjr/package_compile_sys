#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=0.6.21
cur_archive_type=tar.bz2
pkg_source_url="https://qa.debian.org/watch/sf.php/libexif"
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh

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