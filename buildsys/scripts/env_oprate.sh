#source env_oprate.sh
cur_dir=`pwd`
pro_name='x86_64-linux-gnu'
env_bakup=$cur_dir/buildsys/output/$pro_name/.env_bakup
function env_view(){
	echo PATH=$PATH
	echo LD_LIBRARY_PATH=$LD_LIBRARY_PATH
}
function env_clear(){
	rm -rf $env_bakup
}
function env_setup(){
	echo $PATH > $env_bakup
	env_revert
	export LD_LIBRARY_PATH=$cur_dir/output/$pro_name/staging/usr/lib
	export PATH=$cur_dir/output/$pro_name/staging/usr/bin:$cur_dir/output/$pro_name/staging/usr/sbin:$PATH
	env_view
}
function env_revert(){
	export PATH=`cat $env_bakup`
	export LD_LIBRARY_PATH=""
	env_clear
}
case $1 in
	set|setup)
		env_setup
		;;		
	rvt|revert)
		env_revert
		;;
	*)	env_view
		;;
esac


