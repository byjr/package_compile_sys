/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <csignal>
#include "LoggerUtils/DcsSdkLogger.h"
#include "DCSApp/DuerLinkMtkInstance.h"
#include "DCSApp/Configuration.h"
#include "DCSApp/DeviceIoWrapper.h"
#include "duer_link/duer_link_sdk.h"
#include "DCSApp/ApplicationManager.h"
#include <DeviceTools/PrintTickCount.h>
#include <DeviceTools/Utils.h>
#include "DCSApp/AppThreadName.h"
#include "DCSApp/SoundController.h"


namespace duerOSDcsApp {
namespace application {

using std::string;
using std::vector;
using std::ifstream;

static string m_target_ssid;
static string m_target_pwd;
static string m_target_ssid_prefix;
static bool m_ble_is_opened = false;
static volatile int m_ping_interval = 1;
static volatile int m_network_status = 0;
static volatile bool m_pinging = false;

DuerLinkMtkInstance* DuerLinkMtkInstance::m_duerLink_mtk_instance = nullptr;
duerLink::NetworkConfigStatusObserver *DuerLinkMtkInstance::m_pMtkConfigObserver = nullptr;;

bool system_command(const char* cmd) {trc(__func__);
    pid_t status = 0;
    bool ret_value = false;

    APP_INFO("System [%s]", cmd);

    status = system(cmd);

    if (-1 == status) {
        APP_ERROR("system failed!");
    } else {
        if (WIFEXITED(status)) {
            if (0 == WEXITSTATUS(status)) {
                ret_value = true;
            } else {
                APP_ERROR("System shell script failed:[%d].", WEXITSTATUS(status));
            }
        } else {
            APP_INFO("System status = [%d]", WEXITSTATUS(status));
        }
    }

    return ret_value;
}

bool check_ap0_interface_status(string ap) {trc(__func__);
    int sockfd;
    bool ret = false;
    struct ifreq ifr_mac;

    if ((sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        APP_ERROR("socket create failed.");
        return false;
    }

    memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, ap.c_str(), sizeof(ifr_mac.ifr_name)-1);

    if ((ioctl(sockfd, SIOCGIFHWADDR, &ifr_mac)) < 0) {
        APP_ERROR("Mac ioctl failed.");
    } else {
        APP_INFO("Mac ioctl suceess.");
        ret = true;
    }
    close(sockfd);

    return ret;
}

bool tmp_wpa_conf(const string ssid, const string pwd, const string path) {trc(__func__);
    string tmp_conf_path;
    string bk_conf_path;
    string file_conf_path;

    if (ssid.empty() || pwd.empty() || path.empty()){
        APP_ERROR("Input param is empty.");
        return false;
    }

    tmp_conf_path = path + ".tmp";
    bk_conf_path = path + ".bk";
    file_conf_path = path;

    //如果空密码,修改此处
    string insert_ct = NETWORK_WPA_CONF_HEAD;
    insert_ct += ssid;
    insert_ct += NETWORK_WPA_CONF_MIDDLE;
    insert_ct += pwd;
    insert_ct += NETWORK_WPA_CONF_END;

    APP_INFO("Wpa_supplicant.conf insert CT [%s]", insert_ct.c_str());

    deviceCommonLib::deviceTools::printTickCount("network_config cp wpa.conf begin.");

    string cmd_buff = "cp ";
    cmd_buff += file_conf_path + " " + bk_conf_path;
    cmd_buff += " &";

    if (!system_command(cmd_buff.c_str())) {
        APP_ERROR("Bake Wpa_supplicant.conf failed.");
        return false;
    }

    deviceCommonLib::deviceTools::printTickCount("network_config cp wpa.conf end.");

    sync();

    deviceCommonLib::deviceTools::printTickCount("network_config sync1 wpa.conf end.");

    APP_INFO("Bake Wpa_supplicant.conf successed.");

    string line;
    std::ifstream in(file_conf_path);
    std::ofstream out(tmp_conf_path);

    while (getline(in, line)) {
        if (strstr(line.c_str(), DUERLINK_WPA_INSERT_FLAG)){
            out << line << std::endl;
            out << insert_ct << std::endl;
            break;
        } else {
            out << line << std::endl;
        }
    }

    in.close();
    out.close();

    deviceCommonLib::deviceTools::printTickCount("network_config check wpa.conf end.");

    cmd_buff.clear();
    cmd_buff = "mv ";
    cmd_buff += tmp_conf_path + " " + file_conf_path;
    cmd_buff += " &";

    if (!system_command(cmd_buff.c_str())) {
        APP_ERROR("Update wpa_supplicant.conf failed.");
        return false;
    }

    deviceCommonLib::deviceTools::printTickCount("network_config update wpa.conf end.");

    sync();

    deviceCommonLib::deviceTools::printTickCount("network_config sync2 wpa.conf end.");

    APP_INFO("Update wpa_supplicant.conf successed.");

    return true;
}

#if defined(CONFIG_PANTHER)
bool config_and_start_wpa_supplicant(const string ssid, const string pwd) {trc(__func__);

    if (ssid.empty() || pwd.empty()){
        APP_ERROR("Input param is empty.");
        return false;
    }

    deviceCommonLib::deviceTools::printTickCount("network_config connect_ap begin.");

    string tmp = "/lib/wdk/omnicfg_apply 9 ";
    tmp += ssid;
    tmp += ' ';
    tmp += pwd;

    APP_INFO("Config wifi, use %s.", tmp.c_str());

    system_command(tmp.c_str());

    deviceCommonLib::deviceTools::printTickCount("network_config connect_ap end.");

    return true;
}
#endif
bool start_wpa_supplicant() {trc(__func__);

    deviceCommonLib::deviceTools::printTickCount("network_config connect_ap begin.");

    system_command("echo 1 > /dev/wmtWifi &");

    sleep(1);

    system_command("/usr/bin/connect_ap.sh &");

    deviceCommonLib::deviceTools::printTickCount("network_config connect_ap end.");

    return true;
}

bool stop_wpa_supplicant() {trc(__func__);
#if defined(CONFIG_PANTHER)
    return true;
#else
    return system_command("killall wpa_supplicant &");
#endif
}

bool set_ap_tmp_ipaddr() {trc(__func__);
    if (!check_ap0_interface_status(DUERLINK_NETWORK_DEVICE_MTK_FOR_AP)) {
        APP_INFO("%s is down.", DUERLINK_NETWORK_DEVICE_MTK_FOR_AP);
        return false;
    }

#if defined(CONFIG_PANTHER)
    return system_command("ifconfig br0 192.168.0.1 netmask 255.255.255.0 &");
#else
    return system_command("ifconfig ap0 192.168.1.1 netmask 255.255.255.0 &");
#endif
}

bool delete_tmp_addr() {trc(__func__);
    if (!check_ap0_interface_status(DUERLINK_NETWORK_DEVICE_MTK_FOR_AP)) {
        APP_INFO("%s is down.", DUERLINK_NETWORK_DEVICE_MTK_FOR_AP);
        return true;
    }

#if defined(CONFIG_PANTHER)
    return system_command("ip addr delete 192.168.0.1 dev br0 &");
#else
    return system_command("ip addr delete 192.168.1.1 dev ap0 &");
#endif
}

#if !defined(CONFIG_PANTHER)
bool echo_ap_to_devwifi() {
    return system_command("echo AP > /dev/wmtWifi &");
}
#endif

#if defined(CONFIG_PANTHER)
void DuerLinkMtkInstance::clear_network_history() {

    APP_INFO("clear network history");

    system_command("/usr/bin/clear_network_history.sh &");

    sleep(6); // wait for clear history and roll back
}
#endif
bool stop_ap_mode() {trc(__func__);
#if defined(CONFIG_PANTHER)
    return true;
#else
    return system_command("killall hostapd &");
#endif
}

bool start_ap_mode() {trc(__func__);
#if defined(CONFIG_PANTHER)
    string ap_hstapd_pid_path = DUERLINK_WPA_HOSTAPD_PID_FILE_PATH_MTK;
    string pid_content;

    std::ifstream in(ap_hstapd_pid_path.c_str());
    getline(in, pid_content);
    in.close();

    string tmp = "kill -HUP ";
    tmp += pid_content;

    APP_INFO("[start_ap_mode] do %s.", tmp.c_str());

    return system_command(tmp.c_str());
#else
    string cmd = " ";
    cmd += DUERLINK_WPA_HOSTAPD_CONFIG_FILE_PATH_MTK;
    cmd += " &";

    if (stop_ap_mode()) {
        APP_INFO("[start_ap_mode] hostapd is ruuning.");
    }
    sleep(1);

    string tmp = "hostapd ";
    tmp += cmd;

    return system_command(tmp.c_str());
#endif
}

bool starup_ap_interface() {trc(__func__);
    if (check_ap0_interface_status(DUERLINK_NETWORK_DEVICE_MTK_FOR_AP)) {
        APP_INFO("%s is up.", DUERLINK_NETWORK_DEVICE_MTK_FOR_AP);

        return true;
    }

#if defined(CONFIG_PANTHER)
    return system_command("ifconfig wlan0 up &");
#else
    return system_command("ifconfig ap0 up &");
#endif
}

bool down_ap_interface() {trc(__func__);
    if (!check_ap0_interface_status(DUERLINK_NETWORK_DEVICE_MTK_FOR_AP)) {
        APP_INFO("%s is down.", DUERLINK_NETWORK_DEVICE_MTK_FOR_AP);
        return true;
    }

#if defined(CONFIG_PANTHER)
    return system_command("ifconfig wlan0 up &");
#else
    return system_command("ifconfig ap0 down &");
#endif
}

bool starup_wlan0_interface() {trc(__func__);

#if defined(CONFIG_PANTHER)
    return system_command("ifconfig sta0 up &");
#else
    return system_command("ifconfig wlan0 up &");
#endif
}

bool down_wlan0_interface() {trc(__func__);
    if (!check_ap0_interface_status(DUERLINK_NETWORK_DEVICE_MTK_FOR_AP)) {
        APP_INFO("%s is down.", DUERLINK_NETWORK_DEVICE_MTK_FOR_AP);
        return true;
    }

#if defined(CONFIG_PANTHER)
    system_command("ifconfig sta0 0.0.0.0");

    return system_command("ifconfig sta0 down &");
#else
    system_command("ifconfig wlan0 0.0.0.0");

    return system_command("ifconfig wlan0 down &");
#endif
}

bool stop_dhcp_server() {trc(__func__);
    return system_command("killall dnsmasq &");
}

bool start_dhcp_server() {trc(__func__);
    if (stop_dhcp_server()) {
        APP_INFO("[Start_dhcp_server] dnsmasq is killed.");
    }
    sleep(1);

    return system_command("dnsmasq &");
}

bool change_hostapd_conf(const string ssid) {trc(__func__);
    string conf_ct_all = NETWORK_SSID_CONFIG_HEAD;
    conf_ct_all += ssid;
    conf_ct_all += "\n";
    conf_ct_all += NETWORK_SSID_CONFIG_MIDDLE;
    conf_ct_all += "\n";
    conf_ct_all += NETWORK_SSID_CONFIG_END;

    APP_INFO("Hostapd:\n%s", conf_ct_all.c_str());

    string ap_hstapd_temp_confile_path = DUERLINK_WPA_HOSTAPD_CONFIG_FILE_PATH_MTK;
    ap_hstapd_temp_confile_path += ".tmp";

    std::ofstream out(ap_hstapd_temp_confile_path.c_str());
    out << conf_ct_all << std::endl;
    out.close();

    string command_mv_string = ap_hstapd_temp_confile_path + " " + DUERLINK_WPA_HOSTAPD_CONFIG_FILE_PATH_MTK;
    string tmp = "mv ";
    tmp += command_mv_string;
    tmp += " &";

    return system_command(tmp.c_str());
}

bool get_device_interface_mac(string &mac_address) {trc(__func__);
    int sock_mac;
    struct ifreq ifr_mac;
    char mac_addr[30] = {0};

    sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
    if (sock_mac == -1) {
        APP_ERROR("create mac socket failed.");
        return false;
    }

    memset(&ifr_mac,0,sizeof(ifr_mac));
    strncpy(ifr_mac.ifr_name, DUERLINK_NETWORK_DEVICE_MTK_FOR_WORK, sizeof(ifr_mac.ifr_name)-1);

    if ((ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0) {
        APP_ERROR("Mac socket ioctl failed.");
        close(sock_mac);
        return false;
    }

    sprintf(mac_addr,"%02X%02X",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);

    APP_INFO("local mac:%s",mac_addr);

    close(sock_mac);

    mac_address = mac_addr;

    std::transform(mac_address.begin(), mac_address.end(), mac_address.begin(), toupper);

    return true;
}

void set_softAp_ssid_and_pwd(string hostapd_config_file_path) {trc(__func__);
    // hostapd configuration file path
    string m_ap_hostapd_conffile_path = hostapd_config_file_path;

    string ssid;
    string suffix;
    string mac_address;

    m_target_ssid_prefix = Configuration::getInstance()->getSsidPrefix();

    APP_INFO("Configure SSID Prefix: [%s].", m_target_ssid_prefix.c_str());

    get_device_interface_mac(mac_address);
    suffix += mac_address;

    ssid = m_target_ssid_prefix + suffix;
    change_hostapd_conf(ssid);

    return ;
}
string raw_ssid;
bool softap_prepare_env_cb() {trc(__func__);
//	my_popen("kiallall andlink > /dev/null 2>&1 ; andlink -l111 &");
	my_popen("cdb set op_work_mode 1 && mtc commit");
#if 0
	char buf[1024]="";
	cdb_get_str("$wl_bss_ssid1",buf,sizeof(buf),"");
	raw_ssid=buf;
    string ssid;
    string suffix;
    string mac_address;
    m_target_ssid_prefix = Configuration::getInstance()->getSsidPrefix();
    get_device_interface_mac(mac_address);
    suffix += mac_address;
    ssid = m_target_ssid_prefix + suffix;
	cdb_set("$wl_bss_ssid1",(char *)ssid.c_str());
	cdb_set_int("$op_work_mode",1);
	system_command("mtc commit");
	return true;
#else
    bool ret_value = true;

    APP_INFO("prepare softAp environment resource.");

    set_softAp_ssid_and_pwd(DUERLINK_WPA_HOSTAPD_CONFIG_FILE_PATH_MTK);

    //stop wpa supplicant.
    if (!stop_wpa_supplicant()) {
        APP_ERROR("stop supplicant failed.");
        ret_value = false;
    }

    //down and clear address.
//    if (!down_wlan0_interface()) {
//        APP_ERROR("down wlan0 failed.");
//        ret_value = false;
//    }

#if !defined(CONFIG_PANTHER)
    //echo ap to /dev/wifi file.
    if (!echo_ap_to_devwifi()) {
        APP_ERROR("echo ap to wifi failed.");
        ret_value = false;
    }
#endif

    // start ap0 interface.
    if (!starup_ap_interface()) {
        APP_ERROR("starup ap interface failed.");
        ret_value = false;
    }

    //set ap ip address and network mask value.
    if (!set_ap_tmp_ipaddr()) {
        APP_ERROR("set ap tmp ipaddr failed.");
        ret_value = false;
    }

    //start hostapd service process.
    if (!start_ap_mode()) {
        APP_ERROR("start ap mode failed.");
        ret_value = false;
    }

    //startup dhcp server.
    if (!start_dhcp_server()) {
        APP_ERROR("start dhcp server failed.");
        ret_value = false;
    }

    APP_INFO("End prepare softAp environment resource.");

    return ret_value;
#endif
}

bool softap_destroy_env_cb() {trc(__func__);
#if 0
	cdb_set("$wl_bss_ssid1",(char *)raw_ssid.c_str());
#else
    bool ret_value = true;

    APP_INFO("destroy softAp environment resource.");

    if (!stop_dhcp_server()) {
        APP_ERROR("stop dhcp server failed.");
        ret_value = false;
    }

    //delete tmp addression.
    if (!delete_tmp_addr()) {
        APP_ERROR("delete tmp addr failed.");
        ret_value = false;
    }

    //stop ap mode.
    if (!stop_ap_mode()) {
        APP_ERROR("stop ap mode failed.");
        ret_value = false;
    }

    //shutdown ap0
    if (!down_ap_interface()) {
        APP_ERROR("down ap interface failed.");
        ret_value = false;
    }

    //up wlan0
//    if (!starup_wlan0_interface()) {
//        APP_ERROR("up wlan0 interface failed.");
//        ret_value = false;
//    }

    APP_INFO("End destroy softAp environment resource.");

    return ret_value;
#endif
}

bool platform_connect_wifi_cb(const char *ssid,
                              const int ssid_len,
                              const char *pwd,
                              const int pwd_len) {
    m_target_ssid.clear();
    m_target_pwd.clear();
    m_target_ssid = ssid;
    m_target_pwd = pwd;

    APP_INFO("Connect wifi, ssid and pwd [%s] [%s].",
             m_target_ssid.c_str(),
             m_target_pwd.c_str());

#if defined(CONFIG_PANTHER)
    return config_and_start_wpa_supplicant(m_target_ssid, m_target_pwd);
#else
    tmp_wpa_conf(m_target_ssid, m_target_pwd, DUERLINK_WPA_CONFIG_FILE_MTK);

    return start_wpa_supplicant();
#endif
}

bool ble_prepare_env_cb() {trc(__func__);
    APP_INFO("prepare ble environment resource.");

//    m_ble_is_opened = framework::DeviceIo::getInstance()->controlBt(framework::BtControl::BT_IS_OPENED);
//    if (!m_ble_is_opened) {
//        APP_ERROR("Open bluetooth.");
//        framework::DeviceIo::getInstance()->controlBt(framework::BtControl::BT_OPEN);
//    }

    framework::DeviceIo::getInstance()->controlBt(framework::BtControl::BLE_OPEN_SERVER);

    APP_INFO("End prepare ble environment resource.");

    return true;
}

bool ble_destroy_env_cb() {trc(__func__);
    APP_INFO("destroy ble environment resource.");

    framework::DeviceIo::getInstance()->controlBt(framework::BtControl::BLE_CLOSE_SERVER);

//    if (!m_ble_is_opened) {
//        APP_INFO("Close bluetooth.");
//        framework::DeviceIo::getInstance()->controlBt(framework::BtControl::BT_CLOSE);
//        APP_INFO("Close led.");
//////        DeviceIoWrapper::getInstance()->ledBtOff();
//    }

    APP_INFO("End destroy ble environment resource.");

    return true;
}

int ble_send_data_cb(char *data, unsigned short data_length) {trc(__func__);
	trc(__func__);
    framework::DeviceIo::getInstance()->controlBt(framework::BtControl::BLE_SERVER_SEND,
                                                  data,
                                                  data_length);
	
	showHexBuf((char *)data,data_length);
    APP_INFO("End ble send data.");
    return 0;
}

DuerLinkMtkInstance::DuerLinkMtkInstance() {trc(__func__);
    m_maxPacketSize = MAX_PACKETS_COUNT;
    m_datalen = 56;
    m_nsend = 0;
    m_nreceived = 0;
    m_icmp_seq = 0;
    m_pMtkConfigObserver = nullptr;
	m_pObserver = nullptr;
    m_operation_type = operation_type::EOperationStart;
    pthread_mutex_init(&m_ping_lock, nullptr);

    init_network_config_timeout_alarm();
}

DuerLinkMtkInstance::~DuerLinkMtkInstance() {trc(__func__);
    pthread_mutex_destroy(&m_ping_lock);

//    duerLinkSdk::get_instance()->destroy();
}

DuerLinkMtkInstance* DuerLinkMtkInstance::get_instance() {
    if (nullptr == m_duerLink_mtk_instance) {
        m_duerLink_mtk_instance = new DuerLinkMtkInstance();
    }

    return m_duerLink_mtk_instance;
}

void DuerLinkMtkInstance::destroy() {trc(__func__);
    if (nullptr != m_duerLink_mtk_instance) {
        delete m_duerLink_mtk_instance;
        m_duerLink_mtk_instance = nullptr;
    }
}

void DuerLinkMtkInstance::init_duer_link_log(duerLink::duer_link_log_cb_t duer_link_log) {trc(__func__);
//    duerLinkSdk::get_instance()->init_duer_link_log(duer_link_log);
}

bool DuerLinkMtkInstance::set_wpa_conf(bool change_file) {trc(__func__);
    int skip_line = 0;
    string tmp_conf_path;
    string bk_conf_path;
    string file_conf_path;
    string cmd_buff;

#if defined(CONFIG_PANTHER)
    return true;
#endif
    if (m_target_ssid.empty() || m_target_pwd.empty()){
        APP_ERROR("Input param is empty.");
        return false;
    }

    tmp_conf_path = DUERLINK_WPA_CONFIG_FILE_MTK;
    tmp_conf_path += ".tmp";
    bk_conf_path = DUERLINK_WPA_CONFIG_FILE_MTK;
    bk_conf_path += ".bk";
    file_conf_path = DUERLINK_WPA_CONFIG_FILE_MTK;

    if (!change_file) {
        cmd_buff.clear();
        cmd_buff = "mv ";
        cmd_buff += bk_conf_path + " " + file_conf_path;
        cmd_buff += " &";

        if (!system_command(cmd_buff.c_str())) {
            APP_ERROR("Reset wpa_supplicant.conf failed.");
            return false;
        }
        sync();

        APP_INFO("Reset wpa_supplicant.conf successed.");

        return false;
    }

    string insert_ct = NETWORK_WPA_CONF_HEAD;
    insert_ct += m_target_ssid;
    insert_ct += NETWORK_WPA_CONF_MIDDLE;
    insert_ct += m_target_pwd;
    insert_ct += NETWORK_WPA_CONF_END;

    string line;
    std::ifstream in(bk_conf_path);
    std::ofstream out(tmp_conf_path);

    //read file head
    while (getline (in, line)) {
        if (strstr(line.c_str(), DUERLINK_WPA_INSERT_FLAG)){
            out << line << std::endl;
            out << insert_ct << std::endl;
            break;
        } else {
            out << line << std::endl;
        }
    }

    //read file body
    vector<string> body;
    string tmp_ssid = "\"" + m_target_ssid + "\"";
    while (getline (in, line)) {
        if (strstr(line.c_str(), tmp_ssid.c_str())) {
            APP_ERROR("Set ssid is already exists.");
            body.pop_back();
            skip_line = 3;
            continue;
        }

        if (skip_line != 0) {
            skip_line --;
            continue;
        } else {
            body.push_back(line);
        }
    }

    for (unsigned int i = 0; i < body.size(); i++) {
        out << body[i] << std::endl;
    }

    in.close();
    out.close();

    cmd_buff.clear();
    cmd_buff = "mv ";
    cmd_buff += tmp_conf_path + " " + file_conf_path;
    cmd_buff += " &";

    if (!system_command(cmd_buff.c_str())) {
        APP_ERROR("Update Wpa_supplicant.conf failed.");
        return false;
    }
    sync();

    APP_INFO("Update Wpa_supplicant.conf successed.");

    return true;
}

bool DuerLinkMtkInstance::is_first_network_config(string path) {trc(__func__);
    ifstream it_stream;
    int length = 0;
    string wpa_config_file = path;

    it_stream.open(wpa_config_file.c_str());
    if (!it_stream.is_open()) {
#if defined(CONFIG_PANTHER)
        return true;
#else
        APP_ERROR("wpa config file open error.");
        return false;
#endif
    }

    it_stream.seekg(0,std::ios::end);
    length = it_stream.tellg();
    it_stream.seekg(0,std::ios::beg);

    char *buffer = new char[length];
    it_stream.read(buffer, length);
    it_stream.close();

    char * position = nullptr;
    position = strstr(buffer,"ssid");
    delete [] buffer;
    buffer = nullptr;

    if (nullptr == position) {
        APP_ERROR("First network config.");
        return true;
    }

    APP_INFO("Not first network config.");

    return false;
}

void DuerLinkMtkInstance::sigalrm_fn(int sig) {trc(__func__);
    APP_INFO("alarm is run.");

    get_instance()->m_operation_type = operation_type::EAutoEnd;

    get_instance()->stop_network_config_timeout_alarm();

	if (nullptr != m_pMtkConfigObserver) {
		(m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkConfigExited);
	}

}

void DuerLinkMtkInstance::init_network_config_timeout_alarm() {trc(__func__);
    APP_INFO("set alarm.");

    signal(SIGALRM, sigalrm_fn);
}

void DuerLinkMtkInstance::start_network_config_timeout_alarm(int timeout) {trc(__func__);
    APP_INFO("start alarm.");

    alarm(timeout);
}

void DuerLinkMtkInstance::stop_network_config_timeout_alarm() {trc(__func__);
    APP_INFO("stop alarm.");

    alarm(0);
}

void DuerLinkMtkInstance::init_softAp_env_cb() {trc(__func__);
//    duerLinkSdk::get_instance()->init_platform_softAp_control(softap_prepare_env_cb,
//                                                              softap_destroy_env_cb);
}

void DuerLinkMtkInstance::init_ble_env_cb() {trc(__func__);
//    duerLinkSdk::get_instance()->init_platform_ble_control(ble_prepare_env_cb,
//                                                           ble_destroy_env_cb,
//                                                           ble_send_data_cb);
}

void DuerLinkMtkInstance::init_wifi_connect_cb() {trc(__func__);
//    duerLinkSdk::get_instance()->init_platform_connect_control(platform_connect_wifi_cb);
}

void DuerLinkMtkInstance::start_network_recovery() {trc(__func__);
#ifdef Box86
    check_recovery_network_status();
#else 
	bool firstCfgnet = is_first_network_config(DUERLINK_WPA_CONFIG_FILE_MTK);
	bool isDeviceIdInvalid = !Configuration::getInstance()->isDeviceIdValid();
	war("firstCfgnet:%d",firstCfgnet?1:0);
	war("isDeviceIdInvalid:%d",isDeviceIdInvalid?1:0);
    if (firstCfgnet || isDeviceIdInvalid) {		
        get_instance()->m_operation_type = operation_type::EAutoConfig;

        start_network_config_timeout_alarm(DUERLINK_AUTO_CONFIG_TIMEOUT);

		system_command("killall andlink 2>/dev/null");
		system_command("andlink -n >/dev/null 2>&1 &");

	    if (nullptr != m_pMtkConfigObserver) {
	        (m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkConfigStarted);
	    }		
    } else {
        check_recovery_network_status();
    }
#endif
}

void DuerLinkMtkInstance::start_discover_and_bound() {inf(__func__);
//    duerLinkSdk::get_instance()->start_device_discover_and_bound();
}

#ifdef DUERLINK_V2    
void DuerLinkMtkInstance::init_config_network_parameter(platform_type speaker_type,
                                                        auto_config_network_type autoType,
                                                        string devicedID,
                                                        string interface,
														) {
    duerLinkSdk::get_instance()->init_config_network_parameter(speaker_type,
                                                               autoType,
                                                               devicedID,
                                                               interface);
}

void DuerLinkMtkInstance::init_discovery_parameter(string devicedID,
                                                   string appId,
                                                   string interface) {
    //test cuid and access token for linkplay
//    duerLinkSdk::get_instance()->set_config_json_file("/data/duer/duerLink_config.json");

    duerLinkSdk::get_instance()->init_discovery_parameter(devicedID, appId, interface, "/data/duer/bduss.txt");

}
#else
    
void DuerLinkMtkInstance::init_config_network_parameter(platform_type speaker_type,
                                                        auto_config_network_type autoType,
                                                        const std::string& devicedID,
                                                        const std::string& interface,
                                                        const std::string& bdussfile,
                                                        const std::string& clientId) {
//	duerLinkSdk::get_instance()->registerRfreshAccessToken(getAccessTokenCallback);
//    duerLinkSdk::get_instance()->init_config_network_parameter(speaker_type,
//                                                               autoType,
//                                                               devicedID,
//                                                               interface,
//                                                               bdussfile,
//                                                               clientId);
}

void DuerLinkMtkInstance::init_discovery_parameter(const std::string& devicedID,
                                                   const std::string& appId,
                                                   const std::string& interface,
                                                   const std::string& bdussfile) {
    //test cuid and access token for linkplay
//    duerLinkSdk::get_instance()->set_config_json_file("/data/duer/duerLink_config.json");

//    duerLinkSdk::get_instance()->init_discovery_parameter(devicedID, appId, interface, bdussfile);

}
#endif

void DuerLinkMtkInstance::set_networkConfig_observer(NetworkConfigStatusObserver* config_listener) {trc(__func__);
    if (config_listener) {
        m_pMtkConfigObserver = config_listener;
    }
//    duerLinkSdk::get_instance()->set_networkConfig_observer(config_listener);
}

void DuerLinkMtkInstance::set_monitor_observer(NetWorkPingStatusObserver *ping_listener) {trc(__func__);
    if (ping_listener) {
        m_pObserver = ping_listener;
    }

    APP_INFO("Set monitor observer.");
}

bool DuerLinkMtkInstance::check_recovery_network_status() {trc(__func__);
#ifdef MTK8516
    deviceCommonLib::deviceTools::printTickCount("network_config main_thread recovery begin.");
#endif

    if (nullptr != m_pMtkConfigObserver) {
        (m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkRecoveryStart);
    }

    int network_check_count = 0;
    bool recovery = false;

    while(!(recovery = ping_network(false))) {
        if (network_check_count == DUERLINK_NETWORK_CONFIGURE_PING_COUNT) {
            APP_ERROR("Network recovery ping failed.");

            break;
        }

        sleep(1);
        network_check_count++;
    }

    if (recovery) {
        if (nullptr != m_pMtkConfigObserver) {
            APP_INFO("Network recovery succed.");

            (m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkRecoverySucceed);
        }
        return true;
    } else {
        if (nullptr != m_pMtkConfigObserver) {
            APP_ERROR("Network recovery failed.");

            (m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkRecoveryFailed);
        }

        return false;
    }
}

void DuerLinkMtkInstance::ble_client_connected() {trc(__func__);
//    duerLinkSdk::get_instance()->ble_client_connected();
}

void DuerLinkMtkInstance::ble_client_disconnected() {trc(__func__);
//    duerLinkSdk::get_instance()->ble_client_disconnected();
}

bool DuerLinkMtkInstance::ble_recv_data(void *data, int len) {trc(__func__);
	trc(__func__);
	char binBuf[1024]={0};
	inf((char *)data);
	ssize_t binSize=base64Decode((char *)data,len,binBuf,sizeof(binBuf));
	if(binSize<0){
		return false;
	}
	showHexBuf(binBuf,binSize);
//    binSize= duerLinkSdk::get_instance()->ble_recv_data(binBuf,binSize);
	if(binSize<0){
		return false;
	}
	return true;	
}

bool DuerLinkMtkInstance::start_network_config(int timeout) {trc(__func__);
    APP_INFO("start configing %d.", timeout);

    m_operation_type = operation_type::EManualConfig;

    start_network_config_timeout_alarm(timeout);

	system_command("killall andlink 2>/dev/null");
	system_command("andlink -n >/dev/null 2>&1 &");


    if (nullptr != m_pMtkConfigObserver) {
        (m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkConfigStarted);
    }
	return true;	
//    return duerLinkSdk::get_instance()->start_network_config();
}

void DuerLinkMtkInstance::stop_network_config() {trc(__func__);
    APP_INFO("start stopping.");

    m_operation_type = operation_type::EAutoEnd;

    stop_network_config_timeout_alarm();

    if (nullptr != m_pMtkConfigObserver) {
        (m_pMtkConfigObserver)->notify_network_config_status(duerLink::ENetworkConfigExited);
    }
	
//    duerLinkSdk::get_instance()->stop_network_config();
}

void DuerLinkMtkInstance::set_dlp_data_observer(DuerLinkReceivedDataObserver* observer) {trc(__func__);
//    duerLinkSdk::get_instance()->set_dlp_data_observer(observer);
}

bool DuerLinkMtkInstance::send_dlp_msg_to_all_clients(const string& sendBuffer) {trc(__func__);
	return true;
//    return duerLinkSdk::get_instance()->send_dlp_msg_to_all_clients(sendBuffer);
}

bool DuerLinkMtkInstance::send_dlp_msg_by_specific_session_id(
    const string& message, unsigned short sessionId) {
	return true;
//    return duerLinkSdk::get_instance()->send_dlp_msg_to_client_by_specific_id(message, sessionId);
}

bool DuerLinkMtkInstance::start_loudspeaker_ctrl_devices_service(
    device_type device_type_value, const string& client_id, const std::string& message) {
	return true;
//    return duerLinkSdk::get_instance()->start_loudspeaker_ctrl_devices_service(device_type_value,
//                                                                               client_id,
//                                                                               message);
}

bool DuerLinkMtkInstance::start_loudspeaker_ctrl_devices_service_by_device_id(const string& device_id) {trc(__func__);
	return true;
//    return duerLinkSdk::get_instance()->start_loudspeaker_ctrl_devices_service_by_device_id(device_id);
}

bool DuerLinkMtkInstance::send_msg_to_devices_by_spec_type(const string& msg_buf,
                                                           device_type device_type_value) {
	return true;

//    return duerLinkSdk::get_instance()->send_msg_to_devices_by_spec_type(msg_buf,
//                                                                         device_type_value);
}

bool DuerLinkMtkInstance::disconnect_devices_connections_by_spe_type(
        device_type device_type_value, const string& message) {
    APP_DEBUG("device_type_value = [%d], message = [%s]", device_type_value, message.c_str());
	return true;
//    return duerLinkSdk::get_instance()->disconnect_devices_connections_by_spe_type(device_type_value,
//                                                                                   message);
}

bool DuerLinkMtkInstance::get_curret_connect_device_info(string& client_id,
                                                         string& device_id,
                                                         device_type device_type_value) {
	return true;
//    return duerLinkSdk::get_instance()->get_curret_connect_device_info(client_id,
//                                                                       device_id,
//                                                                       device_type_value);
}

void DuerLinkMtkInstance::set_network_observer(NetWorkPingStatusObserver *observer) {trc(__func__);
    if (observer != nullptr){
        m_pObserver = observer;
    }
}
    
unsigned short DuerLinkMtkInstance::getChksum(unsigned short *addr,int len) {
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft-= 2;
    }

    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = ((sum>>16) + (sum&0xffff));
    sum += (sum>>16);
    answer = ~sum;

    return answer;
}

int DuerLinkMtkInstance::packIcmp(int pack_no, struct icmp* icmp) {
    int packsize;
    struct icmp *picmp;
    struct timeval *tval;

    picmp = icmp;
    picmp->icmp_type = ICMP_ECHO;
    picmp->icmp_code = 0;
    picmp->icmp_cksum = 0;
    picmp->icmp_seq = pack_no;
    picmp->icmp_id = m_pid;
    packsize = (8 + m_datalen);
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval, nullptr);
    picmp->icmp_cksum = getChksum((unsigned short *)icmp, packsize);

