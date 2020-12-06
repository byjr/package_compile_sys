#!/bin/bash
START_TIME=`date +%s`
ARGS=$(getopt -o vnsc:h -l vbs,ndep,show,help,cmds: -- "$@")
if [ $? != 0 ]; then
    echo "Terminating..."
    exit 1
fi
eval set -- "$ARGS"
cur_sh_path=`dirname $(readlink -f $0)`
cd $cur_sh_path && \
if [ ! -r ../output ];then
	echo ln -sf $cur_sh_path/output ../output
	ln -sf $cur_sh_path/output ../output
fi

source include/top_env.sh

function show_useage_info(){
echo -e "
Makefile help:

    1. make :compile all pkgs in \$active_pkgs(include its depends).
    2. make \$package_name-\$user_cmds : compile \$package_name by \$user_cmds.
    3. V=1 show compile log in terminal.
    4. D=1 compile \$package_name's depthds befor compile \$package_name.
	5. xclean:
        bclean)		rm -rf \$PRO_BUILD_PATH/*
        hclean)		rm -rf \$PRO_HOST_PATH/*
        oclean)		rm -rf \$PRO_OUT_ROOT/*		
        distclean)	rm -rf \$PRO_OUT_ROOT/* && rm -rf \$PRO_DL_PATH/*
		
${0##*/} useage_info:

    -o vnsc:h -l vbs,ndep,show,help,cmds:
    
    --cmds|-c)   user_cmds=\$2.
    --show|-s)   just_show_active_pkgs.
    --ndep|-n)   do_not_make_dep_pkgs.
    --vbs|-v)    show compile log in terminal.
    --help|-h)   show useage info.
"
}

while true
do
	case "$1" in
		--cmds|-c) user_cmds=$2
			shift 2;;
		--show|-s) just_show_active_pkgs=1
			shift;;
		--ndep|-n) do_not_make_dep_pkgs=1
			shift;;
		--vbs|-v) log_file_path=$TERMINAL_PATH
			shift;;	
		--help|-h)
			show_useage_info
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
hclean)		rm -rf $PRO_HOST_PATH/*
			exit 0;;
oclean)		rm -rf $PRO_OUT_ROOT/*
			exit 0;;			
distclean)	rm -rf $PRO_OUT_ROOT/*
			rm -rf $PRO_DL_PATH/*
			exit 0;;
* )			;;
esac
make_result=0
for c in $user_cmds
do
	for p in $active_pkgs
	do
		${g_pkg_path_dic[$p]} $c >> $log_file_path 2>&1
		make_result=$?
		if [ $make_result != 0 ];then
			[ x"$log_file_path" != x"$TERMINAL_PATH" ] \
				&& tail -n 100 $log_file_path
			break
		fi
	done
done
echo make_result:$make_result
echo -e "\e[0;43m<<< The $0 excute completed,total spend \e[0;43m$((`date +%s`-$START_TIME)) seconds.\e[0m"
exit $make_result