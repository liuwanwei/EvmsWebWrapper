#!/bin/bash

dir="/mnt/"

echo /root/v2 | grep -E "^$dir" | wc -l
