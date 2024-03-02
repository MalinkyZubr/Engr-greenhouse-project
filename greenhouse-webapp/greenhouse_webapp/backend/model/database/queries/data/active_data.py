from pydantic import BaseModel
from model.database.queries.base_query import MetadataListQuery, MetadataObjectQuery, DataQuery, NoReturnQuery
from model.database.queries.query_schemas_shared import QueryByID, QueryByName


class getDeviceData(DataQuery[QueryByID]): # maybe you shouldnt call this mASSIVE QUerY!!!!
    query_str = \
    f"""SELECT * FROM DATA WHERE DeviceID = %(id)s;"""
    
    
class getLatestDeviceData(DataQuery[QueryByID]):
    query = \
    f"""SELECT * FROM Data WHERE DeviceID = %(id)s ORDER BY DateCollected DESC limit 1;"""
    

class DataDateQuery(QueryByID):
    start_date: str
    end_date: str
    
    
class getDeviceDataInRange(DataQuery[DataDateQuery]):
    query_str = \
    f"""SELECT * 
    FROM Data
    WHERE DateCollected 
            BETWEEN  str_to_date(%(start_date)s, '%Y-%m-%d')
                AND   str_to_date(%(end_date)s, '%Y-%m-%d')
        AND DeviceID = %(id)s;"""
    

class getProjectData(DataQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM Data WHERE ProjectID = %(id)s;"""
    
    
class getProjectDataInRange(DataQuery[DataDateQuery]):
    query_str = \
    f"""SELECT * 
    FROM Data
    WHERE DateCollected 
            BETWEEN  str_to_date(%(start_date)s, '%Y-%m-%d')
                AND   str_to_date(%(end_date)s, '%Y-%m-%d')
        AND ProjectID = %(id)s;"""
    
    
class DataInsertQuery(BaseModel):
    device_id: str
    project_id: str
    temperature: float
    humidity: float
    moisture: float
    light_exposure: float
    
    
class insertData(NoReturnQuery[DataInsertQuery]):
    query_str = \
    f"""INSERT INTO Data (DeviceID, ProjectID, Temperature, Humidity, Moisture, LightExposure)
    VALUES (%(device_id)s, %(project_id)s, %(temperature)s, %(humidity)s, %(moisture)s, %(light_exposure)s);"""
    
    
class DataInsertDateQuery(DataInsertQuery):
    date: str
    
    
class insertDataAtTime(NoReturnQuery[DataInsertDateQuery]):
    query_str = \
    f"""INSERT INTO Data (DeviceID, ProjectID, DateCollected, Temperature, Humidity, Moisture, LightExposure)
    VALUES (%(device_id)s, %(project_id)s, str_to_date(%(date)s, '%Y-%m-%d %H:%i'), %(temperature)s, %(humidity)s, %(moisture)s, %(light_exposure)s);"""