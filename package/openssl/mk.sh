#!/bin/bash
pkg_name=openssl
pkg_ver=1.1.1g
cur_archive_type=tar.gz
pkg_source_url=https://www.openssl.org/source
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		./Configure linux-armv4 \
			--prefix=$cur_prefix \
			--openssldir=$cur_prefix/etc/ssl -latomic \
			threads shared no-rc5 enable-camellia enable-mdc2 \
			no-tests no-fuzz-libfuzzer no-fuzz-afl zlib-dynamic
}

source $TOP_DIR/build/make_com_script.sh