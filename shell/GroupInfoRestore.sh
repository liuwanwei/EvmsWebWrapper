#!/bin/bash

. /usr/sbin/sanager/Defines.sh

echo "begin" >> $startup_log

if [ ! -e $groups_bk_dir ]
then
	exit $error_param
fi

RestoreDeviceCtrlFile()
{
	if [ $# -lt 2 ]
	then
		echo "RestoreDeviceCtrlFile param error!" >> $startup_log
		return $error_param
	fi

	device_ctrl_file=/proc/scsi_tgt/groups/$2/"devices"

	cat $1 | while read LINE
	do
		device=`echo $LINE | awk '{print $1}'`
		device_details=`echo $LINE | awk '{print $2}'`
		virtual_lun=`echo $LINE | awk '{print $3}'`
		options=`echo $LINE | awk '{print $4}'`

		if [ ! $device = "Device" ]
		then
			if [ ! -z $options ]
			then
				echo "add $device [READ_ONLY]" > $device_ctrl_file
			else
				echo "add $device" > $device_ctrl_file
			fi
		fi
		:
	done

	return $error_ok
}

RestoreIBPortCtrlFile()
{
	if [ $# -lt 2 ]
	then
		echo "RestoreIBPortCtrlFile param error!" >> $startup_log
		return $error_param
	fi

	cat $1 | while read LINE
	do
		echo $LINE > /proc/scsi_tgt/groups/$2/"names"
		:
	done

	return $error_ok
}

# group restore entry
RestoreGroup()
{
	if [ ! $# -eq  1 ]
	then
		echo "RestoreGroup error param $#!" >> $startup_log
		return $error_param
	fi

	group_name=$1
	cur_group_bk_dir=$groups_bk_dir/$group_name

	if [ ! -d $cur_group_bk_dir ]
	then
		echo " Not exist: $cur_group_bk_dir !" >> $startup_log
		return $error_notexist
	fi

	# create group
	# echo $group_ctrl_file
	echo "add_group $group_name" > $group_ctrl_file

	# restore group device control file
	RestoreDeviceCtrlFile $cur_group_bk_dir"/devices" $group_name
	if [ $? -ne $error_ok ]
	then
		echo "RestoreDeviceCtrlFile $cur_group_bk_dir failed!" >> $startup_log
		return $error_fail
	fi

	# restore group ibport control file
	RestoreIBPortCtrlFile $cur_group_bk_dir"/names"   $group_name 
	if [ $? -ne $error_ok ]
	then
		echo "RestoreIBPortCtrlFile $cur_group_bk_dir failed!" >> $startup_log
		return $error_fail
	fi

	return $error_ok
}

groups=`ls $groups_bk_dir`

if [ "$groups " = " " ]
then
	echo "$groups not exist" >> $startup_log
	exit $error_notexist
fi

# the main working loop: restore each group 
for group_name in $groups
do
	echo "restore group $group_name" >> $startup_log
	RestoreGroup $group_name
done

echo "end" >> $startup_log

exit $error_ok

