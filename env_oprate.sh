#source env_oprate.sh
cur_dir=`pwd`
pro_name=x86_64-linux-gnu
env_bakup=$cur_dir/buildsys/output/$pro_name/.env_bakup
function env_setup(){
	echo $PATH > $env_bakup
	export LD_LIBRARY_PATH=$cur_dir/output/$pro_name/staging/usr/lib
	export PATH=$cur_dir/output/$$pro_name/staging/usr/bin:$PATH
}
function env_revert(){
	export PATH=`cat $env_bakup`
	export LD_LIBRARY_PATH=""
}
