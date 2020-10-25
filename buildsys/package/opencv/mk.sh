#!/bin/bash
pkg_name=${0%/*}
pkg_name=${pkg_name##*/}
pkg_ver=4.5.0
cur_archive_type=tar.gz
pkg_source_url=https://github.com/opencv/opencv/archive
source $TOP_DIR/include/gloable_utils.sh
source $TOP_DIR/include/make_com_var.sh
pkg_source_uri=$pkg_source_url/$pkg_ver.$cur_archive_type

function pkg_sync(){
	[ -e $dst_path/$sync_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	def_download && \
	cd $dst_path && \
		tar xf $src_path && \
		mv * src
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}

real_config="
-DCMAKE_BUILD_TYPE=RELEASE \
-DBUILD_opencv_apps=OFF \
-DBUILD_DOCS=OFF \
-DBUILD_PERF_TESTS=OFF \
-DBUILD_TESTS=OFF \
-DBUILD_WITH_DEBUG_INFO=OFF
"
function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAMEs 
	install_path $dst_path/build
	cd $dst_path/build && \
		cmake $dst_path/src \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DCMAKE_INCLUDE_PATH=$PRO_STAG_USR_PATH/include \
			-DCMAKE_LIBRARY_PATH=$PRO_STAG_USR_PATH/lib \
			$real_config
		res_info $? "[$0:$LINENO]:$FUNCNAME"
}
function pkg_build(){
	[ -e $dst_path/$build_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path/build && \
	make  && \
	make install
	res_info $? "[$0:$LINENO]:$FUNCNAME"
}
source $TOP_DIR/include/make_com_script.sh