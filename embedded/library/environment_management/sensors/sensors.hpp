class Sensor {
  public:
  virtual float sense();
};

class Moisture : public Sensor{
  private:
  int pin;

  public:
  Moisture(int pin);
  float sense();
};

class Light : public Sensor {
  private:
  BH1750 light_sensor;
  bool status;

  public:
  Light();
  float sense();
};

class HumidityTemperature {
  private:
  int pin;

  DHT dht_sensor;

  public:
  typedef struct EnvReading {
    float temperature_c;
    float humidity_percent;
  }EnvReading;

  HumidityTemperature(int pin);
  EnvReading sense();
};

class Sensors : public Callable {
  private:
  StorageManager *global_storage;
  Moisture moisture;
  Light light;
  HumidityTemperature humtemp;

  public:
  Sensors(StorageManager* global_storage, int humtemp_pin, int moisture_pin);

  void submit_readings();
  void callback() override {
    this->submit_readings();
  }
};