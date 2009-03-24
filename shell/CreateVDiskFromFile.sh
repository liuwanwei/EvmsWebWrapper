#!/bin/bash

# We use the default flags for creating VDISK
# As for file, no flags need.
# As for disk, we use default BLOCK_SIZE:512 bytes, and FLAG set to "BLOCKIO"


. Defines.sh

if [ $# -lt 2 ]
then
	echo "Usage: ./CreateVDiskFromFile.sh real_file_path lun_number"
	exit $error_param
fi

if [ ! -e $vdisk_ctrl_file ]
then
	exit $error_param
fi

real_file_name=$1
vlun_name="lun$2"
vdisk_name=""

# get current maximum vdisk index, and use it to generate vdisk name
max_vdisk_index=0
cat $vdisk_ctrl_file | while read LINE
do
	# get the maximum virtual disk index
	name=`echo $LINE | awk '{print $1}' | grep "^vdisk"`
	if [ ! -z $name ]
	then
		# cut off the first 5 charactors:"vdisk".
		index=${name:5}
		if [ $index -gt $max_vdisk_index ]
		then
			max_vdisk_index=`echo $index`
		fi
	fi
done


# generate vdisk name
max_vdisk_index=`expr $max_vdisk_index + 1`
vdisk_name="vdisk$max_vdisk_index"

# create vdisk
# echo "$real_file_name"
is_file=`ls -l -h $real_file_name | grep -E "^-([r-][w-][x-]){3}" | wc -l`
if [ 1 -eq  $is_file ]
then
	# FILEIO mode for file.
	echo "open $vdisk_name $real_file_name" > $vdisk_ctrl_file
else
	# BLOCKIO mode for disk.
	echo "open $vdisk_name $real_file_name BLOCKIO" > $vdisk_ctrl_file
fi

if [ $? -ne 0 ]
then
	exit $error_fail
fi

# backup vdisk ctrl file
cp $vdisk_ctrl_file $vdisk_bk_file -rf

# return vdisk name to caller
echo $vdisk_name > $return_value_file

exit 0

