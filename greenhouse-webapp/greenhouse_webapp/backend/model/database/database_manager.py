import json
import sys
from contextlib import closing
import mysql.connector
from pydantic import BaseModel

sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.database.input_sanitization import check_query
from model.database.queries.base_query import DatabaseQuery, METADATA_LIST, METADATA_OBJECT, DATA_SCHEMA
from model.database.queries.data.active_data import *
from model.database.queries.project.active_projects import *
from model.database.queries.project.archived_projects import *
from model.database.queries.device.devices import *
from model.database.queries.preset.presets import *
from model.database.queries.query_schemas_shared import *


class DatabaseInterface:
    get_project_id: getProjectID
    get_device_id_by_ip: getDeviceIDByIP
    get_active_projects: getActiveProjects
    get_project: getProject
    add_project: addProject
    update_project: updateProject
    archive_project: archiveProject
    delete_project: deleteProject
    get_project_mame: getProjectName
    get_project_devices: getProjectDevices
    get_archive_project_id: getArchiveProjectID
    get_archived_projects: getArchivedProjects
    get_archived_project: getArchivedProject

    get_preset_name: getPresetName
    get_preset_id: getPresetID
    get_presets: getPresets
    get_preset: getPreset
    create_preset: createPreset
    update_preset: updatePreset
    delete_preset: deletePreset
    get_preset_associated_devices: getPresetAssociatedDevices

    get_device_id: getDeviceID
    get_device_name: getDeviceName
    get_device_status: getDeviceStatus
    get_devices: getDevices
    get_device: getDevice
    configure_device: configureDevice
    register_device: registerDevice
    delete_device: deleteDevice

    get_device_data: getDeviceData
    get_latest_device_data: getLatestDeviceData
    get_device_data_in_range: getDeviceDataInRange
    get_project_data: getProjectData
    get_project_data_in_range: getProjectDataInRange
    insert_data: insertData
    insert_data_at_time: insertDataAtTime
    get_device_data_column: getDeviceDataColumn
    get_project_data_column: getProjectDataColumn
    
    def __init__(self, user: str, host: str, database: str, pool_name: str, password: str, pool_size: int, port: int):
        self.connection_pool = mysql.connector.connect(
            user, host, database, pool_name, password, pool_size, port
        )
        
        self.get_project_id: getProjectID = getProjectID(self)
        self.get_active_projects: getActiveProjects = getActiveProjects(self)
        self.get_project: getProject = getProject(self)
        self.add_project: addProject = addProject(self)
        self.update_project: updateProject = updateProject(self)
        self.archive_project: archiveProject = archiveProject(self)
        self.delete_project: deleteProject = deleteProject(self)
        self.get_project_mame: getProjectName = getProjectName(self)
        self.get_project_devices: getProjectDevices = getProjectDevices(self)
        self.get_archive_project_id: getArchiveProjectID = getArchiveProjectID(self)
        self.get_archived_projects: getArchivedProjects = getArchivedProjects(self)
        self.get_archived_project: getArchivedProject = getArchivedProject(self)

        self.get_preset_name: getPresetName = getPresetName(self)
        self.get_preset_id: getPresetID = getPresetID(self)
        self.get_presets: getPresets = getPresets(self)
        self.get_preset: getPreset = getPreset(self)
        self.create_preset: createPreset = createPreset(self)
        self.update_preset: updatePreset = updatePreset(self)
        self.delete_preset: deletePreset = deletePreset(self)
        self.get_preset_associated_devices: getPresetAssociatedDevices = getPresetAssociatedDevices(self)

        self.get_device_id: getDeviceID = getDeviceID(self)
        self.get_device_id_by_ip: getDeviceIDByIP = getDeviceIDByIP(self)
        self.get_device_name: getDeviceName = getDeviceName(self)
        self.get_device_status: getDeviceStatus = getDeviceStatus(self)
        self.get_devices: getDevices = getDevices(self)
        self.get_device: getDevice = getDevice(self)
        self.configure_device: configureDevice = configureDevice(self)
        self.register_device: registerDevice = registerDevice(self)
        self.delete_device: deleteDevice = deleteDevice(self)

        self.get_device_data: getDeviceData = getDeviceData(self)
        self.get_latest_device_data: getLatestDeviceData = getLatestDeviceData(self)
        self.get_device_data_in_range: getDeviceDataInRange = getDeviceDataInRange(self)
        self.get_project_data: getProjectData = getProjectData(self)
        self.get_project_data_in_range: getProjectDataInRange = getProjectDataInRange(self)
        self.insert_data: insertData = insertData(self)
        self.insert_data_at_time: insertDataAtTime = insertDataAtTime(self)
        self.get_device_data_column: getDeviceDataColumn = getDeviceDataColumn(self)
        self.get_project_data_column: getProjectDataColumn = getProjectDataColumn(self)
        
    def execute(self, query: DatabaseQuery, query_parameters: BaseModel)-> list[tuple | None]:
        check_query(query_parameters) # must be updated to take pydantic schemas
        
        with closing(mysql.connector.connect(pool_name="db_pool")) as conn:
            with closing(conn.cursor()) as cursor:
                result = query.full_query(cursor, query_parameters)
                conn.commit()
                
        return result

    def shutdown(self) -> None:
        self.connection_pool.close()