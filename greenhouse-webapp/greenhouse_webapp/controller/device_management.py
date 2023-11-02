from fastapi import APIRouter, Depends
from pydantic import BaseModel
from typing import Optional 
from starlette.responses import FileResponse
import time

from frontend_paths import DEVICE, SCAN
from DBIntRouter import APIDRouter

router = APIDRouter(
    prefix = "/devices",
    tags = ["items"], # this will just assign the tag visually when looking at API docs
    responses = {404: {"description": "Not found"}} # response codes and respective descriptions
    # response errors must be raised with HTTPException
)


class DeviceRegistrationSchema(BaseModel):
    device_name: str
    device_mac: str
    preset_id: Optional[str]
    project_id: Optional[str]
    device_status: bool

@router.get("{device_name}")
async def serve_webpage(device_name):
    """
    serve the device management webpage 
    """ #integrate device name with this
    return FileResponse(DEVICE)

@router.get("/list_devices")
async def list_devices():
    """
    List registered devices, whether they are connected or disconnected
    """
    devices = router.database_connector.execute('getDevices')
    return devices

@router.get("/{device_name}/device_info")
async def get_device(device_name: str):
    """
    get a specific device
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_information = router.database_connector.execute('getDevice', device_id)
    return device_information
    
@router.get("/scan")
async def serve_scan_page():
    """
    Serve the scanning page
    """
    return FileResponse(SCAN)

@router.get("/scan/getscans")
async def scan_devices():
    """
    Scan for available devices
    """
    return router.device_manager.get_scans()

@router.post("/scan/{device_ip}")
async def register_device(device_ip: str, device_info: DeviceRegistrationSchema):
    """
    register a device to the server
    this should send a UDP resonse to the client telling it to stop sending pings.
    should also remove device from the scan list
    """
    router.device_manager.send_registration(device_ip=device_ip)
    
    start_time = time.time()

    condition = router.device_manager.registration_queue['device_ip']
    async with condition:
        await condition.wait_for(lambda: start_time - time.time() > 30)
        if device_ip in router.device_manager.registration_queue:
            raise ConnectionError(f"Registration of Device {device_ip} failed!")
        
    router.database_connector.execute("registerDevice",
        device_info.device_name,
        device_ip,
        device_info.device_mac,
        device_info.preset_id,
        device_info.project_id,
        device_info.device_status)
    
    return "device registered successfully"

@router.post("/confirm/{device_ip}")
async def confirm_registration(device_ip):
    """
    trigger event to finish device registration. This is confirmation via https
    """
    router.device_manager.registration_queue[device_ip].notify_all()