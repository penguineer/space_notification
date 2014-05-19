#!/bin/bash

# Includes (see http://stackoverflow.com/a/12694189)
DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$DIR" ]]; then DIR="$PWD"; fi
source "$DIR/apifunctions.sh"


# HTTP-Headers
echo "Content-Type: text/plain"
echo "Access-Control-Allow-Origin: *"
echo


./color.sh redblink
./spaceapi.sh false

ln -fs closed.png ../state.png
touch ../closed.png

# only if open
isopen=$(space_is_open)
if [ "$isopen" == "true" ]; then

  ./tweet.sh "Das @netz39 beendet die #Spacetime! ($(date "+%Y-%m-%d %H:%M"))"

  echo
  echo 
  echo "Ampel wird in 5 Minuten ausgeschaltet."

  echo "/var/www/cgi-bin/color.sh none" | at now + 5 minutes

else

  echo "Already closed!"

fi

./color.sh red
