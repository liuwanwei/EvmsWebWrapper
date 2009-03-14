#! /bin/bash

process_id=`ps ax|grep opensm | grep -v grep | awk '{print $1}'`

echo $process_id

for i in $process_id
do
	echo $i
done


date=`date`
echo $date
