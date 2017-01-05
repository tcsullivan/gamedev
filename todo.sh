#!/bin/bash

#
# Searches for all TODOs and tosses them in a file.
#
TODO_COUNT=0
rm -f TODOS
touch TODOS
for file in include/*.hpp
do
	echo "########################################" >> TODOS
	echo $file >> TODOS
	echo "========================================" >> TODOS
	grep -n -B 5 -A 5 "TODO" $file | sed s/--/========================================/g >> TODOS
	TODO_COUNT=$((TODO_COUNT+$(grep -c "TODO" $file)))
done

for file in src/*.cpp
do
	echo "########################################" >> TODOS
	echo $file >> TODOS
	echo "========================================" >> TODOS
	grep -n -B 5 -A 5 "TODO" $file | sed s/--/========================================/g >> TODOS
	TODO_COUNT=$((TODO_COUNT+$(grep -c "TODO" $file)))
done

echo "Found" $TODO_COUNT "TODOs."
