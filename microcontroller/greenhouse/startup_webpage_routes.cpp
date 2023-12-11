#include "startup_webpage_routes.hpp"


// implement builder method instead of all these subclasses. Templates and builder will work fine!!!
ReceiveCredentials::ReceiveCredentials(const String route, WifiInfo *temporary_wifi_info) : StartupRouter::Route(route), temporary_wifi_info(temporary_wifi_info) {} 

StartupRouter::StartupRouter(WifiInfo* temporary_wifi_information) {
    this->add_route(new ReceiveCredentials("/submit", temporary_wifi_information));
    this->add_route(new SendHTML("/"));
    this->add_route(new SendCSS("/styles"));
    this->add_route(new SendJS("/script"));
}