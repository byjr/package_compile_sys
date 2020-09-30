#!/bin/sh

# SRC_DEVICE='plughw:0,1,0'
SRC_DEVICE='plughw:1,0'

stopPlayer(){
	echo $0 stopPlayer ...
	killall -9 I2sToUac UacToPhy RtPlayer 2>> /dev/null
	GpioCtrl.sh 452 set out 0
}

startLvds(){
	echo $0 startLvds ...
	GpioCtrl.sh 452 set out 1
}

startUac(){
	echo $0 startUac ...
	GpioCtrl.sh 452 set out 0
	I2sToUac -i $SRC_DEVICE -o uac_card -m 4 &
	UacToPhy -i uac_card -o default -m 4 &
}

startSbar(){
	echo $0 startSbar ...
	GpioCtrl.sh 452 set out 0
	RtPlayer -i $SRC_DEVICE -o plughw:1,1 -m 4 &
}

closePa(){
	echo $0 closePa ...
	amixer set 'tas5805m_2c Mute Control' 0 1 >> /dev/null
	amixer set 'tas5805m_2e Mute Control' 0 1 >> /dev/null
}

openPa(){
	echo $0 openPa ...
	amixer set 'tas5805m_2c Mute Control' 0 0 >> /dev/null
	amixer set 'tas5805m_2e Mute Control' 0 0 >> /dev/null
}

case "$1" in
	uac)stopPlayer
		openPa
		startUac
		;;
	lvds)stopPlayer
		openPa
		startLvds
		;;
	sbar)stopPlayer
		 openPa
		 startSbar
		;;
	stop) stopPlayer
		;;
	open) openPa
		;;
	close) closePa
		;;
  *)
	echo "Usage: $0 {uac|lvds|sbar|open|close}"
	exit 1
esac