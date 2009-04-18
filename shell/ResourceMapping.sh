#! /bin/bash

. /usr/sbin/sanager/Defines.sh

AddGroup()
{
	shift

	group=$1

	if [ -z $group ]
	then
		echo "AddGroup: empty param!"
		return $error_param
	fi

	if [ -d $base_group_dir/$group ]
	then
		echo "$base_group_dir/$group already exist"
		return $error_exist
	fi

	echo "Now adding $group to SRPT group list"

	echo "add_group $group" > $group_ctrl_file

	$group_backup_script $group "add"

	return $error_ok
}

# Function: Delete group
# Parameter:
# 1, group name to be deleted.
DelGroup()
{
	shift

	group=$1

	if [ -z $group ]
	then
		return $error_param
	fi

	group_base_dir=$base_srpt_dir"groups/"$group

	# The group's working directory should be exist.
	if [ ! -d $group_base_dir ]
	then
		return $error_noexist
	fi

	echo "del_group $group" > $group_ctrl_file

	$group_backup_script $group "del"

	return $error_ok
}

# not used
# Group names can be added to $group_ctrl_file directlly, but can't fetched from file later.
GetAllGroups()
{
	rm $tmp_group_name_file -rf

	ls $base_group_dir"/" 		> $tmp_group_name_file

	return $error_ok
}

# Function: Add device to group, one device each time.
# Parameter: 
# 1, resource name, eg. /dev/evms/Volume1
# 2, lun index to add, eg. 0, 1, 2, 3 ... n
# 3, group to add.
AddDeviceToGroup()
{
	shift 

	# resource name
	device=$1
	lun_idx=$2 
	group=$3

	# Checking params
	if [ -z $device ] || [ -z $lun_idx ] || [ -z $group ]
	then
		echo "param"
		return $error_param
	fi

	# check group device control file
	group_res__ctrl_file=$base_srpt_dir"groups/"$group"/devices"
	if [ ! -e $group_res__ctrl_file ]
	then
		echo "$group_res__ctrl_file not exist"
		return $error_notexist
	fi

	# File device and DM device are all handled by scst_vdisk driver,
	# In scst's README file, VDISK could be created by the command:
	# "open NAME [PATH] [BLOCK_SIZE] [FLAGS]"
	# and "BLOCK_SIZE" and "FLAGS" are only valid for disk VDISK.
	# So file type VDISK only need "NAME" and "PATH".
	# Below lines are quoted from OFED's SRPT_README.txt
	# a. modprobe scst
	# b. modprobe scst_vdisk
	# c. echo "open vdisk0 /dev/md0" > /proc/scsi_tgt/vdisk/vdisk
	# d. echo "open vdisk1 /10G-file" > /proc/scsi_tgt/vdisk/vdisk
	# e. echo "add vdisk0 0" >/proc/scsi_tgt/groups/Default/devices
	# f. echo "add vdisk1 1" >/proc/scsi_tgt/groups/Default/devices

	# Create virtual device automatically if $device is a file or virtual disk.
	# FIXME What should I do if $device is a real device? 
	# SCST only support SCSI typed real device, this is belonged to todo list.
	./CreateVDiskFromFile.sh $device $lun_idx
	if [ ! $? -eq 0 ]
	then
		echo "CreateVDiskFromFile error"
		return $error_fail
	fi

	# get vdisk name from tmp file assigned by CreateVDiskFromFile
	if [ ! -e $return_value_file ]
	then
		echo "$return_value_file not exist"
		return $error_fail
	fi
	vdisk_name=`cat $return_value_file`

	# check if the virtual lun number valid.
	cat $group_res__ctrl_file | while read LINE
	do
		lun=`echo $LINE | awk '{print $2}'`
		if [ ! -z $lun ]
		then
			if [ $lun -eq $2 ]
			then
				exit $error_exist;
			fi
		fi
	done

	# add vdisk and vlun to resource group
	echo "add $vdisk_name $lun_idx" >  $group_res__ctrl_file
	if [ $? -ne 0 ]
	then
		echo "add $vdisk_name $lun_idx failed"
		return $error_fail
	fi

	echo $vdisk_name > $return_value_file

	$group_backup_script $group "add"

	return $error_ok
}

# Function: Delete device from group
# Parameters:
# 1, device name
# 2, group name
DelDeviceFromGroup()
{
	shift

	vdisk=$1	
	group=$2

	if [ -z $vdisk ] || [ -z $group ]
	then
		return $error_param
	fi

	group_res__ctrl_file=$base_srpt_dir"groups/"$group"/devices"

	count=`cat $group_res__ctrl_file | grep "\<$vdisk\>" | wc -l`

	if [ $count -eq 0 ]
	then
		return $error_notexist
	fi

	# delete vdisk from group's resource control file
	echo "del $vdisk " > $group_res__ctrl_file

	# delete vdisk if it is created by us on background
	count=`cat $vdisk_ctrl_file | grep "\<$vdisk\>" | wc -l`
	if [ $count -ne 0 ]
	then
		# remove $vdisk from /proc/scsi_tgt/vdisk/vdisk
		echo "close $vdisk" > $vdisk_ctrl_file

		# backup vdisk ctrl file
		cp $vdisk_ctrl_file $vdisk_bk_file -rf
	fi

	$group_backup_script $group "add"

	return $error_ok
}

