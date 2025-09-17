#!/usr/bin/bash
#
# this script tests if the sorts are less than one second
# each for large list sizes.
#
# The script runs in less than 2 seconds on an apollo machine
#
# if this script is not executable then do
#     chmod +x longrun.sh
#
# MP3 place in student directory
if [ ! -f ./geninput ]; then
    if [ -f ./geninput.c ]; then
        gcc geninput.c -o geninput
    else
        echo "geninput.c file not found! Check why not in repository!"
        exit 1
    fi
fi
echo "this script should run in less than 2 seconds"
seed=09292025
./geninput 10000 1 1 gen $seed | ./lab3 
./geninput 10000 1 2 ip $seed | ./lab3 
./geninput 10000 1 3 gen $seed | ./lab3 
./geninput 300000 1 4 ip $seed | ./lab3  
./geninput 500000 1 5 gen $seed | ./lab3  

