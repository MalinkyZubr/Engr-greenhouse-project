import json
import sys
from contextlib import closing
import mysql.connector
from pydantic import BaseModel

sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from utilities.config_reader import get_config
from model.database.input_sanitization import check_query
from model.database.queries.base_query import DatabaseQuery, METADATA_LIST, METADATA_OBJECT, DATA_SCHEMA
from model.database.queries.data.active_data import *
from model.database.queries.project.active_projects import *
from model.database.queries.project.archived_projects import *
from model.database.queries.device.devices import *
from model.database.queries.preset.presets import *
from model.database.queries.query_schemas_shared import *


class DatabaseInterface:
    def __init__(self):
        db_conf: dict[str | int] = get_config("database")
            
        self.connection_pool = mysql.connector.connect(
            **db_conf,
        )
        
    def execute(self, query: DatabaseQuery, query_parameters: BaseModel)-> list[tuple | None]:
        check_query(query_parameters) # must be updated to take pydantic schemas
        
        with closing(mysql.connector.connect(pool_name="db_pool")) as conn:
            with closing(conn.cursor()) as cursor:
                result = query.full_query(cursor, query_parameters)
                conn.commit()
                
        return result

    def shutdown(self) -> None:
        self.connection_pool.close()
        
        
database_interface: DatabaseInterface = DatabaseInterface()


get_project_id: getProjectID = getProjectID(database_interface)
get_active_projects: getActiveProjects = getActiveProjects(database_interface)
get_project: getProject = getProject(database_interface)
add_project: addProject = addProject(database_interface)
update_project: updateProject = updateProject(database_interface)
archive_project: archiveProject = archiveProject(database_interface)
delete_project: deleteProject = deleteProject(database_interface)
get_project_name: getProjectName = getProjectName(database_interface)
get_project_devices: getProjectDevices = getProjectDevices(database_interface)
get_archive_project_id: getArchiveProjectID = getArchiveProjectID(database_interface)
get_archived_projects: getArchivedProjects = getArchivedProjects(database_interface)
get_archived_project: getArchivedProject = getArchivedProject(database_interface)

get_preset_name: getPresetName = getPresetName(database_interface)
get_preset_id: getPresetID = getPresetID(database_interface)
get_presets: getPresets = getPresets(database_interface)
get_preset: getPreset = getPreset(database_interface)
create_preset: createPreset = createPreset(database_interface)
update_preset: updatePreset = updatePreset(database_interface)
delete_preset: deletePreset = deletePreset(database_interface)
get_preset_associated_devices: getPresetAssociatedDevices = getPresetAssociatedDevices(database_interface)

get_device_id: getDeviceID = getDeviceID(database_interface)
get_device_name: getDeviceName = getDeviceName(database_interface)
get_device_status: getDeviceStatus = getDeviceStatus(database_interface)
get_devices: getDevices = getDevices(database_interface)
get_device: getDevice = getDevice(database_interface)
configure_device: configureDevice = configureDevice(database_interface)
register_device: registerDevice = registerDevice(database_interface)
delete_device: deleteDevice = deleteDevice(database_interface)

get_device_data: getDeviceData = getDeviceData(database_interface)
get_latest_device_data: getLatestDeviceData = getLatestDeviceData(database_interface)
get_device_data_in_range: getDeviceDataInRange = getDeviceDataInRange(database_interface)
get_project_data: getProjectData = getProjectData(database_interface)
get_project_data_in_range: getProjectDataInRange = getProjectDataInRange(database_interface)
insert_data: insertData = insertData(database_interface)
insert_data_at_time: insertDataAtTime = insertDataAtTime(database_interface)
get_data_column_device: getDeviceDataColumn = getDeviceDataColumn(database_interface)
get_data_column_project: getProjectDataColumn = getProjectDataColumn(database_interface)