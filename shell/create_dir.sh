#!/bin/bash

. Defines.sh

if [ $# -lt 1 ]
then
	echo "Usage :./create_dir group_name"
	exit $error_param
fi

if [ ! -d /proc ]
then
	mkdir /proc
fi

if [ ! -d /proc/scsi_tgt ]
then
	mkdir /proc/scsi_tgt
fi

if [ ! -e /proc/scsi_tgt/scsi_tgt ]
then
	touch /proc/scsi_tgt/scsi_tgt
fi

if [ ! -d /proc/scsi_tgt/groups/ ]
then
	mkdir /proc/scsi_tgt_groups
fi

if [ ! -d /proc/scsi_tgt/groups/$1 ]
then 
	mkdir /proc/scsi_tgt/groups/$1

	touch /proc/scsi_tgt/groups/$1/devices
	touch /proc/scsi_tgt/groups/$1/names
fi

exit $error_ok
