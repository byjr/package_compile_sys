source include/gloable_utils.sh
#local var

#export const var

export TOP_DIR=`pwd`
export PRO_OUT_ROOT=$TOP_DIR/output/$PRO_NAME
export PRO_BUILD_PATH=$PRO_OUT_ROOT/build
export PRO_HOST_PATH=$PRO_OUT_ROOT/host
export PRO_STAG_PATH=$PRO_OUT_ROOT/staging
export PRO_STAG_USR_PATH=$PRO_STAG_PATH/usr
export PRO_TARGET_PATH=$PRO_OUT_ROOT/target
export PRO_TARGET_USR_PATH=$PRO_TARGET_PATH/usr
export PRO_STUB_PATH=$PRO_OUT_ROOT/stub
export PRO_LOG_PATH=$PRO_OUT_ROOT/logs
export PRO_DL_PATH=$TOP_DIR/dl
export PRO_DL_TMP_PATH=$PRO_DL_PATH/.tmp
export PRO_DL_EXT_PATH=$PRO_DL_PATH/.ext
export PRO_APP_PATH=$TOP_DIR/../app_sources

export state_list="sync patch config build install final"

#export calc var
CUR_NSSID_PID=`cat /proc/self/stat | awk '{print $6}'`
export PROCESS_NUM=`cat /proc/cpuinfo | grep process | wc -l`
export TERMINAL_PATH=`readlink -v /proc/$CUR_NSSID_PID/fd/1`
export state_file_list=`list_add_fix "$state_list" .state_ _done`
export redo_cmd_list=`list_add_fix "$state_list" re`

export patch_sfile=`get_file_by_state patch`
export sync_sfile=`get_file_by_state sync`
export build_sfile=`get_file_by_state build`
export config_sfile=`get_file_by_state config`
export install_sfile=`get_file_by_state install`
export final_sfile=`get_file_by_state final`

#excute segment
install_path_list $PRO_BUILD_PATH $PRO_HOST_PATH $PRO_TARGET_USR_PATH \
	 $PRO_STUB_PATH $PRO_DL_EXT_PATH $PRO_DL_TMP_PATH $PRO_LOG_PATH

declare -A g_pkg_path_dic
build_pkg_path_dic package mk.sh
[ $? == 0 ] || exit 1

declare -A g_dep_dic
source_list_in_spec package Config.in