#!/bin/bash

date

for(( i = 100; i< 111; i++))
do
    {
        ./build/testTcpClient $i
    }&

done
