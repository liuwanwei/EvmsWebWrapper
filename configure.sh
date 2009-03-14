#!/bin/bash

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
