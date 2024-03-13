class StageBroadcasting : public ConnectionStage<WiFiUDP, UDPClient, ConnectionInformation> {
  private:
  ConnectionInformation multicast_information;
  ConnectionInformation local_information;
  const Identifiers &device_identifiers;
  bool received_acknowledgement = false;

  public:
  StageBroadcasting(ConnectionInformation local_information, ConnectionInformation multicast_information, const Identifiers &device_identifiers);
  StageReturn<ConnectionInformation> *run() override;
};

class StageAssociation : public ConnectionStage<WiFiSSLClient, TCPRequestClient, AssociationReturnStruct> {
  private:
  ConnectionInformation server_information;
  Identifiers &identifiers;
  MachineState &machine_state;

  NetworkException test_server_connection();
  Request generate_registration_request();

  public:
  StageAssociation(ConnectionInformation server_information, Identifiers &identifiers, MachineState &machine_state);
  StageReturn<AssociationReturnStruct> *run() override;
};