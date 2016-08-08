#!/bin/bash

PEER=i3c.wittgenstein@platon
DEVICE=0x24
I3CCLI=./i3c_cli

function jsonval {
	temp=$(echo $json | sed -e 's/\\\\\//\//g' -e 's/[{}]//g' \
	                  | awk -v k="text" '{n=split($0,a,","); for (i=1; i<=n; i++) print a[i]}' \
	                  | sed -e 's/\"\:\"/\|/g' -e 's/[\,]/ /g' -e 's/\"//g' \
	                  | grep -w $prop)
	echo ${temp##*|}
}

function space_is_open {
	json=$(curl -s http://spaceapi.n39.eu/json)
	prop='open'
	
	st=$(jsonval)
	isopen=$(echo "$st" | sed -e 's/.*open:\(.*\)/\1/')
	
	echo "$isopen"
}

function space_open {
  ret=$(curl -s http://wittgenstein/open)
  
  echo "$ret"
}

function space_close {
  ret=$(curl -s http://wittgenstein/close)
  
  echo "$ret"
}

function get_switches {
	ret="0x00"
	while [[ "$ret" == "0x00" ]]; do
		ret=$($I3CCLI $PEER $DEVICE 0x1)
		echo $ret
	done
}

function reset_i3c {
	ret="0x00"
	while [[ "$ret" == "0x00" ]]; do
		ret=$($I3CCLI $PEER $DEVICE 0x0)
		echo $ret
	done
}

# stored switch state
known_sw=""

while [[ true ]]; do
  #reset an I3C state
#  if [[ "$(reset_i3c)" -ne "0x01" ]]; then
#    echo "Could not reset the I3C INT!"
#  fi
  
  # get current switch state
  sw=$(get_switches)
  echo "$(date "+%Y-%m-%d %H:%M:%S") Switch Status is $sw"

  if [[ "$known_sw" -ne "$sw" ]]; then
    echo "New State!"
    known_sw=$sw
    
    # Get the space status
    status=$(space_is_open)
    echo "SpaceStatus: $status"
    
    # if close switch issued
    if [[ "$sw" == "0x01" ]]; then
      # close if open
      if [[ "$status" == "true" ]]; then
	echo "Closing …"
	space_close
      fi  
    fi

    # if open switch issued
    if [[ "$sw" == "0x02" ]]; then
      # open if closed
      if [[ "$status" == "false" ]]; then
	echo "Opening …"
	space_open
      fi  
    fi
    
  fi # new state

  sleep 2
done
