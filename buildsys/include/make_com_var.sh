pkg_all_name=$pkg_name-$pkg_ver
pkg_all_file_name=$pkg_name-$pkg_ver.$cur_archive_type
src_path=$PRO_DL_PATH/$pkg_all_file_name
pkg_source_uri=$pkg_source_url/$pkg_all_file_name
dst_path=$PRO_BUILD_PATH/$pkg_all_name
cur_prefix=$dst_path/ipkg-install
cur_stub_path=$PRO_STUB_PATH/$pkg_all_name/$ARCH
cur_dir=`pwd`/`dirname $0`