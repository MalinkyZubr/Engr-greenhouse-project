# Greenhouse Project

Embedded Systems (Microcontroller) 

On the electronics front, our design centers around an Arduino MEGA (ATMega2560) microcontroller which serves as a hub for network interfacing, sensor systems, power, and environmental control systems.  

Greenhouse Software Task Management 

Our device (greenhouse) embedded systems code is written in C++, and operates with a pseudo-asynchronous, timer-based task scheduling system. Given the limited resources of a microcontroller, to send, receive, and process messages while controlling peripherals, and taking sensor readings at the “same” time requires the use of interrupts and the built in hardware timers of the device. A connection manager class listens for server messages if the device is on, and at regular intervals, the Arduino hardware timer sends an interrupt signal and executes a series of tasks. This allows the simulation of concurrent processes.  

      Sensors 

The greenhouse keeps track of 4 different environmental factors: temperature, humidity, soil moisture, and light exposure. Every task cycle, this sensor data is written to a buffer to await being sent to the server, or written to flash memory. 

Environment Management 

In tandem with its sensor array, the greenhouse sports heating elements, water pumps, fans, and LED ring fixtures designed to maintain the environment parameters specified in the loaded device preset. The preset just holds target environment levels for the greenhouse to maintain. Temperature, humidity, and moisture all receive a 1% tolerance about a target value. If the sensor reading for any of these parameters drops below or goes above this target range, the environmental control devices are activated to compensate.  

In the case of the light fixture, the microcontroller tracks daily sunlight exposure. If the ambient light level exceeds the brightness threshold to be considered sunny for a specified amount of time per day, the LED fixture never turns on. However, if not enough sunlight is detected by the time the sun sets, the LED fixture turns on to compensate. 

 

Figure 10: Sensor Environment Control Feedback Flowchart 

 
      Greenhouse Handshake Protocol 
	To connect the greenhouse to the server, we developed a 4-step connection protocol. At the first stage, initialization, the greenhouse starts its own Wi-Fi network, and waits for a user to connect to that Wi-Fi network. The greenhouse prompts the user to open a webpage containing a form to enter Wi-Fi credentials to another available network. Upon receiving these credentials, the greenhouse connects to the Wi-Fi network specified by the user in the form. This is how the greenhouse can get onto a home or company Wi-Fi without directly connecting to the greenhouse via serial. Although this connection is not HTTPS secured, it is made over WPA-2 (enabled by the AP), and so is still moderately secure. 

The second step of the protocol, broadcasting, commences after a successful connection by the greenhouse to a Wi-Fi network. At this stage, the greenhouse periodically sends out multicast UDP messages, which will be received by any server connected to the same Wi-Fi network. This acts as a lighthouse for the greenhouse the server can use to identify it. When the client wants to activate a device, they must access the server’s web portal to send an acknowledgement request to the device to proceed to the next step. 

When the greenhouse receives an acknowledgment of its UDP broadcasts from the server, it begins the next step, association. The greenhouse attempts to make an encrypted HTTPS connection to the server, and when successful synchronizes its own memory to the server database. In other words, the greenhouse tells the server, “I have this device ID, I am assigned to this project, and this preset,” and the server either responds, “Alright, that sounds good," or “Alright, but change your project/preset to X.” At this point, if it is the greenhouse’s first time connecting, it gains an entry in the server database for persistent association with that server. 

Finally, after all previous steps are completed, the greenhouse has a fully encrypted HTTPS session with the server and can begin regulating and taking measurements from the environment. The code provides a RESTful-style routing system to receive requests from the server and execute corresponding tasks on the microcontroller, making it easy to configure and manage greenhouse devices over Wi-Fi. 

                                           

Figure 11: Handshake Protocol Flowchart 

 

      Error Handling and Recovery 

Given the high integration of the greenhouse with Wi-Fi, we developed a robust error-handling system to recover from internet and power interruptions. If the greenhouse loses connection to the server, it can continue operating normally to maintain its environment based on the parameters specified to it at setup in the preset. Until connection to the server is re-established, it writes sensor data to local flash memory, whereupon reconnection can be loaded back into RAM and sent to the server. The greenhouse also has measures in place to automatically re-associate with the server, skipping the slower parts of the authentication process to ensure seamless recovery from network failure. 

In the event of power failure, although the greenhouse will shut off, its configuration will remain intact in flash memory. As such when power returns, the greenhouse can resume normal operations without manual reconfiguration. 

 

Figure 12: Error recovery flowchart 

 

Server 

The server-side software of the project is responsible for aggregating greenhouse data and providing scalable management for several greenhouses. It is the middle ground between the front-facing client web portal and the greenhouses themselves. The server hosts a MySQL database containing project, preset, and device data, with support for archiving of projects and device data.  

For communication with both the web portal and greenhouses, we developed a Python FastAPI-based interface, which combines speed and simplicity appropriate for this design. All these services are designed to work on top of an Apache webserver running HTTPS to allow for secure communications, run from inside a docker container for easy, user-friendly deployments of server infrastructure. Overall, it provides an effective nexus of authentication and data management for the greenhouses and users. 

 

Web Portal 

To facilitate easy user access to the server and embedded system resources, we developed a web portal on top of our server infrastructure. This remains unchanged from the original design except for the addition of a few new pages for more intuitive user interaction.  