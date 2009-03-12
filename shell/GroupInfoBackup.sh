#!/bin/bash

defines_file="Defines.sh"

if [ ! -e "$defines_file" ]
then
	cd shell
fi

. $defines_file


Usage()
{
	echo "./BackupGroupInfo.sh group_name [add|del]"
}

if [ $# -lt 1 ]
then
	Usage
	exit $error_param
fi

group_name=$1
src_group_dir=$base_group_dir/$group_name
src_ibports_ctrl_file=$src_group_dir/"names"
src_devices_ctrl_file=$src_group_dir/"devices"

if [ ! -d $src_group_dir ]
then
	echo "$src_group_dir not exist!"
	exit $error_notexist
fi

groups_bk_dir="/usr/local/sbin/sanager/groups_bk"

if [ ! -d $groups_bk_dir ]
then
	mkdir $groups_bk_dir
fi

dst_group_dir=$groups_bk_dir/$group_name
dst_ibports_ctrl_file=$dst_group_dir/"names"
dst_devices_ctrl_file=$dst_group_dir/"devices"

if [ ! -d $dst_group_dir ]
then
	mkdir $dst_group_dir
fi

if [ $2 = "del" ]
then
	rm $dst_group_dir -rf
else
	cp $src_ibports_ctrl_file $dst_ibports_ctrl_file -rf
	cp $src_devices_ctrl_file $dst_devices_ctrl_file -rf
fi

exit $error_ok
