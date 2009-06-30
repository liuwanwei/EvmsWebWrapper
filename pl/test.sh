#!/bin/bash

cp Iscsi.pl /usr/sbin/sanager/ -rf

#dir=/llww/pl

#ls -h -p -s -1 $dir | grep -E "^[ 0-9]*\.*\w*[KMGT]"

#./client.pl -t iscsi -s AddTarget "lun=3|iotype=fileio|name=haha|path=/dev/hdb4"
#./client.pl -t iscsi -s GetAllTargets
./client.pl -t iscsi -s DelTarget "name=iqn.2001-04.com.example:storage.disk2.sys1.xyz"
