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
    reported_device_id = request.cookies["device_id"]
    source_ip = request.client.host
        
    active_list_device_entry = router.device_manager.active_device_list[reported_device_id]
    
    stored_device_data = router.database_connector.execute("getDevice", reported_device_id)
    
    if not stored_device_data or not reported_device_id in router.device_manager.active_device_list: # this is to handle failed unregisters. If the device is supposed to be gone, get rid of it
        await unregister_device_request(source_ip)
        raise HTTPException(428)
    
    true_device_id = reported_device_id # if the above condition didnt get triggered, we know this is a valid ID
    
    device_status = router.database_connector.execute("getDeviceStatus", true_device_id)
    
    if not device_status: # if the device was down, sync its data back to the server in case it was out of sync
        await configure_device_request(source_ip, device_name=stored_device_data[1], 
                                 device_project=router.database_connector.execute("getProjectName", stored_device_data[5]),
                                 device_preset=router.database_connector.execute("getPresetName", stored_device_data[4])) # resync to the server data stored
        router.database_connector.execute("configureDevice", true_device_id, device_status=True)
        
    active_list_device_entry.update_time() # make sure the timeout doesnt go off...
    
    response = await next_call(request)
    
    return response  
    
@router.post("/data/{device_name}")
async def post_data(device_name: str, data_info: DataSchema):
    """
    post data from a device to the database
    """
    device_id = router.database_connector.execute('getDeviceID', device_name)
    project_id = router.database_connector.execute('getProjectID', data_info.project_name)
    router.database_connector.execute('insertData',
        device_id,
        project_id,
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