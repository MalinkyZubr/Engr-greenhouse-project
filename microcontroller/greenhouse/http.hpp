#ifndef HTTP_HPP
#define HTTP_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <Regexp.h>
#include "storage.hpp"


enum methods {
  GET,
  POST,
  PUT,
  DELETE,
  PATCH
};


String return_method(methods method);

typedef struct {
    methods method;
    String route;
    String host;
    DynamicJsonDocument body;
} parsed_request;


typedef struct {
    int status;
    DynamicJsonDocument body;
} parsed_response;


class Reg {
    public:
    MatchState pattern;
    Reg(char *pattern);
    String match(String content);
};


union ParsedMessage {
    parsed_request request;
    parsed_response response;
};


enum ReceiveType {
    REQUEST,
    RESPONSE
};


typedef struct {
  Reg GET_JSON = Reg(R"(\{[^}]*\})");
  Reg GET_CONTENT_LENGTH = Reg(R"((?<=Content-Length: )\d+)");
  Reg GET_HOST = Reg(R"((?<=Host: )\S+)");
  Reg GET_ROUTE = Reg(R"(/([^ ]+)(?=\sHTTP))");
  Reg GET_STATUS = Reg(R"(\d\d\d)");
  Reg GET_METHOD = Reg(R"(^([A-Z]+)\b)");
} RegularExpressions;


class Requests {
  private:
  RegularExpressions regex;

  public:
  static String request(methods method, String route, String host);
  static String request(methods method, String route, String host, DynamicJsonDocument body);

  static parsed_request parse_request(String request);
};


class Responses {
  private:
  RegularExpressions regex;
  
  public:
  static String response(int status);
  static String response(int status, DynamicJsonDocument content);
  static String html_response(int status, String content);

  static parsed_response parse_response(String response);
};


#endif