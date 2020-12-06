function get_time_stamp(){
	local ts_arr=(`date "+%s %N"`)
	local ns_val=`echo ${ts_arr[1]} | sed 's/^0*//'`
	local ms_var=$(($ns_val/1000000))
	local date_var=`date -d @"${ts_arr[0]}" "+%Y-%m-%d %H:%M:%S"`
	printf "%s.%03d" "$date_var" "$ms_var"
}
# func_info $LINENO $FUNCNAME ...
function func_info(){
	local log_str="\e[0;32m"
	log_str="\e[0;32m$log_str[`get_time_stamp`][${0##*/}:$1]:>>>"
	shift	
	log_str="$log_str\e[1;32m$@\e[0m"
	echo -e $log_str
}
function tip_msg(){
	local log_str="[${0##*/}:$1]:>>>"
	shift
	log_str="\e[1;32m$log_str $@ done!\e[0m"
	echo -e $log_str
}
function err_msg(){
	local log_str="[${0##*/}:$1]:>>>"
	shift
	log_str="\e[1;31m$log_str $@ failed!\e[0m"
	echo -e $log_str
}
function res_info(){
	local res=$1
	shift
	[ $res == 0 ] && tip_msg $@ || err_msg $@
	return $res
}
function empty_assert(){
	local key="$1"
	shift
	if [ "$key" != "" ];then
		tip_msg $@
		return 0
	fi
	err_msg $@
	exit 1
}
