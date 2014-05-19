#!/bin/bash

## Functions for Space API handling in BASH
#
# Author: Tux <tux@netz39.de>


SPACEAPI_URL=http://spaceapi.n39.eu/json

function jsonval {
	temp=$(echo $json | sed -e 's/\\\\\//\//g' -e 's/[{}]//g' \
	                  | awk -v k="text" '{n=split($0,a,","); for (i=1; i<=n; i++) print a[i]}' \
	                  | sed -e 's/\"\:\"/\|/g' -e 's/[\,]/ /g' -e 's/\"//g' \
	                  | grep -w $prop)
	echo ${temp##*|}
}

function space_is_open {
	json=$(curl -s $SPACEAPI_URL)
	prop='open'
	
	st=$(jsonval)
	isopen=$(echo "$st" | sed -e 's/.*open:\(.*\)/\1/')
	
	echo "$isopen"
}