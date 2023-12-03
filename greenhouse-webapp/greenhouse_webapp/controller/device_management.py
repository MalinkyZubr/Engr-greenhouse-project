from fastapi import APIRouter, Depends, Request
from fastapi.responses import HTMLResponse
from pydantic import BaseModel
from typing import Optional, Literal
from starlette.responses import FileResponse
from fastapi.responses import JSONResponse
from fastapi.exceptions import HTTPException
import time
import asyncio
from datetime import datetime
import requests
from controller.frontend_paths import SCAN, ASSIGN_PRES, ASSIGN_PROJ
from controller.DBIntRouter import APIDRouter
from controller.schemas.server_device_schemas import BaseSchema


router = APIDRouter(
    prefix = "/devices",
    tags = ["items"], # this will just assign the tag visually when looking at API docs
    responses = {404: {"description": "Not found"}} # response codes and respective descriptions
    # response errors must be raised with HTTPException
)

class ClientDeviceRegSchema(BaseModel):
    device_name: Optional[str]
    device_mac: str
    
    
class DeviceConfigSchema(BaseModel):
    device_name: Optional[str]
    device_id: Optional[str]
    device_status: Optional[bool]
    preset_name: Optional[str]
    project_name: Optional[str]
    
class RegistrationPreset(BaseModel):
    preset_name: str
    temperature: float
    humidity: float
    moisture: float
    hours_daylight: float
    
class DeviceRegistrationSchema(BaseModel):
    device_name: Optional[str]
    device_id: Optional[int]
    project_name: Optional[str]
    preset: Optional[RegistrationPreset]
    
class DeviceRegistrationResponse(DeviceRegistrationSchema):
    reference_time: float
    
class DeviceInformationSchema(BaseModel):
    device_name: str
    device_ip: str
    device_mac: str
    preset_name: Optional[str]
    project_name: Optional[str]
    device_status: bool
    registration_time: datetime    

class LogSchema(BaseModel):
    log_level: int
    log_timestamp: datetime
    log_content: str
    
    
class ClientDataSchema(BaseModel):
    uptime: str
    temperature: float
    humidity: float
    moisture: float
    ph: float
    light: float
    

class DevicePreset(BaseModel):
    temperature: float
    moisture: float
    humidity: float
    light: float
    
    
async def unregister_device_request(device_ip): # should delete the proejct data from the device, as well as server IP. 
    for _ in range(3): # must make sure the device actually unregisters
        message = BaseSchema("unregister")
        try:
            response = requests.put(f"{device_ip}/unregister", data=message.model_dump_json(), timeout=3) # need to implement https and encryption for server -> device communication
        except Exception as e:
            await asyncio.sleep(3)
            continue
        response_code = response.status_code
        if response_code == 200:
            break
    else:
        raise HTTPException(500, detail="unregistering failed")
    
async def configure_device_request(device_ip, device_name=None, device_id=None, device_status = None, device_project=None, device_preset=None):
    for _ in range(3):
        message = BaseSchema("configure", request_content = DeviceConfigSchema(
            device_name = device_name,
            device_id = device_id,
            device_status = device_status,
            preset_name = device_preset,
            project_name= device_project
        ))
        try:
            response = requests.put(f"{device_ip}/configure", data=message.model_dump_json(), timeout=3) # need to implement https and encryption for server -> device communication
        except Exception as e:
            await asyncio.sleep(10)
            continue
        response_code = response.status_code
        if response_code == 200:
            break
    else:
        raise HTTPException(500, detail="configuration failed") # MAKE SURE IT UNREGISTERES NO MATTER WHAT
    
async def configure_device_preset(device_ip, preset_data: DevicePreset):
    for _ in range(3):
        try:
            response = requests.put(f"{device_ip}/preset_config", data=preset_data.model_dump_json(), timeout=3)
        except Exception as e:
            await asyncio.sleep(10)
            continue
        response_code = response.status_code
        if response_code == 200:
            break
    else:
        raise HTTPException(500, detail="configuration failed") # MAKE SURE IT UNREGISTERES NO MATTER WHAT
    
@router.get("/devices/{device_name}")
async def serve_webpage(request: Request, device_name):
    """
    serve the device management webpage 
    """ #integrate device name with this # return 404 if the device name doesnt exsit
    return router.template_paths.TemplateResponse("device.html", {"request":request,"device_name":device_name}) # assign the metavariable the device name so frontend can get the rest

@router.get("/devices/{device_name}/assign_project")
async def assign_project(request: Request, device_name):
    """
    Serve the project assignment webpage
    """
    return router.template_paths.TemplateResponse("assign_project.html", {"request":request,"device_name":device_name})

@router.get("/devices/{device_name}/assign_preset")
async def assign_preset(request: Request, device_name):
    """
    Serve the project assignment webpage
    """
    return router.template_paths.TemplateResponse("assign_preset.html", {"request":request,"device_name":device_name})

@router.get("/list_devices")
async def list_devices():
    """
    List registered devices, whether they are connected or disconnected
    """
    devices = router.database_connector.execute('getDevices')
    return devices

