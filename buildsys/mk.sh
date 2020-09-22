#!/bin/bash
START_TIME=`date +%s`
ARGS=$(getopt -o vnsc:h -l vbs,ndep,show,help,cmds: -- "$@")
if [ $? != 0 ]; then
    echo "Terminating..."
    exit 1
fi
eval set -- "$ARGS"

PRO_NAME=multi_media
active_pkgs="multiBox curl nghttp2 alsa-utils"
user_cmds="default"
just_show_active_pkgs=0
do_not_make_dep_pkgs=0

cur_sh_path=`dirname $(readlink -f $0)`
cd $cur_sh_path 
[ -L ../output ] || ln -sf $cur_sh_path/output ../output

source include/top_env.sh
log_file_path=$PRO_LOG_PATH/`date +%Y-%m-%d`"_make.log"

while true
do
	case "$1" in
		--cmds|-c) user_cmds=$2
			shift 2;;
		--show|-s) just_show_active_pkgs=1
			shift;;
		--ndep|-n) do_not_make_dep_pkgs=1
			shift;;
		--vbs|-v) log_file_path=/proc/self/fd/1
			shift;;	
		--help|-h)
			echo "  ${0##*/} help:"
			echo "    --cmds|-c :[user_cmds]"
			echo "    --show|-s):just_show_active_pkgs"
			exit 0;;
		--) shift ;break ;;
		*) 	echo "Internal error!"
			exit 1;;
	esac
done

[ $# -gt 0 ] && active_pkgs="$@"

active_pkgs=`get_last_level_name $active_pkgs`

if [ $do_not_make_dep_pkgs == 0 ];then
	source include/get_deps.sh
	active_pkgs=`get_list_active_list "$active_pkgs"`
fi

if [ $just_show_active_pkgs == 1 ];then
	echo active_pkgs:$active_pkgs
	exit 0
fi

case $user_cmds in
bclean)		rm -rf $PRO_BUILD_PATH/*
			exit 0;;
cclean)		rm -rf $PRO_BUILD_PATH/*
			exit 0;;			
distclean)	rm -rf $PRO_OUT_ROOT/*
			rm -rf $PRO_DL_PATH/*
			exit 0;;
* )			;;
esac

for c in $user_cmds
do
	for p in $active_pkgs
	do
		${g_pkg_path_dic[$p]} $c >> $log_file_path 2>&1
		if [ $? != 0 ];then
			break
		fi
	done
done
echo -e "\e[0;43m<<< The $0 excute completed,total spend \e[0;43m$((`date +%s`-$START_TIME)) seconds.\e[0m"	