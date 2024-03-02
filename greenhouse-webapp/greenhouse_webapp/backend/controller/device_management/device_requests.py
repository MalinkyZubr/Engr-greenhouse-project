class DeviceRequest(ABC):
    """Interface for sending requests to connected IoT devices unsolicited by said devices

    Args:
        route, str: route to send the data to
        method, Literal["POST", "PUT", "GET"]: method with which to send the http request

    Raises:
        HTTPException: 500 in case the request fails
    """
    route: str
    method: Literal["POST", "PUT", "GET"]
    
    __attempt_interval: int = 3
    __attempt_count: int = 3
    
    @abstractmethod
    async def call(self, ip: str, data: Optional[Type[BaseModel]] = None) -> Response:
        pass
        
    async def submit_request(self, ip: str, data: Optional[Type[BaseModel]] = None) -> Response:
        """Standard request function for submitting request to IoT devices

        Args:
            ip (str): ip to send request to
            data (Optional[Type[BaseModel]], optional): _description_. Defaults to None.

        Raises:
            HTTPException: _description_

        Returns:
            Response: _description_
        """
        response: Response | None = None
        for _ in range(self.__attempt_count):
            try:
                if self.method == "GET":
                    response: Response = requests.get(f"{ip}{self.route}", timeout=self.__attempt_interval)
                    break
                elif self.method == "PUT":
                    response: Response = requests.put(f"{ip}{self.route}", data=data.model_dump_json(), timeout=self.__attempt_interval)
                    break
                elif self.method == "POST":
                    response: Response = requests.post(f"{ip}{self.route}", data=data.model_dump_json(), timeout=self.__attempt_interval)
                    break
            except Timeout:
                await asyncio.sleep(self.__attempt_interval)
                continue
        if not response:
            raise HTTPException(500, detail="Request to device failed!")
        
        return response
     
        
class UnregisterDeviceRequest(DeviceRequest):
    route = "/unregister"
    method = "PUT"
    
    async def call(self, ip: str) -> Response:
        """Unregistering the device will purge its entry from the database, and remove all stored data from the device. THe device will thereafter
        re-enter a 

        Args:
            ip (str): ip to send the request to

        Raises:
            HTTPException: 500 in case the request fails
            
        Returns:
            Response: response from IOT device
        """
        return await self.submit_request(ip)
    

class ConfigureDeviceRequest(DeviceRequest):
    route = "/configure_identifiers"
    method = "PUT"
    
    async def call(self, ip: str, data: DeviceConfigurationSchema) -> Response:
        """send a request to configure device identifying information

        Args:
            ip, str: ip to send request to
            data, DeviceConfigurationSchema: schema which contains configuration data to send to the device

        Raises:
            HTTPException: 500 in case the request to the device fails
            
        Returns:
            Response: response from IOT device
        """
        return await self.submit_request(ip, data=data)
    
    
class ConfigureDevicePreset(DeviceRequest):
    route = "/configure_preset"
    method = "PUT"
    
    async def call(self, ip: str, data: DeviceRegistrationPreset) -> Response:
        """make a request to the device to configure its preset information

        Args:
            ip (str): ip of the device to be configured
            data (DeviceRegistrationPreset): the preset schema to be sent to the device

        Raises:
            HTTPException: 500 in case the request to the device fails
            
        Returns:
            Response: response from IOT device
        """
        return await self.submit_request(ip, data=data)
    
    
class SetDeviceStatus(DeviceRequest):
    route = "/pause"
    method = "POST"
    
    async def call(self, ip: str, pause_device: bool):
        """makes a call to the device to set its state to either paused or active

        Args:
            ip (str): destination ip to which request will be sent
            pause_device (bool): True if want to pause the device, False if want to activate it

        Raises:
            HTTPException: 500 in case the request to the device fails
            
        Returns:
            Response: response from IOT device
        """
        return await self.submit_request(ip, BaseModel(paused = pause_device))
    

change_preset: ConfigureDevicePreset = ConfigureDevicePreset()
change_identifier: ConfigureDeviceRequest = ConfigureDeviceRequest()
device_unregister: UnregisterDeviceRequest = UnregisterDeviceRequest()
set_device_status: SetDeviceStatus = SetDeviceStatus()