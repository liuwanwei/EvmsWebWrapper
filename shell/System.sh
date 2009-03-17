#!/bin/bash

tmp_dir_file="/tmp/tmp_dir_file"

GetSubDirectoriesAndFiles()
{
	shift

	dir=$1

	if [ -z $dir ]
	then
		echo "GetSubDirectoriesAndFiles: empty param!"
		return $error_param
	fi

	ls -l -h $dir > $tmp_dir_file

	cat $tmp_dir_file | while read LINE
	do
	done

	return $error_ok
}

case "$1" in
	GetSubDirectoriesAndFiles)
		GetSubDirectoriesAndFiles $@
		;;
	*)
		echo "param : $1 error."
		echo "Usage : $0 GetSubDirectoriesAndFiles"
		exit 1
		;;
esac
