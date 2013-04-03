#!/bin/bash

cat spaceapi.json | sed -e s/\\\$\\\$\\\$isOpen\\$\\$\\$/$1/ \
                        -e s/\\\$\\\$\\\$timeMillis\\\$\\\$\\\$/$(date +%s000)/
