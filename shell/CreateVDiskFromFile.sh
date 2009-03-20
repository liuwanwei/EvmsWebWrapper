#!/bin/bash

. Defines.sh

if [ $# -eq 0 ]
then
	echo "Usage: ./CreateVDiskFromFile.sh real_file_path"
	exit $error_param
fi

if [ ! -e $vdisk_ctrl_file ]
then
	exit $error_param
fi

real_file_name=$1
vdisk_name=""
vlun_name=""

max_index=1
cat $vdisk_ctrl_file | while read LINE
do
	name=`echo $LINE | awk '{print $1}' | grep "^vdisk"`
	if [ ! -z $name ]
	then
		index=${name:5}
		if [ $index -gt $max_index ]
		then
			max_index=`echo $index`
		fi
	fi
done

max_index=`expr $max_index + 1`