@router.get("/devices/{device_name}/device_info")
async def get_device(device_name: str) -> DeviceInformationSchema:
    """
    get a specific device
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_information = router.database_connector.execute('getDevice', device_id)
    
    response = DeviceInformationSchema(
        device_name = device_information[1],
        device_ip = device_information[2],
        device_mac = device_information[3],
        project_name = router.database_connector.execute("getProjectName", device_information[5]),
        preset_name = router.database_connector.execute("getPresetName", device_information[4]),
        registration_time=device_information[6],
        device_status=device_information[7]
    )
    return response

@router.get("/devices/{device_name}/recent_data") 
async def get_most_recent_data(device_name: str) -> ClientDataSchema:
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_data = router.database_connector.execute('getLatestDeviceData', device_id)[0]
    
    device_start_time = router.database_connector.execute("getDevice", device_id)[0][6]
    data = ClientDataSchema(
        temperature=device_data[3], 
        humidity=device_data[4],
        moisture=device_data[5],
        light=device_data[6],
        ph=device_data[8],
        uptime=device_start_time)
    
    return data

@router.get("/devices/{device_name}/status")
async def get_device_status(device_name: str):
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_status = router.database_connector.execute("getDeviceStatus", device_id)
    
    return device_status
    
@router.get("/scan")
async def serve_scan_page(request: Request, source_project):
    """
    Serve the scanning page
    """
    return router.template_paths.TemplateResponse("scan.html", {"request":request,"source_project":source_project}) # assign the metavariable the device name so frontend can get the rest

@router.get("/scan/getscans")
async def scan_devices():
    """
    Scan for available devices
    """
    scans = router.device_manager.get_scans()
    scans = {ip:scan_object.json() for ip, scan_object in scans.items()}
    return JSONResponse(content=scans)

@router.put("/devices/{device_name}/update")
async def configure_device(device_name, configuration_info: DeviceConfigSchema):
    # MAKE SURE TO ACTUALLY SEND THE REQUEST TO THE DEVICE HERE!!!
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_ip = router.database_connector.execute('getDevice', device_id)[0][2]
    
    if configuration_info.project_name:
        project_id = router.database_connector.execute("getProjectID", configuration_info.project_name)
    if configuration_info.preset_name:
        preset_id = router.database_connector.execute("getPresetID", configuration_info.preset_name)
    
    router.database_connector.execute("configureDevice", device_id, device_name=configuration_info.device_name, preset_id=preset_id, project_id=project_id) # let it go anyway, the device will sync when reconnecting
    
    try:
        await configure_device_request(
            device_ip, 
            device_name = configuration_info.device_name, 
            device_status = configuration_info.device_status,
            device_project = configuration_info.project_name, 
            device_preset = configuration_info.preset_name
        )
    except Exception as e:
        router.database_connector.execute("configureDevice", device_id, device_status=False)
        raise e
    
    return "Device update successful"

@router.post("/scan/register_device")
async def register_device(project_name, device_info: ClientDeviceRegSchema):
    """
    register a device to the server
    this should send a UDP resonse to the client telling it to stop sending pings.
    should also remove device from the scan list
    """
    router.device_manager.send_registration(device_ip=device_info.device_ip)
    
    start_time = time.time()

    condition = router.device_manager.registration_queue['device_ip']
    async with condition:
        await condition.wait_for(lambda: time.time() - start_time > 30)
        await asyncio.sleep(1)
        if device_info.device_ip in router.device_manager.registration_queue:
            router.device_manager.registration_queue.pop(device_info.device_ip)
            raise HTTPException(503, detail=f"Registration of Device {device_info.device_ip} failed!")
        
    device_id = router.database_connector.execute("getDeviceIDByIP", device_info.device_ip)
    if project_name:
        project_id = router.database_connector.execute("getProjectID", project_name)
        
        await configure_device_request(device_info.device_ip, device_project = project_name)
        router.database_connector.execute("configureDevice", device_id, project_id=project_id)
        
    router.device_manager.active_device_list[device_info.device_ip].set_id(device_id)
    
    return "device registered successfully"

@router.post("/confirm")
async def confirm_registration(stored_device_info: DeviceRegistrationSchema, request: Request) -> DeviceRegistrationResponse: # figure this shit out, I doubt it works
    """
    trigger event to finish device registration. This is confirmation via https. THIS IS A REQUEST FROM THE DEVICE IN QUESTION. AKA the most disgusting function
    """
    source_ip = request.client.host
    
    if source_ip not in router.device_manager.registration_queue:
        raise Exception("Device not found")
    
    reported_device_id = stored_device_info.device_id
    database_device_data = router.database_connector.execute("getDevice", reported_device_id)
    
    # check to see if the device knows its device id and it doesnt have data in the database
    if reported_device_id and not database_device_data:
        true_device_id = reported_device_id
        pass
    # check to see if the device knows its device id, and it does have an entry in the database
    elif reported_device_id and database_device_data:
        true_device_id = reported_device_id
        pass
    # if the device has never been configured before
    else:
        # get the device id by ip here after creating an entry
        true_device_id = None
        pass     
    
    # reference time should also be returned from here
    
    # check for expidited flag here
    router.device_manager.registration_queue[source_ip].notify_all()
    scan_info = router.device_manager.registration_queue.pop(source_ip)

    router.device_manager.active_device_list[true_device_id] = scan_info   
    
@router.delete("/devices/{device_name}/unregister")
async def unregister_device(device_name) -> Literal['successfully unregistered device']:
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_ip = router.device_manager.active_device_list[device_id].ip
    
    try:
        await unregister_device_request(device_ip)
    
        router.database_connector.execute("unregisterDevice", device_id)
        router.device_manager.active_device_list.pop(device_ip)
    except Exception as e:
        pass
    return "successfully unregistered device"

@router.get("/devices/{device_name}/logs")
async def get_logs(device_name) -> list[LogSchema]:
    pass

@router.get("/devices/{device_name}/downloadLogs")
async def get_logs(device_name):
    return FileResponse(path=log_path, media_type="application/octet-stream", filename=f"{device_name}.log")