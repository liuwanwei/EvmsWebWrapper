#!/bin/bash

# history
# 20090323, replace "-P" param with "-E" param used for grep

. /usr/sbin/sanager/Defines.sh

tmp_dir_file="/tmp/tmp_dir_file"
result_dir_file="/tmp/dir_content_list"

GetDirContent()
{
	shift

	dir=$1

	if [ -z $dir ]
	then
		echo "GetSubDirectoriesAndFiles: empty param!"
		exit $error_param
	fi

	# The last '0' in [KMGT0] permitted taken zero length files

	ls -h -p -s -1 $dir | grep -E "^[ 0-9]*\.*\w*[KMGT0]" > $result_dir_file

	# omit the first line the format as "total xxx"
	# using perl style regular express(-P)
	# start with "total",
	# then a ' ',
	# then one or multi numbers,
	# then a '.' or not,
	# then one or multi numbers,
	# then a letter.
	#cat $tmp_dir_file | grep -v -P "^total \d+.?\d+\w+" >  $result_dir_file


	# now we use this regular express
	# cat $tmp_dir_file | grep -E "^[-dl]([r-][w-][x-]){3}" >  $result_dir_file

	exit $error_ok
}

CreateDirectory()
{
	shift

	dir=$1

	exist=`echo $dir | grep -E "^$mount_dir" | wc -l`

	if [ $exist -ne 1 ]
	then
		echo "Invalid directory: $dir"
		exit $error_param
	fi

	if [ -d $dir ]
	then
		echo "Directory $dir already exist!"
		exit $error_exist
	fi

	mkdir $dir 

	exit $error_ok
}

case "$1" in
	GetDirContent)
		GetDirContent $@
		;;
	CreateDirectory)
		CreateDirectory $@
		;;
	*)
		echo "param : $1 error."
		echo "Usage : $0 GetSubDirectoriesAndFiles"
		exit 1
		;;
esac
