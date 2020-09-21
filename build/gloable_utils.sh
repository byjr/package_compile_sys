function func_info(){
	echo -e "\e[0;32m>>>[$1:$2]:$3 ...\n\e[0m"
}
function tip_msg(){
	echo -e "\e[1;32m>>>>>> $@ done!!!\n\e[0m"
}
function err_msg(){
	echo -e "\e[1;31m>>>>>> $@ failed!!!!\n\e[0m"
}
function fifo_show(){
	echo $@ >> /tmp/1.fifo
}
function install_path(){
	if [ ! -d  ${1%/*} ];then
		install_path ${1%/*}
	fi
	if [ ! -d  $1 ];then
		mkdir $1
	fi
}
function install_path_list(){
	for i in $@
	do
		install_path $i
		if [ $? != 0 ];then
			echo install_path $i FAILED!
			return 1
		fi
	done
}
function is_dir_empty(){
	cd $1 && \
	[ $(ls | wc -l) == 0 ] && echo 0 || echo 1
}
function res_check(){
	local res_code=$1
	local pkg_name=$2
	[ $res_code == 0 ] && \
		tip_msg $pkg_name activite excute || \
		err_msg $pkg_name activite excut
	return $res_code
}
function get_sub_list(){
	local a=($1)
	local n=${#a[*]}
	for i in ${!a[*]}
	do
		if [ x"${a[$i]}" == x"$2" ];then
			echo "${a[*]}"
			return 0
		fi
		unset a[$i]
	done		
}
function list_add_fix(){
	local a=($1)
	local oa
	for i in ${!a[*]}
	do
		oa[$i]=$2${a[$i]}$3
	done
	echo ${oa[*]}
}

#====================================================
#function:从目标列表中命中与原列表指定项位置相同的项
#$1:指定的项
#$2:原列表
#$3:目标列表
#res:命中的项的值
function get_list_hit_in(){
	[ $# == 3 ] || return 1
	local src_arr=($2)
	local dst_arr=($3)
	# fifo_show spec_val:$1	
	# fifo_show src_arr:$2
	# fifo_show dst_arr:$3
	for i in ${!src_arr[*]}
	do
		# fifo_show ==>:${src_arr[$i]}
		if [ x"${src_arr[$i]}" == x"$1" ];then
			echo ${dst_arr[$i]}
			return 0
		fi
	done
	# fifo_show $FUNCNAME not found!!!
	return 1
}
function get_file_by_state(){
	# fifo_show  $FUNCNAME $LINENO
	[ $# == 1 ] || return 1
	# fifo_show  $FUNCNAME $LINENO
	local src_arr=($state_list)
	local dst_arr=($state_file_list)
	for i in ${!src_arr[*]}
	do
		# fifo_shsow  $LINENO x"${src_arr[$i]}" == x"$1"
		if [ x"${src_arr[$i]}" == x"$1" ];then
			echo ${dst_arr[$i]}
			return 0
		fi
	done	
	return 1
}
#====================================================
function run_args_in_list(){
	local list=$1
	shift
	for i in $list
	do		
		echo $i $@
		$i $@
	done
}
#====================================================
function run_args_in_spec(){
	local list=$(find $1 -name "$2")
	run_args_in_list $list
	return $?
}
#====================================================
function source_list_in_spec(){
	local list=$(find $1 -name "$2")
	shift && shift
	for i in $list
	do		
		# echo source $i
		source $i
	done
}

function get_pkg_name_from(){
	local part=""
	for i in $1
	do
		part=${i%/*}
		part=${part##*/}
		echo $part
	done
}
#====================================================
#$1:path
#$2:spec file
#g_:g_pkg_path_dic
function build_pkg_path_dic(){
	local path_list=$(find $1 -name "$2")
	local name=""
	local part=""
	for i in $path_list
	do
		part=${i%/*}
		part=${part##*/}
		g_pkg_path_dic+=([$part]="$i")
	done	
}
function get_last_level_name(){
	local arr
	local bak_ifs
	local n
	for i in $@
	do
		bak_ifs=$IFS
		IFS='/'
		arr=($i)
		IFS=' '
		num=${#arr[*]}
		echo "${arr[$((n-1))]}"
		IFS=$bak_ifs
	done
}