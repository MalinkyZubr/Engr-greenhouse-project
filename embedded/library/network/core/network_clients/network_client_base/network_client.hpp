class NetworkClient {
  private:
  Layer4Encryption layer_4_encryption_type = ENCRYPTION_NONE;

  HTTPMessage* parse_message(String &unparsed_message);
  NetworkException client_receive_message(WiFiClient &client, int timeout, String *message);

  public:
  NetworkClient(Layer4Encryption layer_4_encryption);
  Layer4Encryption get_encryption();

  NetworkException client_send_message(WiFiClient &client, HTTPMessage *message); // must free dynamically allocated message
  HTTPMessage* receive_and_parse(WiFiClient &client, int timeout);
};

class TCPRequestClient : private NetworkClient {
  private:
  WiFiClient &client;
  ConnectionInformation server_information;

  Response receive_response(WiFiClient &client);

  public:
  TCPRequestClient(WiFiClient &client, Layer4Encryption layer_4_encryption, ConnectionInformation &server_information);

  Response request(Request *message);
};

class TCPListenerClient : private NetworkClient {
  private:
  WiFiServer &listener;
  Request* request_queue;
  Router router;

  NetworkException respond(Response &response, WiFiClient &client);

  public:
  TCPListenerClient(WiFiServer &listener, Layer4Encryption layer_4_encryption, Router router);
  NetworkException listen();
};

class UDPClient {
  private:
  WiFiUDP &udp_server;
  ConnectionInformation multicast_information;

  public:
  UDPClient(WiFiUDP &udp_server, ConnectionInformation &local_information, ConnectionInformation &multicast_information);
  NetworkException send_udp(UDPRequest &request);
  UDPResponse receive_udp(int timeout);
};