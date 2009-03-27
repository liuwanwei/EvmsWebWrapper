#!/bin/bash

cp Iscsi.pl /usr/sbin/sanager/ -rf

#dir=/llww/pl

#ls -h -p -s -1 $dir | grep -E "^[ 0-9]*\.*\w*[KMGT]"

#./client.pl -t iscsi -s AddTarget "lun=3|iotype=fileio|name=haha|path=/dev/sdb1"
#./client.pl -t iscsi -s GetAllTargets
./client.pl -t iscsi -s TargetCtrlList
#./client.pl -t iscsi -s DelTarget "name=haha"
#./client.pl -t iscsi -s TargetAccessCtrl "type=allow|name=iqn.2001-04.com.example:storage.disk2.sys1.xyz|ctrl=192.168.3.0/[255.255.0.0]"




# bonding related testing commands
#./Bonding.pl ModBonding "device=bond0|mode=3|miimon=200"
#./Bonding.pl ModBonding "device=bond1|ip=192.168.1.67|mask=255.255.255.0"
#./Bonding.pl AddBonding "device=bond1|ip=192.168.1.66|mask=255.255.255.0|mode=2|miimon=100|slave=eth2|slave=eth3"
#./Bonding.pl DelBonding "device=bond1"
#./client.pl -t bonding -s GetAllBondings
#./client.pl -t bonding -s ModBonding "device=bond1|ip=192.168.1.67"
#./client.pl -t bonding -s DelBonding "device=bond1"
#./client.pl -t bonding -s AddBonding "device=bond1|ip=192.168.1.66|mask=255.255.255.0|mode=2|miimon=100|slave=eth2|slave=eth3"

#rmmod bonding
#modprobe bonding
#/etc/init.d/network restart
