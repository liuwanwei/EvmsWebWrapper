#!/bin/bash

sbin_dir=/usr/sbin
sanager_bin_dir=$sbin_dir/sanager
net_server=$sanager_bin_dir/net_server
init_sys=$sanager_bin_dir/InitSys.sh

if [ ! -d $sanager_bin_dir ]
then
	echo "$sanager_bin_dir not exist!"
	exit 1
fi

if [ ! -e $net_server ]
then
	echo "$net_server not exist!"
	exit 1
fi

# Initial system configuration
$init_sys

# Launch net_server
ret=`ps ax | grep "\<net_server\>" | grep -v grep | wc -l`
if [ 1 -eq $ret ]
then
	killall -9 net_server
fi

$net_server

