



///////////////////////////////////////////////////
///////// Connectionmanager ///////////////////////
///////////////////////////////////////////////////

ConnectionManager::ConnectionManager(ConnectionInformation local_connection_information, ConnectionInformation multicast_information, StorageManager *global_storage) : local_connection_information(local_connection_information), multicast_information(multicast_information), global_storage(global_storage) {
  if(this->global_storage->get_wifi().get_channel() == -1) {
    this->set_stage_object(STAGE_INITIALIZING);
  }
  else {
    this->set_stage_object(STAGE_STARTUP);
  }
}

void ConnectionManager::set_stage_object(ConnectionStageIdentifier stage) {
  delete this->stage;

  this->stage_identifier = stage;

  switch(stage) {
    case STAGE_INITIALIZING:
      this->stage = new StageWifiInitialization(this->local_connection_information.port);
      break;
    case STAGE_STARTUP:
      this->stage = new StageWifiStartup(this->global_storage->get_wifi());
      break;
    case STAGE_BROADCASTING:
      this->stage = new StageBroadcasting(this->server_information, this->multicast_information, this->global_storage->get_identifiers());
      break;
    case STAGE_ASSOCIATING:
      this->stage = new StageAssociation(this->server_information, this->global_storage->get_identifiers(), this->global_storage->get_machine_state());
      break;
    case STAGE_CONNECTED:
      this->stage = new StageFullConnection(this->server_information, this->global_storage);
      break;
  }
}

void ConnectionManager::handle_error(NetworkException exception) {
  switch(exception) {
    case NETWORK_WIFI_FAILURE:
      this->set_stage_object(STAGE_STARTUP);
      break;
    case NETWORK_AUTHENTICATION_FAILURE:
      this->set_stage_object(STAGE_INITIALIZING);
      break;
    case NETWORK_SERVER_CONNECTION_FAILURE:
      this->set_stage_object(STAGE_BROADCASTING);
      break;
  }
}

void ConnectionManager::handle_returns(StageReturnBase *return_value) {
  if(return_value->get_exception() != NETWORK_OKAY) {
    this->handle_error(return_value->get_exception());
    return;
  }

  switch(this->stage_identifier) {
    case STAGE_INITIALIZING: // make this a proper state machine. Reference to the context inside each state to automatically advance the state. 
      this->global_storage->get_wifi().copy(dynamic_cast<StageReturn<WifiInfo>*>(return_value)->get_return_value());
      this->set_stage_object(STAGE_BROADCASTING);
      break;
    case STAGE_STARTUP:
      this->set_stage_object(STAGE_BROADCASTING);
      break;
    case STAGE_BROADCASTING:
      this->server_information = dynamic_cast<StageReturn<ConnectionInformation>*>(return_value)->get_return_value();
      this->set_stage_object(STAGE_ASSOCIATING);
      break;
    case STAGE_ASSOCIATING:
      this->set_stage_object(STAGE_CONNECTED);
      this->global_storage->set_network_state(MACHINE_CONNECTED);
      break;
    case STAGE_CONNECTED:
      this->set_stage_object(STAGE_ASSOCIATING);
      this->global_storage->set_network_state(MACHINE_DISCONNECTED);
      break;
  }

  delete return_value;
}

Response ConnectionManager::send_request(Request &request) const {
  Response response;

  if(this->stage_identifier == STAGE_CONNECTED) {
    StageFullConnection* stage = dynamic_cast<StageFullConnection*>(this->stage);
    response = stage->send_request(request);
  }
  else {
    response.set_exception(NETWORK_NOT_ACTIVE);
  }

  return response;
}

const ConnectionInformation& ConnectionManager::get_server_information() const {
  return this->server_information;
}

void ConnectionManager::run() {
  StageReturnBase *return_value;

  return_value = this->stage->run();

  this->handle_returns(return_value);
}

void ConnectionManager::set_stage_identifier(ConnectionStageIdentifier stage) {
  this->stage_identifier = stage;
}

ConnectionStageIdentifier ConnectionManager::get_stage_identifier() const {
  return this->stage_identifier;
}