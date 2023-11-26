#include "http.hpp"


Reg::Reg(char *pattern) {
  this->pattern.Match(pattern);
}

String Reg::match(String content) {
  int size = content.length() * sizeof(char);
  char* converted = malloc(size);
  content.toCharArray(converted, size);

  this->pattern.Target(converted);

  int start = this->pattern.MatchStart;
  int length = this->pattern.MatchLength;

  String result = "";
  for(int x = start; x < start + length; x++) {
    result.concat(converted[x]);
  }

  return result;
}


String Requests::request(methods method, String route, String host) {
  String request = method;
  request.concat(" " + route);
  request.concat(" HTTP/1.1\n");
  reqeust.concat("Host: " + host);

  return request;
}

String Requests::request(methods method, String route, String host, DynamicJsonDocument body) {
  String json;
  serializeJson(body, json);
  String request = Requests::request(method, route, host) + "\n";
  request.concat("Content-Type: application/json\n");
  request.concat("Content-Length: " += json.length() + "\n\n");
  request.concat(json);

  return request;
}

parsed_request Requests::parse_request(String request) {
  parsed_request request_struct;
  request_struct.method = GET_METHOD.match(request);
  request_struct.route = GET_ROUTE.match(request);
  request_struct.host = GET_HOST.match(request);
  String json = GET_JSON.match(request);
  if(json.length()) {
    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    deserializeJson(doc, json);
    request_struct.body = doc;
  }
  return request_struct;
}


String Responses::response(int status) {
  String response = "HTTP/1.1 ";
  response.concat(status);
  response.concat(" OK\n");
 
  return response;
}

String Responses::response(int status, DynamicJsonDocument content) {
  String response = Responses::response(status);
  String json;
  serializeJson(content, json);

  response.concat("Content-Type: application/json\n");
  response.concat("Content-Length: " + json.length() + "\n\n");
  response.concat(json);

  return response;
}

String Responses::html_response(int status, String content) {
  String response = Responses::response(status);

  response.concat("Content-Type: text/html\n");
  response.concat("Content-Length: " + content.length() + "\n\n");
  response.concat(content);

  return response;
}

parsed_response Responses::parse_response(String response) {
  parsed_response response_struct;

  response_struct.status = GET_STATUS.match(response);

  String json = GET_JSON.match(response);
  if(json.length()) {
    DynamicJsonDocument doc(JSON_CONFIG_SIZE);
    deserializeJson(doc, json);
    response_struct.body = json;
  }

  return response;
}