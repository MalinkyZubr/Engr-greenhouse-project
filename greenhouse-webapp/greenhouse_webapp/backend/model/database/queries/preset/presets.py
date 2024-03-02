from pydantic import BaseModel
from typing import Optional
from model.database.queries.base_query import NoReturnQuery, MetadataListQuery, MetadataObjectQuery
from model.database.queries.query_schemas_shared import QueryByID, QueryByName, QueryNameID


class getPresetName(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT PresetName FROM Presets
    WHERE PresetID = %(id)s;"""
    
    
class getPresetID(MetadataObjectQuery[QueryByName]):
    query_str = \
    f"""SELECT PresetID FROM Presets WHERE PresetName = %(name)s;"""


class getPresets(MetadataListQuery[None]):
    query_str = \
    """SELECT * FROM Presets;"""


class getPreset(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM Presets WHERE PresetID = %(id)s;"""


class NewPresetQuery(QueryByName):
    temperature: float
    humidity: float
    moisture: float
    light_exposure: float
    
    
class createPreset(NoReturnQuery[NewPresetQuery]):
    query_str = \
    f"""INSERT INTO Presets (PresetName, Temperature, Humidity, Moisture, LightExposure)
    VALUES (%(name)s, %(temperature)s, %(humidity)s, %(moisture)s, %(light_exposure)s);"""


class UpdatePresetQuery(QueryByID):
    name: Optional[str]
    temperature: Optional[float]
    humidity: Optional[float]
    moisture: Optional[float]
    light_exposure: Optional[float]
    

class updatePreset(NoReturnQuery[UpdatePresetQuery]):
    query_str = \
    f"""UPDATE Presets
    SET PresetName = COALESCE(%(name)s, PresetName), Temperature = COALESCE(%(temperature)s, Temperature), Humidity = COALESCE(%(humidity)s, Humidity), Moisture = COALESCE(%(moisture)s, Moisture), LightExposure = COALESCE(%(light_exposure)s, LightExposure))
    WHERE PresetID = %(id)s;"""


class deletePreset(NoReturnQuery[QueryByID]):
    query_str = \
    f"""DELETE FROM Presets WHERE PresetID = %(id)s;
    UPDATE RegisteredDevices
    SET PresetID = -1
    WHERE PresetID = %(id)s;"""
    
    
class getPresetAssociatedDevices(MetadataListQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM RegisteredDevices WHERE PresetID = %(id)s;"""
    