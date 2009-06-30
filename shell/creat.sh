#!/bin/bash
#echo $1
if [ $# -le 0 ]
then
	echo "Usage: ./create.sh date"
	exit 0;
fi

tarball="llww_$1.tgz"

if [ -f $tarball ]
then
	rm $tarball -rf
fi

tar zcvf  $tarball llww/ --exclude llww/.git 2>&1 >/dev/null
