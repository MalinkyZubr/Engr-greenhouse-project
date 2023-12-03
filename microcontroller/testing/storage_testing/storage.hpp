#ifndef STORAGE_HPP
#define STORAGe_HPP

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <Arduino.h>
#include <SPIMemory.h>

#define CONFIG_ADDRESS 5000
#define CONFIG_JSON_SIZE 256


class ConfigManager {
    private:
    int configAddr = CONFIG_ADDRESS;
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
    } Configuration;

    Configuration config;

    ConfigManager(int configAddr);
    void de_power();
    bool power(String *output);
    bool retrieve_configuration(String *output);
    bool write_configuration(DynamicJsonDocument *document);
};

#endif