#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=1.74.0
cur_archive_type=tar.gz
pkg_source_url=https://dl.bintray.com/boostorg/release/$pkg_ver/source/boost_1_74_0.tar.bz2
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url
# dst_path=$dst_path/build
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./bootstrap.sh \
			--prefix=$cur_prefix \
			--with-bjam=$dst_path/Jamroot \
			--with-toolset=x86_64-linux-gnu \
			--show-libraries \
			--with-icu \
			--with-python=/usr/bin/python \
			--with-python-version=3.5
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	[ -e $dst_path/$build_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
	./b2 -j$PROCESS_NUM && \
	./b2 install
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh