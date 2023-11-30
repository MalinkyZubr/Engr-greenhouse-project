from fastapi import APIRouter
from fastapi.requests import Request
from fastapi.exceptions import HTTPException
from pydantic import BaseModel, Field
from typing import Optional
from datetime import datetime

from controller.DBIntRouter import APIDRouter
from controller.schemas.server_device_schemas import BaseSchema
from controller.device_management import unregister_device_request, configure_device_request

router = APIDRouter(
    prefix="/interface"
)
"""
this file interacts directly with the devices on the entwork
"""


class DataSchema(BaseModel):
    project_name: str # the device should know its own project ID
    device_name: str
    temperature: float
    humidity: float
    moisture: float
    light_exposure: float
    ir_exposure: Optional[float]
    ph_level: Optional[float]
    
    
class LogSchema(BaseModel):
    log_level: int
    log_content: str
    

@router.middleware("http")
async def check_aging(request: Request, next_call): # auto updates the device status
    source_ip = request.client.host
    device_mac = None # <- SOMETHING SHOULD GO HERE! VERY IMPORTANT
    device_info = router.device_manager.active_device_list[source_ip]
    device_id = router.database_connector.execute("getDeviceIDByIP", source_ip)
    
    if not device_id: # this is to handle failed unregisters. If the device is supposed to be gone, get rid of it
        await unregister_device_request(device_info[2])
        raise HTTPException(428)
    
    device_status = router.database_connector.execute("getDeviceStatus", device_id)
    
    if not device_status: # if the device was down, sync its data back to the server in case it was out of sync
        await configure_device_request(source_ip, device_name=device_info[1], 
                                 device_project=router.database_connector.execute("getProjectName", device_info[5]),
                                 device_preset=router.database_connector.execute("getPresetName", device_info[4])) # resync to the server data stored
        router.database_connector.execute("configureDevice", device_id, device_status=True)
        
    device_info.update_time() # make sure the timeout doesnt go off...
    
    response = await next_call(request)
    
    return response  
    
@router.post("/data/{device_name}")
async def post_data(device_name: str, data_info: DataSchema):
    """
    post data from a device to the database
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)
    router.database_connector.execute('insertData',
        device_id,
        data_info.project_id,
        data_info.temperature,
        data_info.humidity,
        data_info.moisture,
        data_info.light_exposure,
        data_info.ir_exposure,
        data_info.ph_level
    )
    return "data inserted successfully"

@router.post("/logs/{device_name}")
async def post_logs(device_name: str, log_info: LogSchema):
    pass