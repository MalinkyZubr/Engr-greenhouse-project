///////////////////////////////////////////////////
///////// FullConnection //////////////////////////
///////////////////////////////////////////////////

StageFullConnection::StageFullConnection(ConnectionInformation &server_information, StorageManager *global_storage) : request_client(this->wifi_ssl_object, ENCRYPTION_SSL, server_information), server_information(server_information), ConnectionStage(server_information.port) {
  Router router;
  router
    .add_route(new NetworkReset("/reset/network", POST))
    .add_route(new DeviceReset("/reset/hard", DELETE))
    .add_route(new SetTime("/time", POST, global_storage))
    .add_route(new ConfigureDeviceIds("/configure/id", POST, global_storage))
    .add_route(new ConfigureDevicePreset("/configure/preset", POST, global_storage))
    .add_route(new PauseDevice("/configure/status", POST, global_storage)); // this should set the network manager state to paused

  this->set_handler(new TCPListenerClient(this->get_listener(), ENCRYPTION_SSL, router));
}

NetworkException StageFullConnection::handle_incoming() {
  NetworkException exception;
  int failure_count = 0;

  while(failure_count < 7) {
    exception = this->get_handler()->listen();

    switch(exception) {
      case NETWORK_TIMEOUT:
        break;
      case NETWORK_SERVER_CONNECTION_FAILURE:
        failure_count++;
        break;
      case NETWORK_OKAY:
        failure_count = 0;
        break;
    }
  }
  return exception;
}

Response StageFullConnection::send_request(Request &request) {
  Response response;

  response = this->request_client.request(&request);

  return response;
}

StageReturn<bool>* StageFullConnection::run() {
  StageReturn<bool>* stage_return = new StageReturn<bool>();
  stage_return->set_return_value(true);

  NetworkException exception;

  exception = this->handle_incoming();

  stage_return->set_exception(exception);

  return stage_return;
}