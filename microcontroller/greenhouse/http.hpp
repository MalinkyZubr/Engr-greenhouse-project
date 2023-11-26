#ifndef HTTP_HPP
#define HTTP_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <Regexp.h>
#include "storage.hpp"


Reg GET_JSON = Reg(R"(\{[^}]*\})");
Reg GET_CONTENT_LENGTH = Reg(R"((?<=Content-Length: )\d+)");
Reg GET_HOST = Reg(R"((?<=Host: )\S+)");
Reg GET_ROUTE = Reg(R"(/([^ ]+)(?=\sHTTP))");
Reg GET_STATUS = Reg(R"(\d\d\d)");
Reg GET_METHOD = Reg(R"(^([A-Z]+)\b)");


union methods {
    GET = "GET";
    POST = "POST";
    PUT = "PUT";
    DELETE = "DELETE";
    PATCH = "PATCH";
};


typedef struct {
    methods method;
    String route;
    String host;
    DynamicJsonDocument body(CONFIG_JSON_SIZE);
} parsed_request;


typedef struct {
    int status;
    DynamicJsonDocument body(CONFIG_JSON_SIZE);
} parsed_response;


class Reg {
    public:
    MatchState pattern;
    Reg(char *pattern);
    String match(String content);
};


class Requests {
    public:
    static String request(methods method, String route, String host);
    static String request(methods method, String route, String host, DynamicJsonDocument body);

    static parsed_request parse_request(String request);
};


class Responses {
    public:
    static String response(int status);
    static String response(int status, DynamicJsonDocument content);
    static String html_response(int status);

    static parsed_response parse_response(String response);
};

#endif