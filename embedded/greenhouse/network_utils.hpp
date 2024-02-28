#ifndef NETWORK_UTILS_HPP
#define NETWORK_UTILS_HPP

#include <Arduino.h>
#include <WiFi101.h>


int get_ap_channel(String &ssid);

bool check_wifi_module_status();


#endif