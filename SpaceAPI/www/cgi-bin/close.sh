#!/bin/bash

# HTTP-Headers
echo "Content-Type: text/plain"
echo "Access-Control-Allow-Origin: *"
echo

./color.sh redblink
./spaceapi.sh false

ln -fs closed.png ../state.png
touch ../closed.png

./tweet.sh "Das @netz39 beendet die #Spacetime! ($(date "+%Y-%m-%d %H:%M"))"

echo
echo 
echo "Ampel wird in 5 Minuten ausgeschaltet."

echo "/var/www/cgi-bin/color.sh none" | at now + 5 minutes

./color.sh red
