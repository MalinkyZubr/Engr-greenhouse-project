#include "network_client.hpp"


///////////////////////////////////////////////////
///////// NetworkClient ///////////////////////////
///////////////////////////////////////////////////

NetworkClient::NetworkClient(Layer4Encryption layer_4_encryption = ENCRYPTION_NONE) : layer_4_encryption_type(layer_4_encryption) {}

Layer4Encryption NetworkClient::get_encryption() {
  return this->layer_4_encryption_type;
}

NetworkException NetworkClient::client_receive_message(WiFiClient &client, int timeout, String *message) {
  String current_line = "";

  long long start_time = millis();

  while(client.connected() && ((millis() - start_time) < timeout)) {
    if(client.available()) {
      char c = client.read();

      start_time = millis();
      if(c == '\n' && current_line.equals("")) {
        if(current_line.length() == 0) {
          break;
        }
        else {
          message->concat(current_line + '\n');
          current_line = "";
        }
      }
      else if(c != '\r') {
        current_line.concat(c);
      }
    }
  }
  if(!client.connected()) {
    return NETWORK_SERVER_CONNECTION_FAILURE;
  }
  else if(((millis() - start_time) > timeout)) {
    return NETWORK_TIMEOUT;
  }
  return NETWORK_OKAY;
}

NetworkException NetworkClient::client_send_message(WiFiClient &client, HTTPMessage *message) {
  NetworkException return_error = NETWORK_OKAY;
  String serialized_message = message->serialize();

  bool status = client.println(serialized_message);
  if(!status) {
    return_error = NETWORK_SERVER_CONNECTION_FAILURE;
  }

  delete message;
  return return_error;
}

HTTPMessage* NetworkClient::parse_message(String &unparsed_message) {
  HTTPMessage *received = nullptr;

  if(unparsed_message.startsWith("HTTP")) {
    received = new Response(unparsed_message);
  }
  else {
    received = new Request(unparsed_message);
  }

  return received;
}

HTTPMessage* NetworkClient::receive_and_parse(WiFiClient &client, int timeout=5000) {
  String unparsed;
  HTTPMessage *received;

  NetworkException receive_error = this->client_receive_message(client, timeout, &unparsed);
  if(receive_error == NETWORK_OKAY) {
    received = this->parse_message(unparsed);
  }
  else {
    received->set_exception(receive_error);
  }

  return received;
}