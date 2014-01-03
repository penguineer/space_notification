#!/bin/sh

gpio load i2c

case "$1" in
  none)
    i2cset -y 1 0x20 0x20
  ;;
  
  red)
    i2cset -y 1 0x20 0x21
  ;;
  
  green)
    i2cset -y 1 0x20 0x22
  ;;
  
  redblink)
    i2cset -y 1 0x20 0x29
  ;;
  
  greenblink)
    i2cset -y 1 0x20 0x2a
  ;;
  
  *)
  echo "Usage: red|green|redblink|greenblink"
  ;;
esac  

echo "$1"
