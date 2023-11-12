from fastapi import APIRouter
from fastapi.requests import Request
from pydantic import BaseModel, Field
from datetime import datetime

from controller.DBIntRouter import APIDRouter
from controller.schemas.server_device_schemas import BaseSchema

router = APIDRouter()
"""
this file interacts directly with the devices on the entwork
"""


class DataSchema(BaseModel):
    project_id: int # the device should know its own project ID
    temperature: float
    humidity: float
    moisture: float
    light_exposure: float
    ir_exposure: float
    ph_level: float
    
    
class LogSchema(BaseModel):
    log_level: int
    log_content: str
    

@router.middleware("http")
async def check_aging(request: Request, next_call): # auto updates the device status
    source_ip = request.client.host
    device_info = router.device_manager.active_device_list[source_ip]
    device_id = router.database_connector.execute("getDeviceIDByIP", source_ip)
    
    device_status = router.database_connector.execute("getDeviceStatus", device_id)
    
    if not device_status:
        router.database_connector.execute("configureDevice", device_id, device_status=True)
    device_info.update_time()
    
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