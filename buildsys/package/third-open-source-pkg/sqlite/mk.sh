#!/bin/bash
pkg_name=sqlite-autoconf
pkg_ver=3300100
cur_archive_type=tar.gz
pkg_source_url=https://www.sqlite.org/2019
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
			--disable-static-shell \
			--enable-threadsafe \
			--disable-editline \
			--disable-readline
	res_info $? "[$0:$LINENO]:$FUNCNAME"			
}

source $TOP_DIR/include/make_com_script.sh