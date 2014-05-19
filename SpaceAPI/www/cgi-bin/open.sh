#!/bin/bash

# Includes (see http://stackoverflow.com/a/12694189)
DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$DIR" ]]; then DIR="$PWD"; fi
source "$DIR/apifunctions.sh"


# HTTP-Headers
echo "Content-Type: text/plain"
echo "Access-Control-Allow-Origin: *"
echo


./color.sh greenblink
./spaceapi.sh true

ln -fs open.png ../state.png
touch ../open.png

# only if not already open
isopen=$(space_is_open)
if [ "$isopen" != "true" ]; then

  ./tweet.sh "#Spacetime im @netz39! ($(date "+%Y-%m-%d %H:%M"))"

else

  echo "Already open!"

fi
  
echo "/var/www/cgi-bin/color.sh green" | at now + 5 minutes
./color.sh green
