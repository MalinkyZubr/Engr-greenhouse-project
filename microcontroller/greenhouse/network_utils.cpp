#include "network_utils.hpp"

/// @brief get the channel of a specified wireless AP in preparation to connect
/// @param ssid the SSID of the AP in question
/// @return integer representation of the AP channel. if -1, the SSID is not associated with any active AP
int get_ap_channel(String &ssid) {
  int num_networks = WiFi.scanNetworks();

  for(int network = 0; network < num_networks; network++) {
    if(ssid.equals(WiFi.SSID(network))) {
      return WiFi.channel(network);
    }
  }
  return -1;
}


bool check_wifi_module_status() {
  if (WiFi.status() == WL_NO_SHIELD) {
    return false;
  }
  return true;
}