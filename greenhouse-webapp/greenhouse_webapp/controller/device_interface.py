from fastapi import APIRouter
from fastapi.requests import Request
from fastapi.exceptions import HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field
from typing import Optional, Literal
from datetime import datetime

from controller.DBIntRouter import APIDRouter
from controller.schemas.server_device_schemas import BaseSchema
from controller.device_management import device_unregister

from model.device_initialization import Device

router = APIDRouter(
    prefix="/interface"
)
"""
this file interacts directly with the devices on the network
"""


class DataSchema(BaseModel):
    """schema for holding all data collected by the device to be put into the database

    Args:
        project_id, int: id of the project in question
        device_id, int: id of the device sending the request
        temperature, float: current temperature reading in celsius
        humidity, float: current humidity reading %
        moisture, float: current moisture reading %
        light_exposure, float: current light level reading in lux
    """
    project_id: int # the device should know its own project ID
    device_id: int
    temperature: float
    humidity: float
    moisture: float
    light_exposure: float
    
    
class OldDataSchema(DataSchema):
    """Schema for handling the contingency where connection to the device fails and the device must load data from flash memory upon reconnecting

    Args:
        next, int: address of next entry in flash memory. Here out of convenience, not for use
        previous, int: address of previous entry in flash memory. Here out of convenience, not for use
        seconds_from_start: seconds from the reference datetime
        reference_datetime: unix reference timestamp from which all data timestamps will be derived
    """
    next: Optional[int]
    previous: Optional[int]
    seconds_from_start: int
    reference_datetime: str

    
class LogSchema(BaseModel):
    log_level: int
    log_content: str
    

@router.middleware("http")
async def check_aging(request: Request, next_call): # auto updates the device status
    """Critical middleware which governs device status. Any device that goes a specified period of time without sending a message to the server will be designated 'disconnected'
    This middleware tracks every request made by devices to the server, and resets their disconnect timers accordingly. Each request is embedded with a cookie contianing the device id
    which allows easy tracking in tandem with the DeviceManager in the router. 
    
    Args:
        request (Request): raw request containing all encapsulated data from the device
        next_call (api call): the next api call to be executed should all middleware checks pass

    Returns:
        response, Any: the response to the next_call the device is requesting
    """
    
    reported_device_id: int = int(request.cookies["device_id"])
    reported_device_status: Literal["ACTIVE", "IDLE"] = request.cookies["status"]
    
    router.database_connector.execute("configureDevice", reported_device_id, status=reported_device_status)
        
    active_list_device_entry: Device = router.device_manager.active_device_list[reported_device_id]
        
    active_list_device_entry.update_time() # make sure the timeout doesnt go off...
    
    response = await next_call(request)
    
    return response  
    
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