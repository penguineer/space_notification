#!/bin/bash

cat spaceapi.template | sed -e s/\\\$\\\$\\\$isOpen\\$\\$\\$/$1/ \
                        -e s/\\\$\\\$\\\$timeMillis\\\$\\\$\\\$/$(date +%s)/ \
                        > ../spaceapi.json
