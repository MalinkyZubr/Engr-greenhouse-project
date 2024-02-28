#ifndef STARTUP_WEBPAGE_ROUTES_HPP
#define STARTUP_WEBPAGE_ROUTES_HPP

#include "http.hpp"
#include "router.hpp"
#include "storage.hpp"
#include "async.hpp"
#include "webpage.hpp"
#include "network_utils.hpp"


class ReceiveCredentials : public Route {
    private:
    WifiInfo *temporary_wifi_info;
    bool *wifi_configured_flag;

    public:
    ReceiveCredentials(const char* route, const Method method, WifiInfo *temporary_wifi_info, bool *wifi_configured_flag);
    Response execute(const Request &request) override;
};

class SendHTML : public Route {
    public:
    SendHTML(const char* route, const Method method);
    Response execute(const Request &request) override;
};

class SendCSS : public Route {
    public:
    SendCSS(const char* route, const Method method);
    Response execute(const Request &request) override;
};

class SendJS : public Route {
    public:
    SendJS(const char* route, const Method method);
    Response execute(const Request &request) override;
};


#endif