# not used
# group's devices can be found in /proc/scsi_tgt/groups/group name/devices directlly
GetDevicesFromGroup()
{
	return $error_fail
}

GetAllIBPorts()
{	
	inter_file="/tmp/search_inter_file"
	result_file="/tmp/all_linked_ports"

	rm $inter_file 	-rf
	rm $result_file -rf

	# Get IB card's node guid(command "bc" requires Uppercase hex number)
	node_guid=` ibv_devinfo | grep node_guid | grep -v grep | awk '{print $2}' | sed -e 's/://g' | sed -e 'y/abcdef/ABCDEF/'`

	# Get first port's guid on the card.
	port1_guid=` echo "ibase=16;$node_guid + 1" | bc`
	port1_guid=` echo "obase=16;$port1_guid"    | bc`
	port1_guid=` echo $port1_guid | sed -e 'y/ABCDEF/abcdef/'`
	port1_guid="0x$port1_guid"
	port1_guid=`printf "0x%016x" $port1_guid`

	# Get second ports' guid on the card.
	port2_guid=` echo "ibase=16;$node_guid + 2" | bc`
	port2_guid=` echo "obase=16;$port2_guid"    | bc`
	port2_guid=` echo $port2_guid | sed -e 'y/ABCDEF/abcdef/'`
	port2_guid="0x$port2_guid"
	port2_guid=`printf "0x%016x" $port2_guid`

	# Searching port1's link path
	osmtest -g $port1_guid -f c > $inter_file
	if [ ! $? -eq 0 ] 
	then
		echo "$port1_guid link path search failed!"
		ret=$error_fail
	fi

	# Searching port2's link path
	osmtest -g $port2_guid -f c >> $inter_file
	if [ ! $? -eq 0 ] 
	then
		echo "$port2_guid link path search failed!"
		if [ $ret == $error_fail ]
		then
			return $error_fail
		fi
	fi

	# Get port guid on the other peer of the link
	if [ ! -e $inter_file ]
	then
		echo "$inter_file not exist!"
		return $error_notexist
	fi

	# cat $inter_file | grep port_guid | grep -v grep | awk '{print $2}' > $result_file

	# now use perl scripts to get all ib ports
	../pl/GetConnectedPorts.pl $port1_guid $port2_guid

	return $error_ok
}

# Parameters:
# 1, Infiniband port name.
# 2, group name to add
AddIBPortToGroup()
{
	shift

	port_name=$1
	group=$2

	if [ -z $port_name"" ] || [ -z $group"" ]
	then
		return $error_param
	fi

	group_ports_ctrl_file=$base_srpt_dir"group/"$group"/names"

	echo "add $port_name" > $group_ports_ctrl_file

	$group_backup_script $group "add"

	return $error_ok
}

# Parameters:
# 1, Infiniband port name.
# 2, group name to add
DelIBPortFromGroup()
{
	shift

	port_name=$1
	group=$2

	if [ -z $port_name"" ] || [ -z $group"" ]
	then
		return $error_param
	fi

	group_ports_ctrl_file=$base_srpt_dir"group/"$group"/names"

	count=`cat $group_ports_ctrl_file | grep $port_name | grep -v grep | wc -l`

	if [ $count -eq 0 ]
	then
		return $error_notexist
	fi

	echo "del $port_name" > $group_ports_ctrl_file

	$group_backup_script $group "add"

	return $error_ok
}

# not used
# The group's IB ports can be found in /proc/scsi_tgt/groups/[group name]/names directlly
GetIBPortsFromGroup()
{
	return $error_fail
}

case "$1" in
	DelIBPortFromGroup)
		DelIBPortFromGroup $@
		;;
	GetAllGroups)
		GetAllGroups
		;;
	AddGroup)
		AddGroup $@
		;;
	DelGroup)
		DelGroup $@
		;;
	AddDeviceToGroup)
		AddDeviceToGroup $@
		;;
	DelDeviceFromGroup)
		DelDeviceFromGroup $@
		;;
	GetAllIBPorts)
		GetAllIBPorts
		;; 
	*)
		echo "param : $1 error."
		echo "Usage: $0 { AddGroup | DelGroup | AddDeviceToGroup | DelDeviceFromGroup | GetAllIBPorts | DelIBPortFromGroup ...}"
		exit 1
		;;
esac
