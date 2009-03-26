#!/bin/bash

dir=/llww/pl

ls -h -p -s -1 $dir | grep -E "^[ 0-9]*\.*\w*[KMGT]"
