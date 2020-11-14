#!/bin/bash
work_dir=`dirname $(readlink -m $0)`
new_ds_name=$1
new_hit_name=$2
add_new_line_ctt='int '$new_ds_name'_main(int argc,char* argv[]);'
add_contex_ctt='\\tmainMap["'$new_hit_name'"] = '$new_ds_name'_main;'

[ x"$new_hit_name" == x"" ] && new_hit_name="$new_ds_name"
cd $work_dir &&\
sed -i  "/<unordered_map>/ a$add_new_line_ctt" main.cpp && \
sed -i "/mainMap;/ a$add_contex_ctt" main.cpp

