#ifndef STORAGE_HPP
#define STORAGe_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>


class ConfigManager {
    private:
    uint32_t configAddr = 200;
    SPIFlash flash;

    public:
    typedef struct {
        float Temperature;
        float Humidity;
        float Moisture;
        float LightExposure;
        float IRExposure;
        String PresetName;
    } Preset;

    typedef struct {
      int device_id;
      String server_hostname;
      String device_name;
      String project_name;
      Preset preset;
    }Configuration;

    Configuration config;

    ConfigManager();
    void de_power();
    bool power(String outputString);
    bool retrieve_configuration(String outputString);
    bool write_configuration(DynamicJsonDocument toSerialize);
};

#endif