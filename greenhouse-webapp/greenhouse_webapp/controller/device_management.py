from fastapi import APIRouter, Depends, Request
from fastapi.responses import HTMLResponse
from pydantic import BaseModel
from typing import Optional 
from starlette.responses import FileResponse
from fastapi.responses import JSONResponse
from fastapi.exceptions import HTTPException
import time
import asyncio

from controller.frontend_paths import SCAN, ASSIGN_PRES, ASSIGN_PROJ
from controller.DBIntRouter import APIDRouter


router = APIDRouter(
    prefix = "/devices",
    tags = ["items"], # this will just assign the tag visually when looking at API docs
    responses = {404: {"description": "Not found"}} # response codes and respective descriptions
    # response errors must be raised with HTTPException
)

class ClientDeviceRegSchema(BaseModel):
    device_name: Optional[str]
    device_ip: str
    device_mac: str

class DeviceRegistrationSchema(BaseModel):
    device_name: Optional[str]
    device_id: Optional[int]
    device_ip: str
    device_mac: str
    preset_id: Optional[str]
    project_id: Optional[str]
    device_status: bool


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
async def get_device(device_name: str):
    """
    get a specific device
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)[0][0]
    device_information = router.database_connector.execute('getDevice', device_id)
    return device_information
    
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
        await condition.wait_for(lambda: start_time - time.time() > 30)
        await asyncio.sleep(1)
        if device_info.device_ip in router.device_manager.registration_queue:
            router.device_manager.registration_queue.pop(device_info.device_ip)
            raise HTTPException(503, detail=f"Registration of Device {device_info.device_ip} failed!")
    if project_name:
        project_id = router.database_connector.execute("getProjectID", project_name)
        device_id = router.database_connector.execute("getDeviceIDByIP", device_info.device_ip)
        router.database_connector.execute("configureDevice", device_id, project_id=project_id)
    return "device registered successfully"

@router.post("/confirm")
async def confirm_registration(device_info: DeviceRegistrationSchema):
    """
    trigger event to finish device registration. This is confirmation via https. THIS IS A REQUEST FROM THE DEVICE IN QUESTION
    """
    if device_info.device_ip in router.device_manager.registration_queue:
        existing_device_info = router.database_connector.execute("getDevice", device_info.device_id)
        if device_info.device_id and not existing_device_info:
            router.database_connector.execute("reregisterDevice",
                                            device_info.device_name,
                                            device_info.device_id,
                                            device_info.device_ip,
                                            device_info.device_mac,
                                            device_info.preset_id,
                                            device_info.project_id,
                                            False)
        else:
            router.database_connector.execute("registerDevice", None, 
                                                device_info.device_ip, 
                                                device_info.device_mac,
                                                None, None, False)
        
        router.device_manager.registration_queue[device_info.device_ip].notify_all()
        router.device_manager.registration_queue.pop(device_info.device_ip)
        device_id = router.database_connector.execute("getDeviceIDByIP", device_info.device_ip)
        router.database_connector.execute("configureDevice", device_id, device_name=device_info.device_mac, preset_id=device_info.preset_id, project_id=device_info.project_id)