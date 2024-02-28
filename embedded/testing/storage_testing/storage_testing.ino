#include "storage.hpp"
#include <ArduinoJson.h>

ConfigManager configger(1024);

void setup() {
  Serial.begin(9600);
  DynamicJsonDocument document(CONFIG_JSON_SIZE);
  document["silly"] = "hehe";
  document["dervicsh"] = "eheh";
  document["legume"] = "bean";
  document["ruh"] = "roh";
  document["dingus"] = "bingus";
  document["longo"] = "dongo";
  document["foo"] = "bar";
  document["yllis"] = "bazingo";

  configger.write_configuration(&document);

  String output;

  DynamicJsonDocument read(CONFIG_JSON_SIZE);

  configger.retrieve_configuration(&output);

  deserializeJson(read, output);

  Serial.print("Retrieved: ");
  Serial.println(output);
  String test = read["silly"];
  Serial.print("Test: ");
  Serial.println(test);

  String reserialize;
  serializeJson(read, reserialize);
  Serial.println(reserialize);
}

void loop() {

}