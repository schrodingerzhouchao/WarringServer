#!/bin/bash

date

for(( i = 100; i< 120; i++))
do
    {
	    ./testLoginClient $i 1 $i
    }&

done
