#!/bin/sh
cur_dir=`pwd`
pro_name='x86_64-linux-gnu'
export LD_LIBRARY_PATH=$cur_dir/output/$pro_name/staging/usr/lib
export PATH=$cur_dir/output/$pro_name/staging/usr/bin:$PATH
pkg_name="$1" 
# run_bin="TcpServTest"
# run_bin="ChronoTest"
run_bin="TupleTest"
# run_bin="EpollTest"
ulimit -c unlimited
c008-makeLinks.sh myBox $run_bin
compile_run(){
	rm -rf core
	./mk.sh $@
	[ $? != 0 ] && exit 1
	$run_bin -l 1111 || gdb $run_bin core
	# $run_bin -l 1111 &
}
compile_run $@
# run_pid=`ps | grep $run_bin| awk '{print $1}'`
# while true;do
	# cat /proc/$run_pid/stat | awk '{print $23}'
	# sleep 0.5
# done