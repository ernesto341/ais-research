#!/bin/bash

while [ 1 ]
do
        nc -l -p 80 -v >> httprequests.dat
done

