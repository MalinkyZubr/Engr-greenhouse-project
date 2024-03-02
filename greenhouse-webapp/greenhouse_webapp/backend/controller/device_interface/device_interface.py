from fastapi import APIRouter
from fastapi.requests import Request
from fastapi.exceptions import HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field
from typing import Optional, Literal
from datetime import datetime

from greenhouse_webapp.controller.APIDRouter import APIDRouter
from controller.schemas.server_device_schemas import BaseSchema
from controller.device_management.device_management import device_unregister

from greenhouse_webapp.model.device_manager.device_initialization import Device


router = APIRouter(
    prefix="/interface"
)


@router.post("/data")
async def post_data(data_info: DataSchema) -> JSONResponse:
    """route for devices to post data intermittently, with stable connection

    Args:
        data_info (DataSchema): schema containing all necessary data to make data entry in database for device

    Returns:
        JSONResponse: 200 to successfully respond to device
    """
    router.database_connector.execute('insertData',
        data_info.device_id,
        data_info.project_id,
        data_info.temperature,
        data_info.humidity,
        data_info.moisture,
        data_info.light_exposure,
    )
    return JSONResponse("data inserted successfully", 200)

@router.post("/olddata")
async def post_old_data(data_info: OldDataSchema) -> JSONResponse:
    """in the event that the device loses connection, and has lots of data stored in flash memory, then wants to upload that data after regaining connection, it will be submitted to this route

    Args:
        data_info (OldDataSchema): data schema containing all necessary data for making database entry, including manual specification of datetime

    Returns:
        JSONResponse: 200 for success
    """
    data_collection_time: float = data_info.reference_datetime - data_info.seconds_from_start
    datetime_of_collection: str = datetime.strftime(datetime.fromtimestamp(data_collection_time), "%y-%m-%d %H:%M:%S")
    
    router.database_connector.execute("insertDataAtTime",
                                      device_id=data_info.device_id, 
                                      project_id=data_info.project_id, 
                                      temperature=data_info.temperature,
                                      humidity=data_info.humidity,
                                      moisture=data_info.moisture,
                                      light_exposure=data_info.light_exposure,
                                      datetime_collected=datetime_of_collection)
    
    return JSONResponse("data inserted successfully", 200)

@router.get("/ping")
async def ping() -> None: # when this is received from a device, it should set the device status to idle, since no non idle device should send standard pings
    """Empty route for the sole purpose of idle (paused) devices confirming they are still connected"""
    pass

@router.post("/logs/{device_name}")
async def post_logs(device_name: str, log_info: LogSchema):
    pass