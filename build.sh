#!/bin/bash

idate=$(date +"%s" -r .git/index)
odate=$(date +"%s" -r ./.updated)

if (( $idate > $odate )); then
	echo "Project updated, building all..."
	make clean
	make -j4
	touch ./.updated
else
	echo "Up to date, building..."
	make -j4
fi
