#!/bin/bash
#=========================================================================
function def_download(){
	[ -e $src_path ] && return 0
	rm -rf $PRO_DL_TMP_PATH/* && \
	cd $PRO_DL_TMP_PATH && \
		wget "$package_source_uri" && \
	mv $PRO_DL_TMP_PATH/* $src_path
}
function def_sync(){
	[ -e $dst_path/$sync_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	if [ -d $dst_path ];then
		touch $dst_path/$patch_sfile
		return 0
	fi
	install_path $dst_path
	case $cur_archive_type in
	src)
		return 0
		;;
	tar.xz|xz)
		def_download && \
		rm -rf $PRO_DL_EXT_PATH/* && \
		cd $PRO_DL_EXT_PATH && \
			xz -dkc $src_path > $src_all_name.tar  && \			
			tar xvf $src_all_name.tar -C $dst_path && \
			rm -rf $src_all_name.tar && \
			mv $dst_path/* "$dst_path"_tmp && \
			mv -T "$dst_path"_tmp $dst_path
		;;
	zip)
		def_download && \
		cd $dst_path && \
			unzip $src_path && \
			mv $dst_path/* "$dst_path"_tmp && \
			mv -T "$dst_path"_tmp $dst_path
		;;		
	tar.gz|tgz|tar.bz2)
		def_download && \	
		cd $dst_path && \
			tar xvf $src_path &&\
			mv $dst_path/* "$dst_path"_tmp && \
			mv -T "$dst_path"_tmp $dst_path
		;;
	git)
		cd $dst_path && \
			git clone $package_source_uri
		;;			
	* )			
		echo unsurport archive type:$cur_archive_type !!
		return 1
		;;
	esac	
}
function call_sync(){
	if [ "`type -t pkg_sync`" == "function" ];then
		pkg_sync $@
	else
		def_sync $@
	fi
}
#=========================================================================
function def_patch(){
	[ -e $dst_path/$patch_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $cur_dir &&\
		local patchs=$(find -name "*.patch" | sort -n)
	[ x"$patchs" == x"" ] && return 0	
	cd $dst_path && \
	for i in $patchs
	do
		patch -i $cur_dir/$i -p1
		if [ $? != 0 ];then
			echo patch $cur_dir/$i failed!!!
			return 1
		fi
	done	
}
function call_patch(){
	if [ "`type -t pkg_patch`" == "function" ];then
		pkg_patch $@
	else
		def_patch $@
	fi
}
#=========================================================================
function def_config(){
	[ -e $dst_path/$config_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
}
function call_config(){
	if [ "`type -t pkg_config`" == "function" ];then
		pkg_config $@
	else
		def_config $@
	fi
}

#=========================================================================
function def_build(){
	[ -e $dst_path/$build_sfile ] && return 0
	func_info $0 $LINENO $FUNCNAME
	cd $dst_path && \
	make -j $PROCESS_NUM && \
	make install
}
function call_build(){
	if [ "`type -t pkg_build`" == "function" ];then
		pkg_build $@
	else
		def_build $@
	fi	
}
#=========================================================================
function def_install_stag(){
	[ -d $cur_prefix/lib ] && \
		cp -raf $cur_prefix/* $PRO_STAG_PATH/usr/
	[ -d $cur_prefix/include ] && \
		cp -raf $cur_prefix/include $PRO_STAG_PATH/usr/
	return 0
}
function def_install_stub(){
	local done_count=0
	install_path $cur_stub_path
	[ -d $cur_prefix/bin ] && \
		cp -raf $cur_prefix/bin $cur_stub_path/usr/ && \
		done_count=$((done_count+1))
	[ -d $cur_prefix/lib ] && \
		cp -raf $cur_prefix/lib $cur_stub_path/usr/ &&\
		done_count=$((done_count+1))
	[ -d $cur_prefix/include ] && \
		cp -raf $cur_prefix/include $cur_stub_path/usr/ && \
		done_count=$((done_count+1))
	[ $done_count -gt 0 ] && return 0 || return 1
}
function def_install_target(){
	local done_count=0
	[ -d $cur_prefix/bin ] && \
		cp -raf $cur_prefix/bin $PRO_TARGET_PATH/usr/ && \
		done_count=$((done_count+1))
	local so_list=`find $cur_prefix -name "*.so*"`
	# echo so_list:$so_list
	if [ x"$so_list" != x"" ];then
		install_path $PRO_TARGET_PATH/usr/lib
		cp -raf $so_list $PRO_TARGET_PATH/usr/lib/
		done_count=$((done_count+1))
	fi
	# echo done_count=$done_count
	[ $done_count -gt 0 ] && return 0 || return 1
}
function def_install(){
	[ -e $dst_path/$install_sfile ] && return 0
	def_install_stag && \
	def_install_stub && \
	def_install_target
}
function call_install(){
	if [ "`type -t pkg_install`" == "function" ];then
		pkg_install $@
	else
		def_install $@
	fi
}

function pkg_clean_for(){
	# func_info $0 $LINENO $FUNCNAME
	[ ! -d $dst_path ] && return 0
	local state_file=$(get_list_hit_in "$1" "$redo_cmd_list" "$state_file_list")
	# echo state_file:$state_file
	if [ x"$state_file" != x"" ];then
		local sub_list=$(get_sub_list "$state_file_list" $state_file)
		# echo sub_list:$sub_list
		[ x"$sub_list" == x"" ] && return 1
		cd $dst_path && \
		rm -rf $sub_list
	else
		state_file=$(get_list_hit_in $1 "$state_list" "$state_file_list")
		# echo state_file:$state_file
		[ x"$state_file" == x"" ] && return 1
		cd $dst_path && \
		rm -rf $state_file
	fi
}

function ex_make(){
	func_info $0 $LINENO $FUNCNAME
	# [ -e $dst_path/$install_sfile ] && exit 0
	call_sync 		&& touch $dst_path/$sync_sfile			&& \
	call_patch 		&& touch $dst_path/$patch_sfile			&& \
	call_config 	&& touch $dst_path/$config_sfile		&& \
	call_build 		&& touch $dst_path/$build_sfile			&& \
	call_install 	&& touch $dst_path/$install_sfile
}

function ex_clean(){
	func_info $0 $LINENO $FUNCNAME
	rm -rf $dst_path
}

case $1 in
clean) 		ex_clean
			;;
remake)		ex_clean;ex_make
			;;
* )			pkg_clean_for $1;ex_make
			;;
esac