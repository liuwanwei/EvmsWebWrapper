#!/bin/bash

# Get IB card's node guid(command "bc" requires Uppercase hex number)
node_guid=` ibv_devinfo | grep node_guid | awk '{print $2}' | sed -e 's/://g' | sed -e 'y/abcdef/ABCDEF/'`

phys_port_cnt=` ibv_devinfo | grep phys_port_cnt | awk '{print $2}'`

# Get first port's guid on the card and run opensm on it.
port1_guid=` echo "ibase=16;$node_guid + 1" | bc`
port1_guid=` echo "obase=16;$port1_guid"    | bc`
port1_guid=` echo $port1_guid | sed -e 'y/ABCDEF/abcdef/'`
port1_guid="0x$port1_guid"
port1_guid=`printf "0x%016x" $port1_guid`
opensm -g $port1_guid -B

# Get second ports' guid on the card and run opensm on it if it exists.
if [ $phys_port_cnt -eq 2 ]
then
	port2_guid=` echo "ibase=16;$node_guid + 2" | bc`
	port2_guid=` echo "obase=16;$port2_guid"    | bc`
	port2_guid=` echo $port2_guid | sed -e 'y/ABCDEF/abcdef/'`
	port2_guid="0x$port2_guid"
	port2_guid=`printf "0x%016x" $port2_guid`
	opensm -g $port2_guid -B
fi

