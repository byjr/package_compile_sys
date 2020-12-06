#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=v1.0.22
cur_archive_type=tar.gz
pkg_source_url=https://github.com/azatoth/minidlna/archive/$pkg_ver.$cur_archive_type
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url
DESTDIR=$cur_prefix
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./genconfig.sh		
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	make && make install
}
source $TOP_DIR/include/make_com_script.sh