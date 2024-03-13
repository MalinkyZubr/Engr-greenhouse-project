class StageWifiBaseClass : public ConnectionStage<WiFiServer, TCPListenerClient, WifiInfo> {
  private:
  bool enterprise_connect(WifiInfo &info);
  bool home_connect(WifiInfo &info);

  public:
  StageWifiBaseClass(int listener_port) : ConnectionStage(listener_port) {};
  NetworkException connect_wifi(WifiInfo &info);
};

// add common templated interface for the message handler
class StageWifiInitialization : public StageWifiBaseClass {
  private:
  WifiInfo temporary_wifi_information;
  bool wifi_configured = false;

  String ssid_config();
  bool start_access_point();
  NetworkException receive_credentials();

  public:
  StageWifiInitialization(int listen_port);
  StageReturn<WifiInfo> *run() override;
};

class StageWifiStartup : public StageWifiBaseClass {
  private:
  WifiInfo &wifi_info;

  public:
  StageWifiStartup(WifiInfo &wifi_info);
  StageReturn<WifiInfo> *run() override;
};