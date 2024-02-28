#include "startup_webpage_routes.hpp"

////////////////////////////////////////////////////
/////////////// ReceiveCredentials /////////////////
////////////////////////////////////////////////////

ReceiveCredentials::ReceiveCredentials(const char* route, const Method method, WifiInfo *temporary_wifi_info, bool *wifi_configured_flag) : Route(route, method), temporary_wifi_info(temporary_wifi_info), wifi_configured_flag(wifi_configured_flag) {} 

Response ReceiveCredentials::execute(const Request &request) {
  Response response;
  const DynamicJsonDocument &body = request.get_body();
  WifiInfo new_wifi_info;

  String ssid;
  WifiNetworkTypes type;
  int channel;

  ssid = (char*)body["ssid"];
  channel = get_ap_channel(ssid); // must have valid reference to this functuion
  
  switch(channel) {
    case -1:
      response = Response(503);
      return response;
    default:
      break;
  }

  if(strcmp((char*)body["type"], "enterprise")) {
    new_wifi_info = WifiInfo(ssid, channel, body["password"], body["username"]);
  }
  else if(strcmp((char*)body["type"], "home")) {
    new_wifi_info = WifiInfo(ssid, channel, body["password"]);
  }
  else {
    new_wifi_info = WifiInfo(ssid, channel);
  }
  
  response = Response(200);

  *this->wifi_configured_flag = true;
  
  return response;
}

////////////////////////////////////////////////////
/////////////// SendHTML ///////////////////////////
////////////////////////////////////////////////////

SendHTML::SendHTML(const char* route, const Method method) : Route(route, method) {}

Response SendHTML::execute(const Request &request) {
  Response response(200, HTML, webpage_html);
  return response;
}

////////////////////////////////////////////////////
/////////////// SendCSS ////////////////////////////
////////////////////////////////////////////////////

SendCSS::SendCSS(const char* route, const Method method) : Route(route, method) {}

Response SendCSS::execute(const Request &request) {
  Response response(200, CSS, webpage_css);
  return response;
}

////////////////////////////////////////////////////
/////////////// SendJS /////////////////////////////
////////////////////////////////////////////////////

SendJS::SendJS(const char* route, const Method method) : Route(route, method) {}

Response SendJS::execute(const Request &request) {
  Response response(200, JS, webpage_js);
  return response;
}