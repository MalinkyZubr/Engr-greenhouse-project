typedef struct {
  Identifiers identifier;
  Preset preset;
} AssociationReturnStruct;

class StageReturnBase {
  private:
  NetworkException exception;

  public:
  StageReturnBase() {};

  virtual NetworkException get_exception() = 0;
  virtual void set_exception(NetworkException exception) = 0;
};

template<typename R>
class StageReturn : public StageReturnBase {
  private:
  R return_value;

  public:
  StageReturn() {};

  R get_return_value();
  void set_return_value(R return_value);

  NetworkException get_exception();
  void set_exception(NetworkException exception);
};

class ConnectionStageBase {
  private:
  ConnectionManager connection_manager;
  
  public:
  virtual StageReturnBase *run() = 0;
  virtual ~ConnectionStageBase() {};
};

template<typename L, typename H, typename R>
class ConnectionStage : public ConnectionStageBase {
  private:
  L listener;
  H *message_handler;

  public:
  ConnectionStage() {};
  ConnectionStage(int listener_port) : listener(listener_port) {};

  L& get_listener();

  void set_handler(H* message_handler);
  H* get_handler();

  virtual StageReturnBase *run() override = 0;

  ~ConnectionStage();
};