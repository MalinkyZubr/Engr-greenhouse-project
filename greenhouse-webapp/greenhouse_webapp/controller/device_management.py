from fastapi import APIRouter, Depends
from pydantic import BaseModel
from typing import Optional 

from DBIntRouter import APIDRouter

router = APIDRouter(
    prefix = "/devices",
    tags = ["items"], # this will just assign the tag visually when looking at API docs
    responses = {404: {"description": "Not found"}} # response codes and respective descriptions
    # response errors must be raised with HTTPException
)


class DeviceRegistrationSchema(BaseModel):
    device_ip: str
    device_mac: str
    preset_id: Optional[str]
    project_id: Optional[str]
    device_status: bool


@router.get("/")
async def list_devices():
    """
    List registered devices, whether they are connected or disconnected
    """
    devices = router.database_connector.execute('getDevices')
    return devices

@router.get("/{device_name}")
async def get_device(device_name: str):
    """
    get a specific device
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)
    device_information = router.database_connector.execute('getDevice', device_id)
    return device_information
    

@router.get("/scan")
async def scan_devices():
    """
    Scan for available devices
    """
    return router.device_manager.get_scans()

@router.post("/scan/{device_name}")
async def register_device(device_name, device_info: DeviceRegistrationSchema):
    """
    register a device to the server
    this should send a UDP resonse to the client telling it to stop sending pings.
    should also remove device from the scan list
    """
    router.database_connector.execute("registerDevice", device_name,
        device_info.device_ip,
        device_info.device_mac,
        device_info.preset_id,
        device_info.project_id,
        device_info.device_status)
    return "device registered successfully"