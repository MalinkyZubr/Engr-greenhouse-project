#ifndef STARTUP_WEBPAGE_ROUTES_HPP
#define STARTUP_WEBPAGE_ROUTES_HPP

#include "wifi.hpp"
#include "http.hpp"
#include "router.hpp"
#include "storage.hpp"
#include "async.hpp"
#include "webpage.hpp"

typedef struct {
    bool wifi_updated = true;
} WifiUpdatedFlag;

class StartupRouter : public Router<WifiUpdatedFlag> {
    StartupRouter(WifiInfo* temporary_wifi_information);
};

class ReceiveCredentials : public StartupRouter::Route {
    private:
    WifiInfo *temporary_wifi_info;

    public:
    ReceiveCredentials(const String route, WifiInfo *temporary_wifi_info);
    RouteReturn<R> execute(Request &request) override;
};

class SendHTML : public StartupRouter::Route {
    public:
    SendHTML(const String route);
    RouteReturn<R> execute(Request &request) override;
};

class SendCSS : public StartupRouter::Route {
    public:
    SendCSS(const String route);
    RouteReturn<R> execute(Request &request) override;
};

class SendJS : public StartupRouter::Route {
    public:
    SendJS(const String route);
    RouteReturn<R> execute(Request &request) override;
}

#endif