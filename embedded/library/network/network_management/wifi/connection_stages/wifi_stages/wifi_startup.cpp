///////////////////////////////////////////////////
///////// StageWifiStartup ////////////////////////
///////////////////////////////////////////////////

StageWifiStartup::StageWifiStartup(WifiInfo &wifi_info) : wifi_info(wifi_info), StageWifiBaseClass(9999) {} // stage shouldnt have active server, fix that

StageReturn<WifiInfo>* StageWifiStartup::run() {
  NetworkException exception;
  StageReturn<WifiInfo>* stage_return = new StageReturn<WifiInfo>();

  exception = this->connect_wifi(this->wifi_info);

  stage_return->set_exception(exception);
  stage_return->set_return_value(this->wifi_info);

  return stage_return;
}