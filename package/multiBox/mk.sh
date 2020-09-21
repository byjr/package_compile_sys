#!/bin/bash
pkg_name=multiBox
pkg_ver=1.2.0
cur_archive_type=src
source $TOP_DIR/build/gloable_utils.sh
source $TOP_DIR/build/make_com_var.sh
src_path=$PRO_APP_PATH/$pkg_name

function pkg_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
		cmake $src_path \
			-DCMAKE_INSTALL_PREFIX=$cur_prefix \
			-DallPlay_ENABLE=1 \
			-DSockServ_ENABLE=1 \
			-DMisc_ENABLE=1 \
			-Dhotplug_ENABLE=1 \
			-Duartd_ENABLE=1 \
			-DBase64Tool_ENABLE=1 \
			-DPaOption_ENABLE=1 \
			-DCrcTool_ENABLE=1 \
			-Dmixer_ENABLE=1 \
			-DAppManager_ENABLE=1 \
			-Dntop_ENABLE=1 \
			-DBroadcast_ENABLE=1 \
			-DHttpCli_ENABLE=1			
}

source $TOP_DIR/build/make_com_script.sh