    return packsize;
}

bool DuerLinkMtkInstance::unpackIcmp(char *buf, int len, struct IcmpEchoReply *icmpEchoReply) {
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    struct timeval *tvsend, tvrecv, tvresult;
    double rtt;

    ip = (struct ip *)buf;
    iphdrlen = ip->ip_hl << 2;
    icmp = (struct icmp *)(buf + iphdrlen);
    len -= iphdrlen;

    if (len < 8) {
        APP_ERROR("ICMP packets's length is less than 8.");
        return false;
    }

    if( (icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == m_pid) ) {
        tvsend = (struct timeval *)icmp->icmp_data;
        gettimeofday(&tvrecv, nullptr);
        tvresult = timevalSub(tvrecv, *tvsend);
        rtt = tvresult.tv_sec*1000 + tvresult.tv_usec/1000;  //ms
        icmpEchoReply->rtt = rtt;
        icmpEchoReply->icmpSeq = icmp->icmp_seq;
        icmpEchoReply->ipTtl = ip->ip_ttl;
        icmpEchoReply->icmpLen = len;

        return true;
    } else {
        return false;
    }
}

struct timeval DuerLinkMtkInstance::timevalSub(struct timeval timeval1, struct timeval timeval2) {
    struct timeval result;

    result = timeval1;

