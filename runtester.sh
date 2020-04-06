#!/bin/bash

config="16 16 1 32 32 20"
length=50

RESULT_FILE=zcresults.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

run=1
while [[ $run -le $length ]]
do
	echo "Running iteration $run out of $length."
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done