from pydantic import BaseModel
from typing import Optional, Literal
from model.database.queries.base_query import MetadataListQuery, MetadataObjectQuery, DataQuery, NoReturnQuery
from model.database.queries.query_schemas_shared import QueryByID, QueryByName


class getDeviceID(MetadataObjectQuery[QueryByName]):
    query_str = \
    f"""SELECT DeviceID FROM RegisteredDevices
    WHERE DeviceName = %(name)s;"""
    
    
class getDeviceName(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT DeviceName FROM RegisteredDevices
    WHERE DeviceID = %(id);"""
    
    
class getDeviceStatus(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT DeviceStatus FROM RegisteredDevices
    WHERE DeviceID = %(id)s;"""


class getDevices(MetadataListQuery[None]):
    query_str = \
    """SELECT * FROM RegisteredDevices;"""
    

class getDevice(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM RegisteredDevices
    WHERE DeviceID = %(id)s;"""


class ConfigureDeviceQuery(QueryByID):
    name: Optional[str]
    preset_id: Optional[int]
    project_id: Optional[int]
    device_status: Optional[bool]
    device_ip: Optional[Literal['ACTIVE', 'IDLE', 'DISCONNECTED']]
    
    
class configureDevice(NoReturnQuery[ConfigureDeviceQuery]):
    query_str = \
    f"""UPDATE RegisteredDevices
    SET DeviceName = COALESCE(%(name)s, DeviceName), PresetID = COALESCE(%(preset_id)s, PresetID), ProjectID = COALESCE(%(project_id)s, ProjectID), DeviceStatus = COALESCE(%(device_status)s, DeviceStatus), DeviceIP = COALESCE(%(device_ip)s, DeviceIP)
    WHERE DeviceID = %(id)s;"""
    

class RegisterDeviceQuery(QueryByName):
    preset_id: int
    project_id: int
    device_status: Literal['ACTIVE', 'IDLE', 'DISCONNECTED']
    

class registerDevice(NoReturnQuery[RegisterDeviceQuery]):
    query_str = \
    f"""INSERT INTO RegisteredDevices (DeviceName, PresetID, ProjectID, DeviceStatus)
    VALUES (%(name)s, %(preset_id)s, %(project_id)s, %(device_status)s);"""


class deleteDevice(NoReturnQuery[QueryByID]):
    query_str = \
    f"""DELETE FROM RegisteredDevices WHERE DeviceID = %(id)s;"""