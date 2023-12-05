#include "http.hpp"

// need to fix http formatting, ensure it is all correct

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

Reg::Reg(char *pattern) {
  this->pattern.Match(pattern);
}

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

  free(converted);
  return result;
}


String Requests::request(methods method, String route, String host, int device_id, MachineOperationalState op_state) {
  String request = return_method(method);
  request.concat(" ");
  request.concat(route);
  request.concat(" HTTP/1.1\n");
  request.concat("Host: ");
  request.concat(host);
  request.concat("\n");
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
  request.concat("\n");

  return request;
}

String Requests::request(methods method, String route, String host, int device_id, MachineOperationalState op_state, DynamicJsonDocument body) {
  String json;
  serializeJson(body, json);
  String request = Requests::request(method, route, host, device_id, op_state);
  request.concat("\n");
  request.concat("Content-Type: application/json\n");
  request.concat("Content-Length: ");
  request.concat(json.length());
  request.concat("\n");
  request.concat(json);

  return request;
}

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
  response.concat("Content-Length: ");
  response.concat(json.length()); 
  response.concat("\n\n");
  response.concat(json);

  return response;
}

String Responses::file_response(String content, FileType type) {
  String response = Responses::response(200);

  switch(type) {
    case HTML:
      response.concat("Content-Type: text/html\n");
      break;
    case CSS:
      response.concat("Content-Type: text/css\n");
      break;
    case JS:
      response.concat("Content-Type: application/javascript\n");
      break;
  }

  response.concat("Content-Length: ");
  response.concat(content.length());
  response.concat("\n");
  response.concat(content);

  return response;
}

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