#ifndef API_HPP
#define API_HPP


#include <Arduino.h>
#include <ArduinoJson.h>


class Serializer {
    public:
    DynamicJsonDocument registration_confirmation(char[] device_ip, 
            char[] device_mac, 
            long device_id=0, 
            char[] device_name="",
            char[] preset_name="",
            char[] project_name="");
    
}

#endif