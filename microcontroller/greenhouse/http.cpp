#include "http.hpp"

// need to fix http formatting, ensure it is all correct

/// @brief return a string which corresponds to the http method enum value
/// @param method http method enum to convert from
/// @return String representing the stringified HTTP method
String return_method_str(Method method) {
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

Method return_method_enum(String &method) {
  Method method_enum;
  
  if(method.equals("GET")) {
    method_enum = GET;
  }
  else if(method.equals("POST")) {
    method_enum = POST;
  }
  else if(method.equals("PUT")) {
    method_enum = PUT;
  }
  else if(method.equals("DELETE")) {
    method_enum = DELETE;
  }
  else if(method.equals("PATCH")) {
    method_enum = PATCH;
  }
  return method_enum;
}


///////////////////////////////////////////////////
///////// Regex ///////////////////////////////////
///////////////////////////////////////////////////

/// @brief regex function to return a matching substring of a larger string
/// @param content string representing the entire string
/// @return String, the matching substring
String RegularExpressions::match(const String &content, const char* pattern_str) {
  MatchState pattern;

  char* char_array_content = new char[content.length() + 1];
  strcpy(char_array_content, content.c_str());

  pattern.Match(pattern_str);

  pattern.Target(char_array_content);

  int start = pattern.MatchStart;
  int length = pattern.MatchLength;

  String result = "";
  for(int x = start; x < start + length; x++) {
    result.concat(content[x]);
  }

  delete char_array_content;

  return result;
}

String RegularExpressions::get_body(const String &content) {
  return RegularExpressions::match(content, RegularExpressions::json_regex);
}

String RegularExpressions::get_content_length(const String &content) {
  return RegularExpressions::match(content, RegularExpressions::content_length_regex);
}

String RegularExpressions::get_host(const String &content) {
  return RegularExpressions::match(content, RegularExpressions::host_regex);
}

String RegularExpressions::get_route(const String &content) {
  return RegularExpressions::match(content, RegularExpressions::route_regex);
}

int RegularExpressions::get_status(const String &content) {
  return RegularExpressions::match(content, RegularExpressions::status_regex).toInt();
}

Method RegularExpressions::get_method(const String &content) {
  String method = RegularExpressions::match(content, RegularExpressions::method_regex);
  return return_method_enum(method);
}

///////////////////////////////////////////////////
///////// HTTPMessage /////////////////////////////
///////////////////////////////////////////////////

HTTPMessage::HTTPMessage() : body_type(NONE) {}

HTTPMessage::HTTPMessage(DynamicJsonDocument body) : body(body), body_type(JSON) {}

void HTTPMessage::set_body(DynamicJsonDocument body) {
  this->body = body;
  this->body_type = JSON;
}

BodyType HTTPMessage::get_body_type() {
  return this->body_type;
}

DynamicJsonDocument& HTTPMessage::get_body() {
  return this->body;
}

void HTTPMessage::set_exception(NetworkException exception) {
  this->exception = exception;
}

NetworkException HTTPMessage::get_exception() {
  return this->exception;
}

void HTTPMessage::set_body_type(BodyType type) {
  this->body_type = type;
}

///////////////////////////////////////////////////
///////// Request /////////////////////////////////
///////////////////////////////////////////////////

Request::Request(Method method, const char* route, String host, int device_id, MachineOperationalState machine_state) : method(method), route(route), host(host), device_id(device_id), machine_state(machine_state) {}

Request::Request(Method method, const char* route, String host, int device_id, MachineOperationalState machine_state, DynamicJsonDocument body) : method(method), device_id(device_id), machine_state(machine_state), route(route), host(host), HTTPMessage(body) {}

String Request::get_route() {
  return this->route;
}

Method Request::get_method() {
  return this->method;
}

Request::Request(String &unparsed) {
  this->method = RegularExpressions::get_method(unparsed);
  this->route = (char*)RegularExpressions::get_route(unparsed).c_str();
  this->host = RegularExpressions::get_host(unparsed);

  String json = RegularExpressions::get_body(unparsed);

  if(json.length()) {
    DynamicJsonDocument doc(CONFIG_JSON_SIZE);
    deserializeJson(doc, json);
    this->set_body(doc);
  }
}

Request::Request(Request* base_pointer) : method(base_pointer->method), 
  route(base_pointer->route), 
  host(base_pointer->host), 
  device_id(base_pointer->device_id),
  machine_state(base_pointer->machine_state) {
    if(base_pointer->get_body_type() == JSON) {
      this->set_body(base_pointer->get_body());
    }
    delete base_pointer;
}

String Request::serialize() {
  String request = return_method_str(this->method);
  String body;

  request.concat(" ");
  request.concat(this->route);
  request.concat(" HTTP/1.1");
  request.concat("\r\nHost: ");
  request.concat(this->host);
  request.concat("\r\nCookie: device_id=");
  request.concat(this->device_id);
  request.concat("; status=");
  switch(this->machine_state) {
    case MACHINE_ACTIVE:
      request.concat("ACTIVE");
      break;
    case MACHINE_PAUSED:
      request.concat("IDLE");
      break;
  }
  if(this->get_body_type() == NONE) {
    request.concat("\r\n\r\n");
  }
  else if(this->get_body_type()) {
    serializeJson(this->get_body(), body);
    request.concat("\r\nContent-Type: application/json");
    request.concat("\r\nContent-Length: ");
    request.concat(body.length());
    request.concat("\r\n\r\n");
  }

  return request;
}

///////////////////////////////////////////////////
///////// Response ////////////////////////////////
///////////////////////////////////////////////////

Response::Response(int status) : status(status) {}

Response::Response(int status, DynamicJsonDocument body) : status(status), HTTPMessage(body) {}

Response::Response(int status, BodyType file_type, const char* file_content) : status(status), file_content(file_content) {
  this->set_body_type(file_type);
}

Response::Response(String &unparsed) {
  this->status = RegularExpressions::get_status(unparsed);
  String json = RegularExpressions::get_body(unparsed);

  if(json.length()) {
    DynamicJsonDocument doc = DynamicJsonDocument(CONFIG_JSON_SIZE);
    deserializeJson(doc, json);
    this->set_body(doc);
  }
}

Response::Response(Response* base_pointer) : status(base_pointer->status) {
  if(strlen(base_pointer->file_content)) {
    this->file_content = base_pointer->file_content;
    this->set_body_type(base_pointer->get_body_type());
  }
  delete base_pointer;
} 

void Response::set_directive(DeviceDirective directive) {
  this->directive = directive;
}

DeviceDirective Response::get_directive() {
  return this->directive;
}

String Response::serialize() {
  String response = "HTTP/1.1 ";
  String body;

  response.concat(status);
  response.concat(" OK");
  
  if(!this->get_body_type()) {
    response.concat("\r\n\r\n");
  }

  else {
    serializeJson(this->get_body(), body);

    switch(this->get_body_type()) {
      case JSON:
        serializeJson(this->get_body(), body);
        response.concat("\r\nContent-Type: application/json");
        break;
      case HTML:
        body = *this->file_content;
        response.concat("\r\nContent-Type: text/html");
        break;
      case CSS:
        body = *this->file_content;
        response.concat("\r\nContent-Type: text/css");
        break;
      case JS:
        body = *this->file_content;
        response.concat("\r\nContent-Type: application/javascript");
        break;
    }
    response.concat("\r\nContent-Length: ");
    response.concat(body.length()); 
    response.concat("\r\n\r\n");
    response.concat(body);
  }

  return response;
}