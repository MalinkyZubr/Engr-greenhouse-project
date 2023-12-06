from fastapi import APIRouter, Depends, Request
from fastapi.responses import HTMLResponse
from pydantic import BaseModel
from typing import Optional, Literal, Type
from starlette.responses import FileResponse
from fastapi.responses import JSONResponse
from fastapi.exceptions import HTTPException
import time
import asyncio
from datetime import datetime

import requests
from requests import Response
from requests.exceptions import Timeout

from controller.frontend_paths import SCAN, ASSIGN_PRES, ASSIGN_PROJ
from controller.DBIntRouter import APIDRouter
from controller.schemas.server_device_schemas import BaseSchema
from model.device_initialization import Device
from jinja2 import Template
from abc import ABC, abstractmethod


router = APIDRouter(
    prefix = "/devices",
    tags = ["items"], # this will just assign the tag visually when looking at API docs
    responses = {404: {"description": "Not found"}} # response codes and respective descriptions
    # response errors must be raised with HTTPException
)
    
class DeviceDataSchema(BaseModel):
    """Base schema for holding device data, can be used for presets or the data uplink to the frontend

    Args:
        temperature, float: temperature that the plant likes
        humidity, float: humidity percentage that the plant likes
        moisture, float: moisture level in the soil that the plant likes
    """
    temperature: float
    humidity: float
    moisture: float
    
class DeviceRegistrationPreset(DeviceDataSchema):
    """Preset schema which represents preset data while communicating with the device from the server

    Args:
        hours_daylight, float: how many hours of daylight does the plant need?
        preset_id, int: id of the preset in question for database interactions
    """
    hours_daylight: float
    preset_id: int
    

class DeviceConfigurationSchema(BaseModel):
    """schema representing data necessary to submit a configuration request to the device from the server

    Args:
        project_id, Optional[str]: id of the project associated with the device
        device_name, Optional[str]: the name of the device to register. Might be the IP address
        device_id, Optional[int]: the id of the device that the device stores in flash. If this isnt included, the device was either reset, or is connecting for the first time
    """
    device_name: Optional[str]
    device_id: Optional[int]
    project_id: Optional[str]
    
class DeviceRegistrationSchema(DeviceConfigurationSchema):
    """schema representing the request from the IoT device to the server to request registration

    Args:
        preset, Optional[DeviceRegistrationPreset]: schema containing all information about the preset the device believes it is assigned
    """
    preset: Optional[DeviceRegistrationPreset]
    
class DeviceRegistrationResponse(DeviceRegistrationSchema):
    """Schema holding the response from the server to the IoT device when the IoT device requests association

    Args:
        inherits all fields from DeviceRegistrationSchema
        reference_time, float: current unix timestamp for the device to sync to
    """
    reference_time: float
    
class ClientDeviceDataSchema(DeviceDataSchema):
    """data that is delivered to the client frontend periodically for the device webpage. 

    Args:
        uptime, str: the time the device has been active since registration
        light, str: light level in lumens, the user will like something like this
    """
    uptime: str
    light: str
    
class ClientDeviceConfigSchema(BaseModel):
    """This is the schema the client side frontend sends data to the server in when it wants to configure a device

    Args:
        device_name, Optional[str]: name of the device in question
        device_id, Optional[int]: id of the device in question
        device_status, Literal["ACTIVE", "IDLE", "DISCONNECTED"]: enum representing the state of the device. Defaults to DISCONNECTED
        preset_name, Optional[str]: name of preset_name associated with device
        project_name, Optional[str]: name of project associated with device
    """
    device_name: Optional[str]
    device_id: Optional[int]
    device_status: Literal["ACTIVE", "IDLE", "DISCONNECTED"] = "DISCONNECTED"
    preset_name: Optional[str]
    project_name: Optional[str]  

class LogSchema(BaseModel):
    """schema for holding log data. This should probably be in projects......"""
    log_level: int
    log_timestamp: datetime
    log_content: str
    
    
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
        
    
@router.get("/devices/{device_name}")
async def serve_webpage(request: Request, device_name: str) -> HTMLResponse:
    """serve the webpage for the specified device to the frontend

    Args:
        request (Request): raw request necessary for templating
        device_name (str): device name to load the webpage for

    Returns:
        Template: template of webpage to return
    """
     #integrate device name with this # return 404 if the device name doesnt exsit
    return router.template_paths.TemplateResponse("device.html", {"request":request,"device_name":device_name}) # assign the metavariable the device name so frontend can get the rest

