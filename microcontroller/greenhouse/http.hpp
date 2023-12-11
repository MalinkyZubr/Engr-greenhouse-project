#ifndef HTTP_HPP
#define HTTP_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <Regexp.h>
#include "storage.hpp"
#include "exceptions.hpp"


enum Method {
  GET,
  POST,
  PUT,
  DELETE,
  PATCH
};

enum BodyType {
  NONE,
  JSON,
  HTML,
  CSS,
  JS
};

enum DeviceDirective {
  DEVICE_NONE,
  DEVICE_HARD_RESET,
  DEVICE_NETWORK_RESET
};

String return_method_str(Method method);
Method return_method_enum(String &method);

class RegularExpressions {
  public:
  static const char* json_regex = R"(\{[^}]*\})";
  static const char* content_length_regex = R"((?<=Content-Length: )\d+)";
  static const char* host_regex = R"((?<=Host: )\S+)";
  static const char* route_regex = R"(/([^ ]+)(?=\sHTTP))";
  static const char* status_regex = R"(\d\d\d)";
  static const char* method_regex = R"(^([A-Z]+)\b)";

  static String match(const String &content, const char* pattern_str);
  static String get_body(const String &content);
  static String get_content_length(const String &content);
  static String get_host(const String &content);
  static String get_route(const String &content);
  static int get_status(const String &content);
  static Method get_method(const String &content);
}


class HTTPMessage {
  private:
  BodyType body_type;
  DynamicJsonDocument body = DynamicJsonDocument(0);
  NetworkExceptions exception = NETWORK_OKAY;

  public:
  HTTPMessage();
  HTTPMessage(DynamicJsonDocument body);

  BodyType get_body_type();
  DynamicJsonDocument& get_body();

  void set_exception(NetworkExceptions exception);
  NetworkExceptions get_exception();

  void set_body(DynamicJsonDocument body);
  void set_body_type(BodyType type);

  virtual String serialize() = 0;
};

class Request : public HTTPMessage {
  private:
  Method method;
  String route;
  String host;
  int device_id;
  MachineOperationalState machine_state;

  public:
  Request(Method method, String route, String host, int device_id, MachineOperationalState machine_state);
  Request(Method method, String route, String host, int device_id, MachineOperationalState machine_state, DynamicJsonDocument body);
  Request(String &unparsed);
  Request(Request* base_pointer);

  String get_route();
  Method get_method();

  String serialize();
};

class Response : public HTTPMessage {
  private:
  int status;
  String *file_content;
  DeviceDirective directive = DEVICE_NONE;

  public:
  Response(int status);
  Response(int status, DynamicJsonDocument body);
  Response(int status, BodyType file_type, String *file_content);
  Response(String &unparsed);
  Response(Response* base_pointer);

  void set_directive(DeviceDirective directive);
  DeviceDirective get_directive();

  String serialize();
};

enum ReceiveType {
  REQUEST,
  RESPONSE,
};

class ParsedMessage {
  public:
  NetworkExceptions error = OKAY;
  ReceiveType type;
  Response response;
  Request request;
};


#endif