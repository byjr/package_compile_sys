#!/bin/bash
START_TIME=`date +%s`
ARGS=$(getopt -o nsc:h -l ndep,show,help,cmds: -- "$@")
if [ $? != 0 ]; then
    echo "Terminating..."
    exit 1
fi
eval set -- "$ARGS"

source build/top_env.sh

active_pkgs="multiBox curl nghttp2 "
user_cmds="default"
just_show_active_pkgs=0
do_not_make_dep_pkgs=0

while true
do
	case "$1" in
		--cmds|-c) user_cmds=$2
			shift 2;;
		--show|-s) just_show_active_pkgs=1
			shift 1;;
		--ndep|-n) do_not_make_dep_pkgs=1
			shift 1;;					
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
	source build/get_deps.sh
	active_pkgs=`get_list_active_list "$active_pkgs"`
fi

if [ $just_show_active_pkgs == 1 ];then
	echo active_pkgs:$active_pkgs
	exit 0
fi

for c in $user_cmds
do
	for p in $active_pkgs
	do
		${g_pkg_path_dic[$p]} $c
		if [ $? != 0 ];then
			err_msg make $c $p
			break
		fi
	done
done
echo -e "\e[0;43m<<< The $0 excute completed,total spend \e[0;43m$((`date +%s`-$START_TIME)) seconds.\e[0m"	