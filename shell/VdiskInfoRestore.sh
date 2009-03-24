#!/bin/bash

. Defines.sh

if [ ! -e $vdisk_bk_file ]
then
	echo "$vdisk_bk_file not exist!"
	exit 0
fi

vdisk=""
device=""

# restore all vdisks in /usr/sbin/sanager/vdisk
cat $vdisk_bk_file | grep -E "^vdisk" | while read LINE
do
	vdisk=`echo $LINE |  awk '{print $1}'`
	device=`echo $LINE | awk '{print $4}'`

	echo "wwliu: $vdisk"
	echo "wwliu: $device"

	is_file=`ls -l -h $device| grep -E "^-([r-][w-][x-]){3}" | wc -l`
	if [ 1 -eq $is_file ]
	then
		# FILEIO mode for file.
		echo "open $vdisk $device" > $vdisk_ctrl_file
	else
		# BLOCKIO mode for disk.
		echo "open $vdisk $device BLOCKIO" > $vdisk_ctrl_file
	fi
done
