from fastapi import APIRouter
from pydantic import BaseModel, Field

from controller.DBIntRouter import APIDRouter

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