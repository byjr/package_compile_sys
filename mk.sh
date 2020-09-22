#!/bin/bash
START_TIME=`date +%s`
ARGS=$(getopt -o vnsc:h -l vbs,ndep,show,help,cmds: -- "$@")
if [ $? != 0 ]; then
    echo "Terminating..."
    exit 1
fi
eval set -- "$ARGS"

PRO_NAME=multi_media
active_pkgs="multiBox curl nghttp2 "
user_cmds="default"
just_show_active_pkgs=0
do_not_make_dep_pkgs=0
log_stub_path=logs/`date +%Y-%m-%d`"_make.log"
source include/top_env.sh

while true
do
	case "$1" in
		--cmds|-c) user_cmds=$2
			shift 2;;
		--show|-s) just_show_active_pkgs=1
			shift;;
		--ndep|-n) do_not_make_dep_pkgs=1
			shift;;
		--vbs|-v) log_stub_path=/proc/self/fd/1
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

if [ x"$user_cmds" == x"bclean" ];then
	rm -rf $PRO_BUILD_PATH
	exit 0
fi

for c in $user_cmds
do
	for p in $active_pkgs
	do
		${g_pkg_path_dic[$p]} $c >> $log_stub_path 2>&1
		if [ $? != 0 ];then
			break
		fi
	done
done
echo -e "\e[0;43m<<< The $0 excute completed,total spend \e[0;43m$((`date +%s`-$START_TIME)) seconds.\e[0m"	