#! /bin/bash

. /etc/init.d/functions
. /usr/sbin/sanager/Defines.sh

# Chekd if the specific driver module exists.
CheckModule()
{
	#echo "Checking $1..."

	ret=` lsmod | awk '{print $1}' | grep "\<$1\>" | grep -v grep `

	if [ " $ret" != " $1" ]
	then
		return $error_fail
	fi

	return $error_ok
}

# Check if all IB-dependent driver modules exist.
CheckModules()
{
	#printf "Checking Modules ...					"

	modules_list="scst ib_srpt iscsi_trgt"

	for i in $modules_list
	do
		CheckModule $i
		ret=$?

		if [ ! $ret -eq 0 ]
		then
			#echo "Error while Checking $i"
			return $ret
		fi
	done

	printf "[OK]\n"

	return $error_ok
}

CannotFind()
{
	# echo "Can't find $1"

	return $error_ok
}

CheckDirectories()
{
	#printf "Checking Directories ...				"

	# Firstly, IB related directories.
	if [ ! -d $base_srpt_dir ]
	then
		CannotFind $base_srpt_dir
		return $error_fail
	fi

	if [ ! -e $group_ctrl_file ]
	then
		CannotFind $group_ctrl_file
		return $error_fail
	fi

	if [ ! -d $base_group_dir ]
	then
		CannotFind $base_group_dir
		return $error_fail
	fi

	if [ ! -d $base_vdisk_dir ]
	then
		CannotFind $base_vdisk_dir
		return $error_fail
	fi

	if [ ! -e $vdisk_ctrl_file ]
	then
		CannotFind $vdisk_ctrl_file
		return $error_fail
	fi

	# Secondly, iSCSI related directories
	if [ ! -d $iscsi_proc_dir ]
	then
		CannotFind $iscsi_proc_dir
		return $error_fail
	fi

	if [ ! -e $iscsi_proc_volume_ctrl_file ]
	then
		CannotFind $iscsi_proc_volume_ctrl_file
		return $error_fail
	fi

	if [ ! -e $iscsi_proc_session_ctrl_file ]
	then
		CannotFind $iscsi_proc_session_ctrl_file_
		return $error_fail
	fi

	return $error_ok
}

CheckDaemons()
{
	daemons="opensm heartbeat evmsd ietd"

	for daemon in $daemons
	do
		exist=`ps ax | grep $daemon | grep -v grep | wc -l`
		if [ $exist -lt 1 ]
		then
			CannotFind $daemon
			return $error_daemons
		fi
	done
}

case "$1" in
	all)
		action $"Checking kernel modules..." CheckModules
		action $"Checking directories..."    CheckDirectories
		action $"Checking daemons..."        CheckDaemons
		;;
	modules)
		action $"Checking kernel modules..." CheckModules
		;;
	dirs)
		action $"Checking directories..."    CheckDirectories
		;;
	daemons)
		action $"Checking daemons..."        CheckDaemons
		;;
	*)
		echo "Usage: $0 {all | modules | dirs | daemons}"
		exit 1
esac