    if ((result.tv_usec < timeval2.tv_usec) && (timeval2.tv_usec < 0)) {
        -- result.tv_sec;
        result.tv_usec += 1000000;
    }

    result.tv_sec -= timeval2.tv_sec;

    return result;
}

bool DuerLinkMtkInstance::sendPacket() {
    size_t packetsize;
    while( m_nsend < m_maxPacketSize) {
        m_nsend ++;
        m_icmp_seq ++;
        packetsize = packIcmp(m_icmp_seq, (struct icmp*)m_sendpacket);

        if (sendto(m_sockfd,m_sendpacket, packetsize, 0, (struct sockaddr *) &m_dest_addr, sizeof(m_dest_addr)) < 0) {
            APP_ERROR("Ping sendto failed:%s.", strerror(errno));
            continue;
        }
    }

    return true;
}

bool DuerLinkMtkInstance::recvPacket(PingResult &pingResult) {
    int len = 0;
    struct IcmpEchoReply icmpEchoReply;
    int maxfds = m_sockfd + 1;
    int nfd  = 0;
    fd_set rset;
    struct timeval timeout;
    socklen_t fromlen = sizeof(m_from_addr);

    timeout.tv_sec = MAX_WAIT_TIME;
    timeout.tv_usec = 0;

    FD_ZERO(&rset);

    for (int recvCount = 0; recvCount < m_maxPacketSize; recvCount ++) {
        FD_SET(m_sockfd, &rset);
        if ((nfd = select(maxfds, &rset, nullptr, nullptr, &timeout)) == -1) {
            APP_ERROR("Ping recv select failed:%s.", strerror(errno));
            continue;
        }

        if (nfd == 0) {
            icmpEchoReply.isReply = false;
            pingResult.icmpEchoReplys.push_back(icmpEchoReply);
            continue;
        }

        if (FD_ISSET(m_sockfd, &rset)) {
            if ((len = recvfrom(m_sockfd,
                                m_recvpacket,
                                sizeof(m_recvpacket),
                                0,
                                (struct sockaddr *)&m_from_addr,
                                &fromlen)) <0) {
                if(errno == EINTR) {
                    continue;
                }
                APP_ERROR("Ping recvfrom failed: %s.", strerror(errno));
                continue;
            }

            icmpEchoReply.fromAddr = inet_ntoa(m_from_addr.sin_addr) ;
            if (strncmp(icmpEchoReply.fromAddr.c_str(), pingResult.ip, strlen(pingResult.ip)) != 0) {
                recvCount--;
                continue;
            }
        }

        if (!unpackIcmp(m_recvpacket, len, &icmpEchoReply)) {
            recvCount--;
            continue;
        }

        icmpEchoReply.isReply = true;
        pingResult.icmpEchoReplys.push_back(icmpEchoReply);
        m_nreceived ++;
    }

    return true;

}

bool DuerLinkMtkInstance::getsockaddr(const char * hostOrIp, struct sockaddr_in* sockaddr) {
    struct hostent *host;
    struct sockaddr_in dest_addr;
    unsigned long inaddr = 0l;

    bzero(&dest_addr,sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;

    inaddr = inet_addr(hostOrIp);
    if (inaddr == INADDR_NONE) {
        host = gethostbyname(hostOrIp);
        if (host == nullptr) {
            return false;
        }
        memcpy( (char *)&dest_addr.sin_addr,host->h_addr, host->h_length);
    } else if (!inet_aton(hostOrIp, &dest_addr.sin_addr)) {
        return false;
    }

    *sockaddr = dest_addr;

    return true;
}

bool DuerLinkMtkInstance::ping(string host, int count, PingResult& pingResult) {
    int size = 50 * 1024;
    IcmpEchoReply icmpEchoReply;

    m_nsend = 0;
    m_nreceived = 0;
    pingResult.icmpEchoReplys.clear();
    m_maxPacketSize = count;
    m_pid = getpid();

    pingResult.dataLen = m_datalen;

    if ((m_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        APP_ERROR("Ping socket failed:%s.", strerror(errno));
        pingResult.error = strerror(errno);
        return false;
    }

    if (setsockopt(m_sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0) {
        APP_ERROR("Setsockopt SO_RCVBUF failed:%s.", strerror(errno));
        close(m_sockfd);
        return false;
    }

    if (!getsockaddr(host.c_str(), &m_dest_addr)) {
        pingResult.error = "unknow host " + host;
        close(m_sockfd);
        return false;
    }

    strcpy(pingResult.ip, inet_ntoa(m_dest_addr.sin_addr));

    sendPacket();
    recvPacket(pingResult);

    pingResult.nsend = m_nsend;
    pingResult.nreceived = m_nreceived;

    close(m_sockfd);

    return true;
}

bool DuerLinkMtkInstance::ping_network(bool wakeupTrigger) {trc(__func__);
    string hostOrIp = PING_DEST_HOST1;
    int nsend = 0, nreceived = 0;
    bool ret;
    PingResult pingResult;
    InternetConnectivity networkResult = UNAVAILABLE;

    pthread_mutex_lock(&m_ping_lock);

    for (int count = 1; count <= MAX_PACKETS_COUNT; count ++) {
        memset(&pingResult.ip, 0x0, 32);
        ret = ping(hostOrIp, 1, pingResult);

        if (!ret) {
            APP_ERROR("Ping error:%s", pingResult.error.c_str());
        } else {
            nsend += pingResult.nsend;
            nreceived += pingResult.nreceived;
            if (nreceived > 0)
                break;
        }

        if (count == 2) {
            hostOrIp = PING_DEST_HOST2;
        }
    }

    if (nreceived > 0) {
        ret = true;
        networkResult = AVAILABLE;
        if (m_network_status == (int)UNAVAILABLE) {
            m_ping_interval = 1;
        } else {
            if (m_ping_interval < MAX_PING_INTERVAL) {
                m_ping_interval = m_ping_interval * 2;
                if (m_ping_interval > MAX_PING_INTERVAL) {
                    m_ping_interval = MAX_PING_INTERVAL;
                }
            }
        }
        m_network_status = 1;
    } else {
        ret = false;
        networkResult = UNAVAILABLE;
        m_network_status = 0;
        m_ping_interval = 1;
    }
	inf("networkResult:%d,wakeupTrigger:%d",networkResult, wakeupTrigger?1:0);
    if (m_pObserver) {
        m_pObserver->network_status_changed(networkResult, wakeupTrigger);
    }

    pthread_mutex_unlock(&m_ping_lock);

    return ret;
}

void *DuerLinkMtkInstance::monitor_work_routine(void *arg) {trc(__func__);
    auto thread = static_cast<DuerLinkMtkInstance*>(arg);
    int time_interval = 1;
    int time_count = 1;
    while(1) {
        thread->ping_network(false);
        time_count = time_interval = m_ping_interval;
        APP_INFO("monitor_work_routine m_ping_interval:%d", m_ping_interval);
        while (time_count > 0) {
            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            ::select(0, nullptr, nullptr, nullptr, &tv);
            time_count--;
            if (time_interval != m_ping_interval) {
                APP_INFO("monitor_work_routine m_ping_interval:%d, time_interval:%d", m_ping_interval, time_interval);
				inf("monitor_work_routine m_ping_interval:%d, time_interval:%d", m_ping_interval, time_interval);
                break;
            }
        }
    }

    return nullptr;
}

void DuerLinkMtkInstance::start_network_monitor() {trc(__func__);
    pthread_t network_config_threadId;

    pthread_create(&network_config_threadId, nullptr, monitor_work_routine, this);
    deviceCommonLib::deviceTools::setThreadName(network_config_threadId, APP_THREAD_NAME_DUERLINK_STARTNETWORKMONITOR);
    pthread_detach(network_config_threadId);
}

void *DuerLinkMtkInstance::verify_work_routine(void *arg) {trc(__func__);
    auto thread = static_cast<DuerLinkMtkInstance*>(arg);
    APP_INFO("verify_work_routine begin");
    m_pinging = true;
    thread->ping_network(true);
    m_pinging = false;
    APP_INFO("verify_work_routine end");

    return nullptr;
}

void DuerLinkMtkInstance::start_verify_network() {trc(__func__);
    if (!m_pinging) {
        pthread_t network_verify_threadId;

        pthread_create(&network_verify_threadId, nullptr, verify_work_routine, this);
        deviceCommonLib::deviceTools::setThreadName(network_verify_threadId, APP_THREAD_NAME_DUERLINK_STARTVERIFYNETWORK);
        pthread_detach(network_verify_threadId);
    } else {
        APP_INFO("start_verify_network m_pinging is TRUE, return");
    }
}

}  // namespace application
}  // namespace duerOSDcsApp
