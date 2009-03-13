#! /bin/bash

SubAction()
{
	. /etc/init.d/functions
	action $"hahahaah: " ./CheckIBRunningEnv.sh all
}

SubBondDevice()
{
	ifdown eth0
	ifdown eth1
	ifdown eth2
	ifdown eth3

	ifconfig bond0 hw ether 00:0c:29:a9:76:ad
	ifconfig bond0 192.168.1.66 up

	ifenslave bond0 eth0
	ifenslave bond0 eth1
	ifenslave bond0 eth2
	ifenslave bond0 eth3

	return 0;
}

SubBondDevice;

exit $?;

