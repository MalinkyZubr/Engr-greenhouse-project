## there must be a way for clients to connect to the server without being in serial with the microcontroller
* The device starts off as a wireless AP, supporting one connection
* serves managemenet webpage to the connecting device. Client must connect to the IP of the greenhouse, serving this webpage
* user enters the wifi network bssid, username, and password (must be configured for enterprise network, for now)
* the wifi network must be in soft ap mode so it can act as a client to the network, and as an AP to the user. In the atwinc 1500 this is hotspot mode
* this si the begin provision mode, and it automatically uses the webpage!!
* must configure the webpage to support the enterprise network, since it only takes SSID and password
* the files for the webpage are stored in flash memory, I should read it using my arduino in order to check out the file structure, and upload my own
* the original atmel http page has a device name field

* the ssid is the visual name of the AP
* migth need an annoying workaround that will receive creds, kill AP, check, then re-do ap mode if the creds are wrong

* still the problem remains, how is the initial connection to the server made?
* either:
  * the user enters the server IP during the privisoning, and it attempts an SSL connection
  * if changing provisioned html webpage doesnt work in the firmware, might just need to use UDP scan with some for of authentication -> ill just do this, I guess

* also, at every startup the device should dhcp request the same ip address!!

* THERE MUST BE SERVER LOGGING, LIKE CONNECTS AND DISCONNECTS