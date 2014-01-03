#!/bin/bash

# HTTP-Headers
echo "Content-Type: text/plain"
echo "Access-Control-Allow-Origin: *"
echo

./color.sh greenblink
./spaceapi.sh true

ln -fs open.png ../state.png
touch ../open.png

./tweet.sh "#Spacetime im @netz39! ($(date "+%Y-%m-%d %H:%M"))"

echo "/var/www/cgi-bin/color.sh green" | at now + 5 minutes

./color.sh green