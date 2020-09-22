#!/bin/bash
pkg_name=curl
pkg_ver=7.68.0
cur_archive_type=tar.xz
pkg_source_url=https://curl.haxx.se/download
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
			--disable-dependency-tracking \
			--enable-ipv6 \
			--disable-nls \
			--disable-static \
			--enable-shared \
			--disable-manual \
			--disable-ntlm-wb \
			--enable-hidden-symbols \
			--with-random=/dev/urandom \
			--disable-curldebug \
			--without-polarssl \
			--disable-libcurl-option \
			--enable-threaded-resolver \
			--disable-verbose \
			--with-ssl=$PRO_STAG_PATH/usr/ \
			--with-ca-path=/etc/ssl/certs \
			--without-gnutls \
			--without-nss \
			--without-mbedtls \
			--without-wolfssl \
			--disable-ares \
			--with-libidn2 \
			--without-libssh2 \
			--without-brotli \
			--with-nghttp2 \
			--enable-cookies \
			--enable-proxy \
			--enable-dict \
			--enable-gopher \
			--enable-imap \
			--enable-ldap \
			--enable-ldaps \
			--enable-pop3 \
			--enable-rtsp \
			--enable-smb \
			--enable-smtp \
			--enable-telnet \
			--enable-tftp
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh