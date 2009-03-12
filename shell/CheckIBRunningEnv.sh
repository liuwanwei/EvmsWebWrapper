#! /bin/bash

define_file="Defines.sh"

# If called from parent directory, the CWD will be in parent directory, so we need to change it to "ib" dir  manually.
if [ -e $define_file ]
then
	. $define_file
else
	. shell/$define_file
fi


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
	printf "Checking Modules ...					"

	modules_list="scst ib_srpt"

	for i in $modules_list
	do
		CheckModule $i
		ret=$?

		if [ ! $ret -eq 0 ]
		then
			echo "Error while Checking $i"
			return $ret
		fi
	done

	printf "[OK]\n"

	return $error_ok
}

CannotFind()
{
	echo "Can't find $1"

	return $error_ok
}

CheckDirectories()
{
	printf "Checking Directories ...				"

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

	printf "[OK]\n"

	return $error_ok
}

CheckDaemons()
{
	printf "Checking Daemons					"

	daemons="opensm heartbeat evmsd"

	for daemon in $daemons
	do
		exist=`ps ax | grep $daemon | grep -v grep | wc -l`
		if [ $exist -lt 1 ]
		then
			CannotFind $daemon
			return $error_daemons
		fi
	done

	printf "[OK]\n"
}

CheckAll()
{
	ret=0

	echo ""

	CheckModules
	ret=$?
	if [ ! $ret -eq 0 ]
	then
		return $error_daemons
	fi

	CheckDirectories
	ret=$?
	if [ ! $ret -eq 0 ]
	then
		return $error_directories
	fi

	CheckDaemons
	ret=$?
	if [ ! $ret -eq 0 ]
	then
		return $error_daemons
	fi
}


case "$1" in
	all)
		CheckAll
		;;
	modules)
		CheckModules
		;;
	dirs)
		CheckDirectories
		;;
	daemons)
		CheckDaemons
		;;
	*)
		echo "Usage: $0 {all | modules | dirs | daemons}"
		exit 1
esac
