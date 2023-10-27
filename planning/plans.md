# Microcontroller Planning
## Microcontroller Side
### Base Requirements
* Microcontroller should be able to collect data on:
  * humidity
  * temperature
  * light
  * Infrared
  * soil moisture
* Microcontroller should be able to respond to collected conditions by:
  * dispensing water onto the plant with pump
  * turning on LEDs
  * increase temperature with heating pad
* Microcontroller should be configurable over wifi:
  * UDP
  * should work on enterprise network
  * must have stable authentication
  * transmit requested data to server
  * microcontroller should be able to receive authentication information via serial terminal

### Additional (best case) features
* water level sensor to tell when water pump must be refilled

## Server Side
### Base Requirements
* should be able to discover and pair with greenhouse clients (effectively, the greenhouse will be the actual server)
* be able to store data entries in a database
* be able to configure parameters for the greenhouse to maintain
* be able to present data, and accept queries
* should be able to support multiple units

## Important questions:
* What plant is best for the test?
  * I will do tomato
* What kind of light do plants like?
  * red + blue is the best combo
  * flowering plants also need good IR!
* Do plants like constant termperature and light throughout the day?
  * flowering and fruiting plants need periods of dark too! Different plants need different amounts of light per day
  * night temperatures should be 10-15 degrees cooler than the day temperatures
* is constant or intermitent humidity the best?
  * constant humidity is best

## Revised Requirements
### Microcontroller
* for light:
  * must be able to keep track of total light throughout the day, checking light exposure at certain intervals throughout the day. 
  * If at the end of the day there was not enough consistent light, LEDs will be turned on for an interval equalling the sum of interval times light was insufficient
* for IR:
  * same operations as regular light
* Temperature
  * unit must know the desired temperature of the plant
  * when temperature drops below threshold, turn on heater
  * at night, drop average desired temeprature by 10 degrees
* for soil moisture and humidity
  * if these drop below a level, turn on water spray
* In case connection is lost, the data should be added to a buffer. The client should maintain an identifier separate from IP so that they can reconnect to the desired project after

### server side
* user should be able to create presets for different plants, that can have different requirements and be reused
* server should be able to support multiple greenhouses at once, and coordinate them
* server should be able to easily discover clients (greenhouses)
* server should store presets in its own database table, and collected data in another. 
  * each project should have its own identifier whcih can be used to query data from specific projects
* should provide timekeeping for the clients. At every data exchange for information from sensors, sync the times
* microcontrollers should send the status of their active devices as well, like heaters, lights, and pumps
  * when a device turns on, the iot device should send a logging message

### ALTERNATIVELY
* Greenhouse connects directly to the webserver and makes http requests

### Frontend
* View data in tabulated or graphed format for current and past projects
* create projects
  * projects should contain a list of clients and a preset
  * projects can be 'active' or 'complete'
* create, configure, and delete presets
* assign projects to presets and clients

### Alternative connection structure
* during normal operations the greenhouses send requests directly to the HTTP server
* when setting up, a separate UDP client should be used for configuration and discovery on the LAN

### More questions
* for data collection, should the server make a request to the iot device, or should the iot device make a request to the server containing the data?
* same goes for management
* should decisions, such as turning on the light be delegated to the server, or should they be delegated to the iot device?

### Push or pull model?
* push model
  * the greenhouse sends update messages to the server
  * advantages:
    * can send updates for equipment status as events occur (eg the heater turns on, send notification)
    * the server does not need to have code for sending requests to the iot devices
  * disadvantages:
    * means of pulling from (to) the greenhouses is still needed for remote configuration
* pull model
  * the server asks for data at intermittent intervals
  * advantages:
    * standard time intervals for gathering data
    * already needed for configuration
  * disadvantages
    * does not support updating
* could just implement both

### Need Non Volatile Flash Memory!!
* or you could just use the mac address
* but what if I want to save configuration?

### Additional requirement!
* There should be local logging to write to an SD card, so upon failure, this can be inspected

### Questions
* with projects, should each project have a globally applied preset, or should each device in a project have its own preset
* individual devices, probably, since this is small scale

### Logging
* logging should take place over network and to an SD card
* each device should have its own log file in its own project directory. Those logs are persistent even if the device disconnects or changes projects