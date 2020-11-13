#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=8420
cur_archive_type=zip
# pkg_source_url=https://prdownloads.sourceforge.net/tcl/tcl8420-src.zip
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=https://prdownloads.sourceforge.net/tcl/tcl8420-src.zip

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./unix/configure \
			--prefix=$cur_prefix \
			--enable-shared
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	def_build &&\
	cp $dst_path/generic/*.h $cur_prefix/include &&\
	cp $dst_path/unix/*.h $cur_prefix/include
}
source $TOP_DIR/include/make_com_script.sh