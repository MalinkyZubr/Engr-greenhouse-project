///////////////////////////////////////////////////
///////// Broadcasting ////////////////////////////
///////////////////////////////////////////////////

StageBroadcasting::StageBroadcasting(ConnectionInformation local_information, ConnectionInformation multicast_information, const Identifiers &device_identifiers) : local_information(local_information), multicast_information(multicast_information), device_identifiers(device_identifiers), ConnectionStage(multicast_information.port) {
  this->set_handler(new UDPClient(this->get_listener(), local_information, multicast_information));
}

StageReturn<ConnectionInformation>* StageBroadcasting::run() {
  NetworkException exception;
  ConnectionInformation returned_connection_information;
  StageReturn<ConnectionInformation>* stage_return = new StageReturn<ConnectionInformation>();

  UDPRequest request(this->local_information.ip, this->device_identifiers);
  UDPResponse response;

  bool received_acknowledgement = false;

  while(received_acknowledgement) {
    exception = this->get_handler()->send_udp(request);
    response = this->get_handler()->receive_udp(5000);
    exception = response.get_exception();

    switch(exception) {
      case NETWORK_TIMEOUT:
        break;
      case NETWORK_OKAY:
        returned_connection_information.ip = response.get_server_ip();
        returned_connection_information.port = response.get_server_port();
        stage_return->set_return_value(returned_connection_information);

        this->received_acknowledgement = true;
        break;
    }
  }

  return stage_return;
}