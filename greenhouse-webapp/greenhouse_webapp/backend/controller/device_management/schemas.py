from pydantic import BaseModel
from typing import Optional, Literal
from datetime import datetime

class DeviceDataSchema(BaseModel):
    """Base schema for holding device data, can be used for presets or the data uplink to the frontend

    Args:
        temperature, float: temperature that the plant likes
        humidity, float: humidity percentage that the plant likes
        moisture, float: moisture level in the soil that the plant likes
    """
    temperature: float
    humidity: float
    moisture: float
    
class DeviceRegistrationPreset(DeviceDataSchema):
    """Preset schema which represents preset data while communicating with the device from the server

    Args:
        hours_daylight, float: how many hours of daylight does the plant need?
        preset_id, int: id of the preset in question for database interactions
    """
    hours_daylight: float
    preset_id: int
    

class DeviceConfigurationSchema(BaseModel):
    """schema representing data necessary to submit a configuration request to the device from the server

    Args:
        project_id, Optional[str]: id of the project associated with the device
        device_name, Optional[str]: the name of the device to register. Might be the IP address
        device_id, Optional[int]: the id of the device that the device stores in flash. If this isnt included, the device was either reset, or is connecting for the first time
    """
    device_name: Optional[str]
    device_id: Optional[int]
    project_id: Optional[str]
    
    
class DeviceRegistrationResponse(DeviceConfigurationSchema):
    """Schema holding the response from the server to the IoT device when the IoT device requests association

    Args:
        inherits all fields from DeviceRegistrationSchema
        reference_time, float: current unix timestamp for the device to sync to
    """
    preset: Optional[DeviceRegistrationPreset]
    reference_time: float
    
class ClientDeviceDataSchema(DeviceDataSchema):
    """data that is delivered to the client frontend periodically for the device webpage. 

    Args:
        uptime, str: the time the device has been active since registration
        light, str: light level in lumens, the user will like something like this
    """
    uptime: str
    light: str
    
class ClientDeviceConfigSchema(BaseModel):
    """This is the schema the client side frontend sends data to the server in when it wants to configure a device

    Args:
        device_name, Optional[str]: name of the device in question
        device_id, Optional[int]: id of the device in question
        device_status, Literal["ACTIVE", "IDLE", "DISCONNECTED"]: enum representing the state of the device. Defaults to DISCONNECTED
        preset_name, Optional[str]: name of preset_name associated with device
        project_name, Optional[str]: name of project associated with device
    """
    device_name: Optional[str]
    device_id: Optional[int]
    device_status: Literal["ACTIVE", "IDLE", "DISCONNECTED"] = "DISCONNECTED"
    preset_name: Optional[str]
    project_name: Optional[str]  

class LogSchema(BaseModel):
    """schema for holding log data. This should probably be in projects......"""
    log_level: int
    log_timestamp: datetime
    log_content: str