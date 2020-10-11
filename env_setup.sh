cur_dir=`pwd`
function env_set(){
	echo $PATH > $cur_dir/output/x86_64-linux-gnu/.env_bakup
	LD_LIBRARY_PATH=$cur_dir/output/x86_64-linux-gnu/staging/usr/lib
	PATH=$cur_dir/output/x86_64-linux-gnu/staging/usr/bin:$PATH
}
function env_revert(){
	PATH=`cat $cur_dir/output/x86_64-linux-gnu/.env_bakup`
	LD_LIBRARY_PATH=""
}
