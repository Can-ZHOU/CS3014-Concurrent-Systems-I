#!/bin/bash

# Test Case One [16, 16, 1, 32, 32, 1000]
config="16 16 1 32 32 1000"
length=50

RESULT_FILE=16_16_1_32_32_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done


# Test Case Two [16, 16, 1, 32, 64, 1000]
config="16 16 1 32 64 1000"
length=50

RESULT_FILE=16_16_1_32_64_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done


# Test Case Three [16, 16, 1, 32, 128, 1000]
config="16 16 1 32 128 1000"
length=50

RESULT_FILE=16_16_1_32_128_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done


# Test Case Four [16, 16, 1, 32, 256, 1000]
config="16 16 1 32 256 1000"
length=50

RESULT_FILE=16_16_1_32_256_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done


# Test Case Five [16, 16, 1, 32, 512, 1000]
config="16 16 1 32 512 1000"
length=50

RESULT_FILE=16_16_1_32_512_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done


# Test Case Six [16, 16, 1, 32, 1024, 1000]
config="16 16 1 32 1024 1000"
length=50

RESULT_FILE=16_16_1_32_1024_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

# Test Case Seven [16, 16, 1, 32, 2048, 1000]
config="16 16 1 32 2048 1000"
length=50

RESULT_FILE=16_16_1_32_2048_1000.txt

echo "Running on: $(hostname). Values are in microseconds." >> $RESULT_FILE
echo "Test running using parameters \"$config\" for $length iterations:" >> $RESULT_FILE

echo "David's Code" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_David $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with SSE" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_SSE $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done

echo "My Code with OpenMP Without if" >> $RESULT_FILE
run=1
while [[ $run -le $length ]]
do
	res=`./conv-harness_OpenMP_Without_If $config | grep Team`
	if ! [[ $res =~ ^.*\ ([0-9]+)\ .*$ ]] ; then
		echo $res
	else
		echo ${BASH_REMATCH[1]} >> $RESULT_FILE
	fi
	((run = run + 1))
done