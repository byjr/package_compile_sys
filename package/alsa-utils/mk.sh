#!/bin/bash
pkg_name=alsa-utils
pkg_ver=1.2.1
cur_archive_type=tar.bz2
pkg_source_url=ftp://ftp.alsa-project.org/pub/utils
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh

export LIBPTHREAD=""
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME	
	cd $dst_path && \
		./configure \
			--target=$TARGET \
			--host=$PRO_CC_HOST \
			--build=x86_64-pc-linux-gnu \
			--prefix=$cur_prefix \
			--disable-dependency-tracking \
			--disable-nls \
			--disable-xmlto \
			--disable-rst2man \
			--disable-alsaloop \
			--disable-alsamixer \
			--with-systemdsystemunitdir=$cur_prefix/lib/systemd/system \
			--with-udev-rules-dir=$cur_prefix/lib/udev/rules.d \
			--disable-bat
}

source $TOP_DIR/build/make_com_script.sh