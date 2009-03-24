#!/bin/bash

# add device to group
if [ $1 = "add" ]
then
	./client.pl -t 3 -s 36 device=/vdisk/10G lun=1 group=Default
fi

# del device from group
if [ $1 = "del" ]
then
	./client.pl -t 3 -s 37 device=vdisk1 group=Default
fi
