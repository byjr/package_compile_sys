#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=1.3.0
cur_archive_type=tar.gz
pkg_source_url=https://sourceforge.net/projects/minidlna/files/minidlna/1.3.0/minidlna-1.3.0.tar.gz/download
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./configure \
			--target=$TARGET \
			--host=$PRO_CC_HOST \
			--build=x86_64-linux-gnu \
			--prefix=$cur_prefix 		
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	make && make install
}
source $TOP_DIR/include/make_com_script.sh