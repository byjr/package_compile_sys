#!/bin/sh
PIN_NUM=$1
PIN_IO=$2
PIN_DIR=$3
PIN_VAL=$4
if [ X"$PIN_IO" = X"set" ];then
	if [ ! -e /sys/class/gpio/gpio$PIN_NUM ];then
		echo gpio:$PIN_NUM undefined export it.
		echo $PIN_NUM > /sys/class/gpio/export
	fi
	if [ X"$PIN_DIR" = X"out" ];then
		echo "set gpio:$PIN_NUM direction to out"
		echo out > /sys/class/gpio/gpio$PIN_NUM/direction
		if [ X"$PIN_VAL" != X ];then
				# echo "set gpio:$PIN_NUM value to $PIN_VAL"
				echo $PIN_VAL > /sys/class/gpio/gpio$PIN_NUM/value
		fi
	else
		echo "set gpio:$PIN_NUM direction to in"
		echo in > /sys/class/gpio/gpio$PIN_NUM/direction
	fi	
fi
cur_val=`cat /sys/class/gpio/gpio$PIN_NUM/value`
echo current gpio$PIN_NUM value:$cur_val
