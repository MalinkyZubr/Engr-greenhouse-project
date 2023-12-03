#ifndef HTTP_HPP
#define HTTP_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <Regexp.h>
#include "../storage/storage.hpp"
#include "wifi_info.hpp"


enum methods {
  GET,
  POST,
  PUT,
  DELETE,
  PATCH
};


String return_method(methods method);


class ParsedRequest {
  public:
  String method;
  String route;
  String host;
  DynamicJsonDocument body;
  ParsedRequest() : body(DynamicJsonDocument(0)) {};
  ParsedRequest(DynamicJsonDocument body) : body(body) {};
};

class ParsedResponse {
  public:
  int status;
  DynamicJsonDocument body;
  ParsedResponse() : body(DynamicJsonDocument(0)) {};
  ParsedResponse(DynamicJsonDocument body) : body(body) {};
};

enum ReceiveType {
  REQUEST,
  RESPONSE,
};

class ParsedMessage {
  public:
  NetworkReturnErrors error = OKAY;
  ReceiveType type;
  ParsedResponse response;
  ParsedRequest request;
};

class Reg {
    public:
    MatchState pattern;
    Reg(char *pattern);
    String match(String content);
};

typedef struct {
  Reg GET_JSON = Reg(R"(\{[^}]*\})");
  Reg GET_CONTENT_LENGTH = Reg(R"((?<=Content-Length: )\d+)");
  Reg GET_HOST = Reg(R"((?<=Host: )\S+)");
  Reg GET_ROUTE = Reg(R"(/([^ ]+)(?=\sHTTP))");
  Reg GET_STAT = Reg(R"(\d\d\d)");
  Reg GET_METHOD = Reg(R"(^([A-Z]+)\b)");
} RegularExpressions;

enum FileType {
  HTML,
  CSS,
  JS
};

class Requests {
  public:
  static RegularExpressions regex;
  static String request(methods method, String route, String host, int device_id);
  static String request(methods method, String route, String host, int device_id, DynamicJsonDocument body);

  static ParsedRequest parse_request(String request);
};


class Responses {
  public:
  static RegularExpressions regex;
  static String response(int status);
  static String response(int status, DynamicJsonDocument content);
  static String file_response(String content, FileType type);

  static ParsedResponse parse_response(String response);
};


#endif