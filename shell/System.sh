#!/bin/bash

tmp_dir_file="/tmp/tmp_dir_file"
result_dir_file="/tmp/dir_content_list"

GetDirContent()
{
	shift

	dir=$1

	if [ -z $dir ]
	then
		echo "GetSubDirectoriesAndFiles: empty param!"
		return $error_param
	fi

	ls -l -h $dir > $tmp_dir_file

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
	cat $tmp_dir_file | grep -P "^[-dl]([r-][w-][x-]){3}" >  $result_dir_file

	return $error_ok
}

case "$1" in
	GetDirContent)
		GetDirContent $@
		;;
	*)
		echo "param : $1 error."
		echo "Usage : $0 GetSubDirectoriesAndFiles"
		exit 1
		;;
esac
