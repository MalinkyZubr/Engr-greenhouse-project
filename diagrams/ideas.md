should be able to set a device to sleep mode when configuring, or when want to not run
in a sleeping state network card will be active but the sensors will be deactivated
under this circumstance the arduino should go to deepsleep mode. It should wake up and send a UDP multicast every couple minutes

On turn on, the device will try to ping with the https to the server. If there is a 404, revert to UDP