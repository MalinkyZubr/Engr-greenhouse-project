class CommonDataBuffer {
  private:
  float temperature;
  float humidity;
  float moisture;
  float light_exposure;

  long long ms_sunlight;
  bool night_time;

  long long last_measurement;

  public:
  void increment_light(bool increment);
  void set_common_data(float temperature, float humidity, float moisture, float light_exposure);

  const float get_temperature() const;
  const float get_humidity() const;
  const float get_moisture() const;
  const float get_light_exposure() const;
  const long long get_ms_light() const;
  const bool is_night_time() const;

  void set_night_time(bool is_night_time);

  const DynamicJsonDocument to_json() const;
};

class DataManager {
  private:
  SPIFlash *flash;

  int current;
  int partition_size = CONFIG_JSON_SIZE + 10;

  int flash_address;
  int max_size;

  bool is_full = false;
  bool is_storing;

  public:
  float reference_datetime = 0;

  DataManager(SPIFlash *flash, int start_address, int max_size);
  void set_reference_datetime(int timestamp);
  int get_end_address() const;

  bool check_is_storing() const;
  
  bool write_data(DynamicJsonDocument &data);
  bool read_data(DynamicJsonDocument &data_output);
  bool erase_data();
};