@router.get("/devices/{device_name}/assign_project")
async def assign_project(request: Request, device_name: str) -> HTMLResponse:
    """assign a project to a device

    Args:
        request (Request): raw request necessary for template serving
        device_name (str): name of the device to assign a project

    Returns:
        Template: webpage template being returned
    """
    return router.template_paths.TemplateResponse("assign_project.html", {"request":request,"device_name":device_name})

@router.get("/devices/{device_name}/assign_preset")
async def assign_preset(request: Request, device_name: str) -> HTMLResponse:
    """Opens the preset assignment webpage to assign a preset to a device

    Args:
        request (Request): raw request necessary for template serving
        device_name (str): name of the device to assign a preset

    Returns:
        Template: webpage template being returned
    """
    return router.template_paths.TemplateResponse("assign_preset.html", {"request":request,"device_name":device_name})

@router.get("/devices/list_devices")
async def list_devices() -> list[tuple | None]:
    """get a list of all devices registered to the server, whether they are idle or active

    Returns:
        list[tuple | None]: list of devices retrieved from the database
    """
    devices = router.database_connector.execute('getDevices')
    return devices

@router.get("/devices/{device_name}/device_info")
async def get_device(device_name: str) -> ClientDeviceConfigSchema:
    """Get the data for a specific device stored in the server

    Args:
        device_name (str): name of the device to get data for

    Returns:
        ClientDeviceConfigSchema: represents critical data for the device to send back to the client
    """
    device_id: int = router.database_connector.execute('getDeviceID', device_name)
    device_information: tuple = router.database_connector.execute('getDevice', device_id)
    
    response: ClientDeviceConfigSchema = ClientDeviceConfigSchema()(
        device_name = device_information[1],
        device_ip = device_information[2],
        project_name = router.database_connector.execute("getProjectName", device_information[5]),
        preset_name = router.database_connector.execute("getPresetName", device_information[4]),
        registration_time=device_information[6],
        device_status=device_information[7]
    )
    return response

@router.get("/devices/{device_name}/recent_data") 
async def get_most_recent_data(device_name: str) -> ClientDeviceDataSchema:
    """get the most recent data entry for the specified device in the database

    Args:
        device_name (str): name of the device to get the data for

    Returns:
        ClientDataSchema: schema to store the data
    """
    device_id: int = router.database_connector.execute('getDeviceID', device_name)
    device_data: tuple = router.database_connector.execute('getLatestDeviceData', device_id)
    
    device_start_time: datetime = router.database_connector.execute("getDevice", device_id)[0][6]
    data = ClientDeviceDataSchema(
        temperature=device_data[3], 
        humidity=device_data[4],
        moisture=device_data[5],
        light=device_data[6],
        uptime=device_start_time)
    
    return data

@router.get("/devices/{device_name}/status")
async def get_device_status(device_name: str) -> JSONResponse:
    """get the status of the device with device_name in question

    Args:
        device_name (str): name of the device to retrieve status of

    Returns:
        JSONResponse: response contains enum between ACTIVE, IDLE and DISCONNETED to represent device status
    """
    device_id: int = router.database_connector.execute('getDeviceID', device_name)
    device_status: Literal["ACTIVE", "IDLE", "DISCONNECTED"] = router.database_connector.execute("getDeviceStatus", device_id)
    
    return JSONResponse(device_status, 200)
    
@router.get("/scan")
async def serve_scan_page(request: Request, source_project: Optional[str]=None) -> HTMLResponse:
    """_summary_

    Args:
        request (Request): raw request body, needed to serve the template
        source_project (_type_): if the webpage serve is a redirect from a project then the request should include the name of that project so registered devices 

    Returns:
        Template: webpage template
    """
    return router.template_paths.TemplateResponse("scan.html", {"request":request,"source_project":source_project}) # assign the metavariable the device name so frontend can get the rest

@router.get("/idle/getdevices")
async def get_idle() ->JSONResponse:
    """get a list of idle devices (devices which are attached to neither a project nor a preset)

    Returns:
        JSONResponse: return respective device data in json format for device in the sleeping list
    """
    idles = router.device_manager.sleeping_device_list
    idles = {id:device.json() for id, device in idles.items()}
    
    return JSONResponse(idles, 200)

@router.get("/scan/getscans")
async def scan_devices() -> JSONResponse:
    """Get a list of devices in the scanning list of the device manager

    Returns:
        JSONResponse: returns device data json for respective entry in scan list
    """
    scans = router.device_manager.get_scans()
    scans = {ip:device.json() for ip, device in scans.items()}
    
    return JSONResponse(scans, 200)

@router.put("/devices/{device_name}/update")
async def configure_device_identifiers(device_name: str, configuration_info: ClientDeviceConfigSchema) -> JSONResponse:
    """The user will often want to re-configure their device settings to fulfill new tasks. This route will receive configuration information
    from the user on the frontend, write it to the database, and forward it to the device

    Args:
        device_name (str): The device name to configure
        configuration_info (DeviceConfigSchema): all of the configuration details to be written to the DB and device

    Raises:
        e: _description_ will fix later

    Returns:
        JSONResponse(202) in the event the configuration is successful
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_ip = router.database_connector.execute('getDevice', device_id)[2]
    
    if configuration_info.project_name:
        project_id = router.database_connector.execute("getProjectID", configuration_info.project_name)
    
    router.database_connector.execute("configureDevice", device_id, device_name=configuration_info.device_name, project_id=project_id) # let it go anyway, the device will sync when reconnecting
    
    try:
        body: DeviceConfigurationSchema = DeviceConfigurationSchema(
            device_name=configuration_info.device_name,
            device_id=device_id,
            project_id=router.database_connector.execute("getProjectName", configuration_info.project_name),
        )
        response: Response = await change_identifier.call(device_ip, data=body)
    except Exception as e: # more graceful handling of the timeout, in case device disconnects it shouldnt actually prevent the user from configuring it, the device just syncs later
        router.database_connector.execute("configureDevice", device_id, device_status="IDLE")
        raise e
    
    return JSONResponse("Device configuration successful", 202)

@router.put("/devices/{device_name}/update_preset")
async def configure_device_preset(device_name: str, preset_name: str) -> JSONResponse:
    """receive request from the frontend client to change the device preset

    Args:
        device_name (str): name of the device to configure
        preset_name (str): name of the preset to switch to

    Returns:
        JSONResponse: respond with 200 if successful 
    """
    device_id: int = router.database_connector.execute('getDeviceID', device_name)
    device_ip: str = router.database_connector.execute('getDevice', device_id)[2]
    
    preset_id: int = router.database_connector.execute('getPresetID', preset_name)
    preset_information: tuple = router.database_connector.execute("getPreset", preset_id)
    
    router.database_connector.execute("configureDevice", device_id, preset_id=preset_id)
    
    try:
        body: DeviceRegistrationPreset(
            temperature = preset_information[2],
            humidity=preset_information[3],
            moisture=preset_information[4],
            hours_daylight=preset_information[5],
            preset_id=preset_id
        )
        response: Response = await change_preset.call(device_ip, data=body)
    except Exception as e: # more graceful handling of the timeout, in case device disconnects it shouldnt actually prevent the user from configuring it, the device just syncs later
        router.database_connector.execute("configureDevice", device_id, device_status="IDLE")
        raise e
    
    return JSONResponse("Device preset set successfully", 200)

@router.post("/scan/register_device")
async def register_device(device_ip: str, project_name: Optional[str] = None) -> JSONResponse:
    """1st step in device registration process. API call from the user on the web portal to register a device listen on the scans page. This function then tells the device manager to
    send an acknowledgement to the requested device to proceed with the registration process

    Args:
        device_ip (str): the ip of the device in question, used for sending the acknowledgement to the device
        project_name (Optional[str], optional): In the event that the user is registering the device directly into a project, this should be called to configure that project. Defaults to None

    Raises:
        HTTPException: in case the requested device never proceeds to call the "confirm_registration" route, the request will time out and raise 503
        
    Returns:
        JSONResponse(200): if the registration is successful. SHould trigger success message or some behavior on frontend
    """
    router.device_manager.send_registration(device_ip=device_ip)
    
    start_time: int = time.time()

    condition = router.device_manager.registration_queue['device_ip']
    async with condition:
        await condition.wait_for(lambda: time.time() - start_time > 30)
        await asyncio.sleep(10)
        if device_ip in router.device_manager.registration_queue:
            router.device_manager.registration_queue.pop(device_ip)
            raise HTTPException(503, detail=f"Registration of Device {device_ip} failed!")
        
    device_id: int = router.database_connector.execute("getDeviceID", device_ip) # the device name will be the same as its IP for now
    
    if project_name: # if the device is being registered as a project device. This might work, except now we must take into consideration idle status devices. Idle devices could just be included in "scan list" that should be in a separate function though
        project_id: int = router.database_connector.execute("getProjectID", project_name)
        
        device_information: tuple = router.database_connector.execute("getDevice", device_id)
        
        body: DeviceConfigurationSchema = DeviceConfigurationSchema(
            device_id=device_id,
            device_name=device_information[1],
            project_id=project_id
        )
        
        response: Response = await change_identifier.call(device_ip, data=body)
        
        router.database_connector.execute("configureDevice", device_id, project_id=project_id)
        
    router.device_manager.active_device_list[device_ip].set_id(device_id)
    
    return JSONResponse("Device registered successfully", 200)

@router.post("/confirm")
async def confirm_registration(stored_device_info: DeviceRegistrationSchema, request: Request) -> DeviceRegistrationResponse:
    """Step 3 of the device registration process (see register_device and send_registration). Waits for a request from an IoT device containing
    device information. This information should be from the flash memory unit on the IoT device. If the device knows its own ID, it must be in the database
    and so there is no need to create a new entry in the database. If however, it is the devices first time connecting, it needs a new database entry.
    
    If the device was disconnected, and an unregister request was sent before it reconnected, this method will detect that and automatically unregister the device

    Args:
        stored_device_info (DeviceRegistrationSchema): contains the data stored on the flash memory of the device (if any)
        request (Request): the raw request received by the api, used to extract the ip of the device in question

    Raises:
        HTTPException: in the event that the device is not in the router.device_manager registration queue, the device should not be allowed to connect

    Returns:
        DeviceRegistrationResponse: this is the http response to the device containing server side information on that device. The device should
        always sync to server side data to maintain synchronization. This also contains reference time for the device to align to.
    """
    source_ip: str = request.client.host
    
    if source_ip not in router.device_manager.registration_queue:
        raise HTTPException(404, "Device not found")
    
    reported_device_id: str = stored_device_info.device_id
    database_device_data: tuple = router.database_connector.execute("getDevice", reported_device_id)
    
    # check to see if the device knows its device id and it doesnt have data in the database. This means the device was unregistered while the device was disconnected
    if reported_device_id and not database_device_data: # this is an edge case. If it becomes necessary ill implement it later
        true_device_id: str = reported_device_id
        
        await device_unregister.call(source_ip)
        # do something with reregistration here, I dont frickin know
        return
    
    # check to see if the device knows its device id, and it does have an entry in the database
    elif reported_device_id and database_device_data:
        true_device_id: str = reported_device_id
        
        server_preset_response: Optional[DeviceRegistrationPreset] = None
        
        database_preset_id: int = database_device_data[4]
        database_preset_information: tuple = router.database_connector.execute("getPreset", database_preset_id)

        if database_preset_information:
            server_preset_response = DeviceRegistrationPreset(preset_id=database_preset_id,
                temperature=database_preset_information[2],
                humidity=database_preset_information[3],
                moisture=database_preset_information[4],
                hours_daylight=database_preset_information[5])
            
        database_project_id: int = database_device_data[5]
        
        response: DeviceRegistrationResponse = DeviceRegistrationResponse(device_id = true_device_id,
            device_name = database_device_data[1],
            project_id = database_project_id,
            preset = server_preset_response,
            reference_time = time.time())
    
    # if the device has never been configured before (if no reported ID)
    else:
        router.database_connector.execute("registerDevice", source_ip, None, None, "IDLE") # device name initially set to the ip
        true_device_id: int = router.database_connector.execute("getDeviceID", source_ip)
        
        response = DeviceRegistrationResponse(device_id = true_device_id,
            reference_time=time.time())
    
    # check for expidited flag here
    device_info: Device = router.device_manager.registration_queue.pop(source_ip)
    
    if not device_info.expidited: # if the process is expidited as specified by the device, that means the user never made an explicit request. Thus this step can be skipped
        router.device_manager.registration_queue[source_ip].notify_all()
        
    router.device_manager.active_device_list[true_device_id] = device_info  
    
    return response 
    
@router.delete("/devices/{device_name}/unregister")
async def unregister_device(device_name) -> Literal['successfully unregistered device']:
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_ip = router.device_manager.active_device_list[device_id].ip
    
    try:
        await device_unregister.call(device_ip)
    
        router.database_connector.execute("unregisterDevice", device_id)
        router.device_manager.active_device_list.pop(device_ip)
    except Exception as e:
        raise e
    return "successfully unregistered device"

@router.get("/devices/{device_name}/logs")
async def get_logs(device_name) -> list[LogSchema]:
    pass

@router.get("/devices/{device_name}/downloadLogs")
async def get_logs(device_name) -> FileResponse:
    return FileResponse(path=log_path, media_type="application/octet-stream", filename=f"{device_name}.log")