#include "http.hpp"

// need to fix http formatting, ensure it is all correct

/// @brief return a string which corresponds to the http method enum value
/// @param method http method enum to convert from
/// @return String representing the stringified HTTP method
String return_method(methods method) {
  String method_str;
  switch(method) {
    case GET:
      method_str = "GET";
      break;
    case POST:
      method_str = "POST";
      break;
    case PUT:
      method_str = "PUT";
      break;
    case DELETE:
      method_str = "DELETE";
      break;
    case PATCH:
      method_str = "PATCH";
      break;
  }
  return method_str;
}

/// @brief instantiate regex class based on char array pattern
/// @param pattern the char array pattern
Reg::Reg(char *pattern) {
  this->pattern.Match(pattern);
}

/// @brief regex function to return a matching substring of a larger string
/// @param content string representing the entire string
/// @return String, the matching substring
String Reg::match(String content) {
  int size = content.length() * sizeof(char);

  void* temp = malloc(size);
  char* converted = static_cast<char*>(temp);

  content.toCharArray(converted, size);

  this->pattern.Target(converted);

  int start = this->pattern.MatchStart;
  int length = this->pattern.MatchLength;

  String result = "";
  for(int x = start; x < start + length; x++) {
    result.concat(converted[x]);
  }

  free(temp);
  free(converted);
  return result;
}


/// @brief generate an HTTP request without a request body, good for simple http operations with the server
/// @param method The method to use in the request, like POST or GET
/// @param route the server API route to send the request to, must start with a "/"
/// @param host the hostname/ip address of the desired server
/// @param device_id the id of the device this function runs on, to be stored in a cookie for tracking requests server side
/// @param op_state the operational state of the device, IDLE or ACTIVE, stored in a cookie to track device status on the server
/// @param contains_body flag to say if the request should include a request body. If you have no json or webpage to serve, this should always be false
/// @return String, the formatted http request containing all specified information
String Requests::request(methods method, String route, String host, int device_id, MachineOperationalState op_state, bool contains_body = false) {
  String request = return_method(method);
  request.concat(" ");
  request.concat(route);
  request.concat(" HTTP/1.1\r\n");
  request.concat("Host: ");
  request.concat(host);
  request.concat("\r\n");
  request.concat("Cookie: device_id=");
  request.concat(device_id);
  request.concat("; status=");
  switch(op_state) {
    case MACHINE_ACTIVE:
      request.concat("ACTIVE");
      break;
    case MACHINE_PAUSED:
      request.concat("IDLE");
      break;
  }
  request.concat("\r\n");

  if(!contains_body) {
    request.concat("\r\n");
  }

  return request;
}


/// @brief generate a request with a json body
/// @param method The method to use in the request, POST or GET
/// @param route the hostname/ip address of the desired server
/// @param host the hostname/ip address of the desired server
/// @param device_id the id of the device this function runs on, to be stored in a cookie for tracking requests server side
/// @param op_state the operational state of the device, IDLE or ACTIVE, stored in a cookie to track device status on the server
/// @param body JSON object representing the request body
/// @return formatted request containing all necessary information, including the request body
String Requests::request(methods method, String route, String host, int device_id, MachineOperationalState op_state, DynamicJsonDocument body) {
  String json;
  serializeJson(body, json);
  String request = Requests::request(method, route, host, device_id, op_state, true);
  request.concat("Content-Type: application/json\r\n");
  request.concat("Content-Length: ");
  request.concat(json.length());
  request.concat("\r\n\r\n");
  request.concat(json);

  return request;
}


/// @brief use regex to parse an incoming request, from string to ParsedRequest object
/// @param request the string representing a raw http request received from a server
/// @return ParsedRequest, request object for use in code
ParsedRequest Requests::parse_request(String request) {
  ParsedRequest request_struct(DynamicJsonDocument(0));

  request_struct.method = Requests::regex.GET_METHOD.match(request);
  request_struct.route = Requests::regex.GET_ROUTE.match(request);
  request_struct.host = Requests::regex.GET_HOST.match(request);
  String json = Requests::regex.GET_JSON.match(request);

  if(json.length()) {
    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    deserializeJson(doc, json);
    request_struct.body = doc;
  }

  return request_struct;
}


/// @brief generate a string representation of an http response with no body
/// @param status int representing http status, 200 for instance
/// @param contains_body this should be false if no json is included in the request. Use the overload for that
/// @return String, string representation of the response
String Responses::response(int status, bool contains_body = false) {
  String response = "HTTP/1.1 ";
  response.concat(status);
  response.concat(" OK\r\n");
  
  if(!contains_body) {
    response.concat("\r\n\r\n");
  }

  return response;
}

/// @brief generate a string representation of an http response with no body
/// @param status int representing http status, 200 for instance
/// @param content JSON content representing response body
/// @return String, string representation of the response
String Responses::response(int status, DynamicJsonDocument content) {
  String response = Responses::response(status, true);
  String json;
  serializeJson(content, json);

  response.concat("Content-Type: application/json\r\n");
  response.concat("Content-Length: ");
  response.concat(json.length()); 
  response.concat("\r\n\r\n");
  response.concat(json);

  return response;
}


/// @brief generate a string representation of an http response which includes webpage data (html, css, or js) as the body
/// @param content String representation of the content to send
/// @param type enum for type of file to be sent over http, HTML, CSS, or JS
/// @return String, represents the http response contianing file content
String Responses::file_response(String content, FileType type) {
  String response = Responses::response(200, true);

  switch(type) {
    case HTML:
      response.concat("Content-Type: text/html\r\n");
      break;
    case CSS:
      response.concat("Content-Type: text/css\r\n");
      break;
    case JS:
      response.concat("Content-Type: application/javascript\r\n");
      break;
  }

  response.concat("Content-Length: ");
  response.concat(content.length());
  response.concat("\r\n\r\n");
  response.concat(content);

  return response;
}


/// @brief parse a response to a ParsedResponse object from a received string
/// @param response string representing raw http response
/// @return ParsedResponse, response object to be used in the code
ParsedResponse Responses::parse_response(String response) {
  ParsedResponse response_struct(DynamicJsonDocument(CONFIG_JSON_SIZE));

  response_struct.status = Responses::regex.GET_STAT.match(response).toInt();

  String json = Responses::regex.GET_JSON.match(response);
  if(json.length()) {
    DynamicJsonDocument doc = DynamicJsonDocument(CONFIG_JSON_SIZE);
    deserializeJson(doc, json);
    response_struct.body = doc;
  }
  return response_struct;
}