class StageFullConnection : public ConnectionStage<WiFiServer, TCPListenerClient, bool> {
  private:
  WiFiSSLClient wifi_ssl_object;
  TCPRequestClient request_client;
  ConnectionInformation server_information;

  NetworkException handle_incoming();

  public:
  StageFullConnection(ConnectionInformation &server_information, StorageManager *global_storage);
  Response send_request(Request &request);
  StageReturn<bool>* run() override;
};