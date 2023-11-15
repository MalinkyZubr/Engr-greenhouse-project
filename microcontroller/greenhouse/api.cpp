#include "api.hpp"


DynamicJsonDocument registration_confirmation(
    char[] device_ip, 
    char[] device_mac, 
    long device_id=0, 
    char[] device_name="",
    char[] preset_name="",
    char[] project_name="") {
      DynamicJsonDocument doc(128);
      doc["decice_name"] = device_name;
      doc["device_id"] = device_id;
      doc["device_ip"] = device_ip;
      doc["device_mac"] = device_mac;
      doc["preset_name"] = preset_name;
      doc["project_name"] = project_name;

      return doc;
}