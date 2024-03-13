class ConfigStruct {
  private:
  int flash_address;
  SPIFlash *flash;
  bool is_configured = false;

  public:
  ConfigStruct() {};
  ConfigStruct(SPIFlash *flash, int flash_address);

  virtual const DynamicJsonDocument to_json() const = 0;
  virtual StorageException from_json(const DynamicJsonDocument &data) = 0;

  void set_configured();
  void set_unconfigured();
  bool check_is_configured() const;

  StorageException write(const DynamicJsonDocument &data);
  bool read();
  bool erase();
};

/// @brief global device state tracker for both connection and operational states. Critical for state driven operations on the device
class MachineState : public ConfigStruct {
  private:
  MachineOperationalState operational_state;

  public:
  MachineState(SPIFlash *flash, int flash_address);

  MachineOperationalState get_state() const;
  void set_state(MachineOperationalState state);

  StorageException from_json(const DynamicJsonDocument &data);
  const DynamicJsonDocument to_json() const override ;
};

enum MeasurementCompliance{LOWER, HIGHER, COMPLIANT};

class Preset : public ConfigStruct {
  private:
  float temperature;
  float humidity;
  float moisture;
  float hours_daylight;
  int preset_id = -1;

  MeasurementCompliance check_measurement_compliance(float desired, float real) const;

  public:
  Preset() {};
  Preset(SPIFlash *flash, int flash_address);

  float get_temperature() const;
  float get_humidity() const;
  float get_moisture() const;
  float get_hours_daylight() const;
  float get_preset_id() const;

  MeasurementCompliance check_temperature_compliance(float measured) const;
  MeasurementCompliance check_humidity_compliance(float measured) const;
  MeasurementCompliance check_moisture_compliance(float measured) const;

  StorageException from_json(const DynamicJsonDocument &data) override;
  const DynamicJsonDocument to_json() const override;
};

class Identifiers : public ConfigStruct {
  private:
  int device_id = -1;
  int project_id = -1;
  String device_name;

  public:
  Identifiers() {};
  Identifiers(SPIFlash *flash, int flash_address);

  int get_device_id() const;
  int get_project_id() const;
  String get_device_name() const;

  StorageException from_json(const DynamicJsonDocument &data) override;
  const DynamicJsonDocument to_json() const override;
};

class WifiInfo : public ConfigStruct {
  private:
  WifiNetworkTypes type;
  int channel;
  String ssid;
  String username = "";
  String password = "";

  public:
  WifiInfo::WifiInfo(SPIFlash *flash, int flash_address);
  WifiInfo() {};
  WifiInfo(String ssid, int channel);
  WifiInfo(String ssid, int channel, String password);
  WifiInfo(String ssid, int channel, String password, String username);

  WifiNetworkTypes get_type() const;
  String get_ssid() const;
  String get_username() const;
  String get_password() const;
  int get_channel() const;

  bool copy(WifiInfo to_copy);

  StorageException from_json(const DynamicJsonDocument &data) override;
  const DynamicJsonDocument to_json() const override;
};