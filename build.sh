#!/bin/bash

idate=$(date +"%s" -r .git/index)
odate=$(date +"%s" -r ./.updated)

echo $idate
echo $odate

if (( $idate > $odate )); then
	echo "frig"
	touch ./.updated
fi
