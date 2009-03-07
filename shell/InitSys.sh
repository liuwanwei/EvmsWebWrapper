#!/bin/bash

scst_driver="scst_vdisk"

. /usr/sbin/sanager/Defines.sh

date=`date`
echo $date >> $startup_log

# scst driver has been invoked by somebody I don't know
modprobe $scst_driver

# wait for scst drivers invoke finish
sleep 1

if [ $? != 0 ]
then
	echo "modprobe $scst_driver failed!"
fi

# shutdown the opensm process already running
opensm_process_id=`ps ax | grep opensm | grep -v grep | awk '{print $1}'`
if [ "$opensm_process_id " != " " ]
then
	for id in $opensm_process_id
	do
		echo "Kill $id"
		kill -9 $id
	done

	echo "Shutdown opensm ... " >> $startup_log
	sleep 3
fi

# check if we successfully shutdown 'opensm'.
opensm_process_id=`ps ax | grep opensm | grep -v grep | awk '{print $1}'`
echo $opensm_process_id
if [ "$opensm_process_id " != " " ]
then
	echo "kill opensm process failed" >> $startup_log
	exit 1
fi


# Get IB card's node guid(command "bc" requires Uppercase hex number)
ibv_devinfo="/usr/bin/ibv_devinfo"
if [ -e $ibv_devinfo ]
then
	node_guid=`$ibv_devinfo | grep node_guid | awk '{print $2}' \
       		 | sed -e 's/://g' | sed -e 'y/abcdef/ABCDEF/'`

	phys_port_cnt=`$ibv_devinfo | grep phys_port_cnt | awk '{print $2}'`
else
	echo "$ibv_devinfo not exist!"
fi


# load opensm on at most two ib ports
opensm="/usr/sbin/opensm"
if [ -e $opensm ]
then
	# Get first port's guid on the card and run opensm on it.
	port1_guid=` echo "ibase=16;$node_guid + 1" | bc`
	port1_guid=` echo "obase=16;$port1_guid"    | bc`
	port1_guid=` echo $port1_guid | sed -e 'y/ABCDEF/abcdef/'`
	port1_guid="0x$port1_guid"
	port1_guid=`printf "0x%016x" $port1_guid`
	$opensm -g $port1_guid -B

	# Get second ports' guid on the card and run opensm on it if it exists.
	if [ $phys_port_cnt -eq 2 ]
	then
		port2_guid=` echo "ibase=16;$node_guid + 2" | bc`
		port2_guid=` echo "obase=16;$port2_guid"    | bc`
		port2_guid=` echo $port2_guid | sed -e 'y/ABCDEF/abcdef/'`
		port2_guid="0x$port2_guid"
		port2_guid=`printf "0x%016x" $port2_guid`
		$opensm -g $port2_guid -B
	fi
else
	echo "$opensm not exist!"
fi

# Starting up iscsi driver and daemon
modprobe iscsi_trgt
ps ax | grep ietd | grep -v grep 2>&1 > /dev/null
if [ $? -ne 0 ]
then
	ietd
fi

# Restore vdisk info on the server
/usr/sbin/sanager/VdiskInfoRestore.sh

# Restore group info on the server
/usr/sbin/sanager/GroupInfoRestore.sh
