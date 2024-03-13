class ControlDevice {
  public:
  bool status;
  int pin;

  Device(int pin);
  void set_high();
  void set_low();
};

class LedStrip {
  private:
  int led_count = 16;
  int standard_color[3] = {214, 83, 211};
  int startup_sequence_colors[3][3] = {
    {0,0,255},
    {0,255,0},
    {255,0,0}
  };
  int pin;

  Adafruit_NeoPixel led_strip;

  void set_strip(int color[], int delay_time, bool sequence);

  public:
  bool status;

  LedStrip(int pin, int led_count);
  void startup();
  void set_strip_high();
  void set_strip_low();
};

enum DeviceState{DEVICE_HIGH, DEVICE_LOW};

class EnvironmentManager : public Callable {
  private:
  Device pump;
  Device heater;
  Device fan;
  LedStrip led;
  StorageManager *global_storage;

  void set_devices_low(Device devices[]);
  void set_devices_high(Device devices[]);
  void set_device_statuses(MeasurementCompliance current_measurement_classification, Device set_high_when_higher[], Device set_low_when_higher[]);

  public:
  EnvironmentManager::EnvironmentManager(StorageManager *global_storage, int pump_pin, int heater_pin, int fan_pin, int led_pin, int num_led);

  void device_activation();
  void callback() override;
};