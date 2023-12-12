#!/bin/bash

date

for(( i = 1; i< 20; i++))
do
    {
        ./build/testLoginClient $i 10
    }&

done
