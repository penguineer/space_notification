#!/bin/sh

PEER=i3c.wittgenstein@platon
DEVICE=0x20
#gpio load i2c

case "$1" in
  none)
#    i2cset -y 1 0x20 0x20
    ./i3c_cli $PEER $DEVICE 0x2 0x0 
  ;;
  
  red)
#    i2cset -y 1 0x20 0x21
    ./i3c_cli $PEER $DEVICE 0x2 0x1 
  ;;
  
  green)
#    i2cset -y 1 0x20 0x22
    ./i3c_cli $PEER $DEVICE 0x2 0x2 
  ;;
  
  redblink)
#    i2cset -y 1 0x20 0xa9
    ./i3c_cli $PEER $DEVICE 0x2 0x9 
  ;;
  
  greenblink)
#    i2cset -y 1 0x20 0xaa
    ./i3c_cli $PEER $DEVICE 0x2 0xa 
  ;;
  
  *)
  echo "Usage: red|green|redblink|greenblink"
  ;;
esac  

echo "$1"
