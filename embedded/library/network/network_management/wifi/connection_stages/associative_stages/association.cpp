///////////////////////////////////////////////////
///////// Association /////////////////////////////
///////////////////////////////////////////////////

StageAssociation::StageAssociation(ConnectionInformation server_information, Identifiers &identifiers, MachineState &machine_state) : server_information(server_information), identifiers(identifiers), machine_state(machine_state) {
  this->set_handler(new TCPRequestClient(this->get_listener(), ENCRYPTION_SSL, this->server_information));
}

/// @brief function designed to connect to server using the object's ssl client. Makes 5 attempts at 5 second intervals to connect
/// @return if the function fails to connect after 5 attempts, return false
NetworkException StageAssociation::test_server_connection() {
  for(int x = 0; x < 3; x++) {
    if(this->get_listener().connectSSL(IPAddressExtended((char*)this->server_information.ip.c_str()), this->server_information.port)) {
      delay(1000);
      this->get_listener().stop();
      return NETWORK_OKAY;
    }
    delay(5000);
  }
  return NETWORK_SERVER_CONNECTION_FAILURE;
}

Request StageAssociation::generate_registration_request() {
  Request registration_request(POST, "/devices/confirm", this->server_information.ip, this->identifiers.get_device_id(), this->machine_state.get_state());
  return registration_request;
}

StageReturn<AssociationReturnStruct>* StageAssociation::run() {
  AssociationReturnStruct received_configuration;
  NetworkException exception;
  Identifiers identifier;
  Preset preset;
  StageReturn<AssociationReturnStruct>* stage_return = new StageReturn<AssociationReturnStruct>();

  Request registration_request;
  Response registration_response;

  int failure_count = 0;
  bool associated = false;

  registration_request = this->generate_registration_request();

  while(!associated && failure_count < 3) {
    registration_response = this->get_handler()->request(&registration_request);
    exception = registration_response.get_exception();

    received_configuration.identifier.from_json(registration_response.get_body());
    received_configuration.preset.from_json(registration_response.get_body());

    switch(exception) {
      case NETWORK_SERVER_CONNECTION_FAILURE:
        failure_count++;
        if(failure_count >= 3) {
          stage_return->set_exception(exception);
        }
        break;
      case NETWORK_OKAY:
        associated = true;
        break;
    }
  }

  return stage_return;
}