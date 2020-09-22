#$1:pkg
#$2:dep_list
#$3:active_list
#$4:un_list

function up_active_list(){
	local un_list=$4
	local active_list=$3
	if [ x"$2" != x"" ];then
		for i in $2
		do
			local in_un_list=0
			for k in $4
			do
				if [ $k == $i ];then
					in_un_list=1
					break;
				fi
			done
			[ $in_un_list == 1 ] && continue
			local had_appeared=0
			for j in $active_list
			do
				if [ $j == $i ];then
					had_appeared=1
					break;
				fi
			done
			if [ $had_appeared == 0 ];then
				local subd_list=${g_dep_dic[$i]}
				un_list="$un_list $i"
				active_list=`up_active_list "$i" "$subd_list" "$active_list" "$un_list"`
			fi
		done
	fi
	for i in $active_list
	do
		if [ $1 == $i ];then
			echo $active_list
			return 0
		fi
	done
	active_list="$active_list $1"
	echo $active_list
	return 0
}
#=========================================================
#$1:pkg:包名
#$2:active_list:现有的活动包列表
function get_pkg_active_list(){
	local dep_list=${g_dep_dic[$1]}
	echo `up_active_list "$1" "$dep_list" "$2"`
}
#=========================================================
#$1:active_list:现有的活动包列表
get_list_active_list(){
	local active_list=""
	for i in $1
	do
		active_list=`get_pkg_active_list "$i" "$active_list"`
		# echo active_list:$active_list
	done
	echo $active_list
}