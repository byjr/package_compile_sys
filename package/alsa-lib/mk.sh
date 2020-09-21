#!/bin/bash
pkg_name=alsa-lib
pkg_ver=1.2.1.2
cur_archive_type=tar.bz2
pkg_source_url=https://www.alsa-project.org/files/pub/lib
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh


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
			--with-alsa-devdir=/dev/snd \
			--with-pcm-plugins=all \
			--with-ctl-plugins=all \
			--without-versioned \
			--enable-static=no \
			--disable-python
}

source $TOP_DIR/build/make_com_script.sh