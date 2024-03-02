from pydantic import BaseModel


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