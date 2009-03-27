#!/bin/bash

rmmod bonding >/dev/null
modprobe bonding >/dev/null
/etc/init.d/network restart
