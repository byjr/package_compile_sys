#!/bin/sh

function get_wifi_info(){
    if [ X"$1" = X"sta" ];then
        if [ X"$2" = X ] || [ X"$3" = X ];then
            GOT_SSID=ASUS_8528_2G
            GOT_PSK=test12345678
        else
            GOT_SSID=$2
            GOT_PSK=$3
        fi
        echo -----------------------GOT_PSK:$GOT_PSK
        echo -----------------------GOT_SSID:$GOT_SSID
    fi
}
function start_sta(){
    ifconfig wlan0 up
    wpa_supplicant -Dnl80211 -c /etc/wpa_supplicant.conf -iwlan0 -B
    wpa_cli -iwlan0 scan
    wpa_cli -iwlan0 scan_r
    wpa_cli -iwlan0 add_network
    wpa_cli -iwlan0 set_network 0 ssid \"$GOT_SSID\"
    wpa_cli -iwlan0 set_network 0 psk \"$GOT_PSK\"
    wpa_cli -iwlan0 select_network 0
    udhcpc -n -t 10 -i wlan0
    wpa_cli -iwlan0 status
    [ $? = 0 ] && echo "station OK" || echo "station FAIL"
}
function start_ap(){
    ifconfig lo 127.0.0.1 netmask 255.255.255.0
    ifconfig uap0 192.168.100.1 netmask 255.255.255.0
    dnsmasq -C /etc/dnsmasq.conf &
    if [ $? -ne 0 ];then
        return 1;
    fi
    hostapd /etc/hostapd.conf -B
    [ $? = 0 ] && echo "AP OK" || echo "AP FAIL"
}
function stop_sta(){
    killall wpa_supplicant
    killall -9 udhcpc 2> /dev/null
}
function stop_ap(){
    killall hostapd
    killall -9 dnsmasq 2> /dev/null
}
case "$1" in
    sta)
        "$0" stop_sta
        sleep 1
		get_wifi_info && \
        start_sta
        ;;
    ap)
        "$0" stop_ap
        sleep 1
        start_ap    
        ;;
    wsp)
        start_sta;start_ap
        ;;
    stop_ap)
        stop_ap
        ;;
    stop_sta)
        stop_sta
        ;;
    stop_wsp)
        stop_ap;stop_sta
        ;;
    *)
        echo "Usage: $0 {sta|ap|wsp|stop_...}"
        exit 1
esac

exit $?