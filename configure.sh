#!/bin/bash

# creating lib directory depending on machine type: 32bits or 64bits
make_rule="./make.rules"

if [ -e $make_rule ]
then
	rm $make_rule -rf
fi

smp=`uname -a | grep x86_64 | wc -l`

touch $make_rule

if [ $smp -eq 1 ]
then
	echo "LIB_DIR   = /lib64/" > $make_rule
else
	echo "LIB_DIR   = /lib/"  > $make_rule
fi

# creating top working directory
if [ ! -d /usr/sbin/sanager ]
then
	mkdir /usr/sbin/sanager
fi
