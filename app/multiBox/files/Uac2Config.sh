#!/bin/sh
cfg_path=/sys/kernel/config
work_name=amlogic
ist_path=$cfg_path/usb_gadget/$work_name
mount -t configfs none $cfg_path
mkdir $ist_path
cd $ist_path
echo "0x1d6b" > idVendor
echo "0x0104" > idProduct
mkdir strings/0x409
echo "0123456789" > strings/0x409/serialnumber
echo "Acousticlink" > strings/0x409/manufacturer
echo "USB Gadget" > strings/0x409/product

mkdir configs/c.1
mkdir configs/c.1/strings/0x409
echo "USB Gadget" > configs/c.1/strings/0x409/configuration

mkdir functions/uac2.usb0
ln -s functions/uac2.usb0 configs/c.1
#ls /sys/class/udc
echo "ff400000.dwc2_a" > UDC