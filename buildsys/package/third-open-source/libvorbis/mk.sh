#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=1.3.6
cur_archive_type=tar.xz
pkg_source_url="http://downloads.xiph.org/releases/vorbis"
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
			--prefix=$cur_prefix \
			--disable-gtk-doc \
			--disable-gtk-doc-html \
			--disable-doc \
			--disable-docs \
			--disable-documentation \
			--with-xmlto=no \
			--with-fop=no \
			--disable-dependency-tracking \
			--enable-ipv6 \
			--disable-nls \
			--disable-static \
			--enable-shared \
			--disable-oggtest
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh