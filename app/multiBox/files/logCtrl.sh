#!/bin/sh
echo /dev/console > /data/log_path
killall -USR1 uartd
echo 111100 > /data/log_ctrl
killall -USR2 uartd