import os
import sys
import json
import typing

import mysql.connector
from mysql.connector.cursor import MySQLCursorPrepared
from abc import ABC, abstractmethod


script_dir = os.path.dirname(os.path.abspath(__file__))
sql_creation_path = os.path.join(script_dir, "create_db.sql")
sql_config_path = os.path.join(script_dir, "conf/dbconf.json")


class DatabaseQuery(ABC):
    black_list = ["--", ";", "/*", "*/", "@@", "@",
                  "char", "nchar", "varchar", "nvarchar",
                  "alter", "begin", "cast", "create", "cursor",
                  "declare", "delete", "drop", "end", "exec",
                  "execute", "fetch", "insert", "kill", "open",
                   "sys", "sysobjects", "syscolumns",
                  "table", "update"]
    sql_injection_single_characters = [
        "'",  
        "-",  
        "#",  
        ";",  
        "&",  
        "|",  
        "$",  
        "*",  
        "%",  
        "_",  
        "\\"
    ]

    query_str = """"""
        
    @abstractmethod
    def query(self, cursor, *query_parameters, **keyword_parameters):
        """cursor.execute should go in here, along with query parameters"""
        pass

    def check_query(self, query_parameters: list):
        for element in query_parameters:
            if element in self.black_list:
                raise Exception(f"Invalid character {element} detected")
            for character in self.sql_injection_single_characters:
                if character == element:
                    raise Exception(f"Invalid character {character} detected")

    def __call__(self, cursor, *query_parameters: list):
        self.check_query(query_parameters)
        return self.query(cursor, *query_parameters)


class getProjectID(DatabaseQuery):
    query_str = \
    f"""SELECT ProjectID FROM ActiveProjects WHERE ProjectName = %s;"""
    def query(self, cursor, project_name):
        return cursor.execute(self.query_str, (project_name,))


class getArchiveProjectID(DatabaseQuery):
    query_str = \
    f"""SELECT ProjectID FROM ArchivedProjects WHERE ProjectName = %s;"""
    def query(self, cursor, project_name):
        return cursor.execute(self.query_str, (project_name,))


class getActiveProjects(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM ActiveProjects;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)


class getProject(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM ActiveProjects WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))


class getArchivedProjects(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM ArchivedProjects;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)


class getArchivedProject(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM ArchivedProjects
    WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))
    

class getProjectData(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM Data WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))
    
    
class getDeviceData(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM DATA WHERE DeviceID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))


class addProject(DatabaseQuery):
    query_str = \
    f"""INSERT INTO ActiveProjects (ProjectName)
    VALUES (%s);"""
    def query(self, cursor, project_name):
        cursor.execute(f"""SELECT * FROM ArchivedProjects WHERE ProjectName = %s""", (project_name,))
        if cursor.fetchall():
            raise KeyError("The selected name already exists in the archive table")
        return cursor.execute(self.query_str, (project_name,))#{"name":project_name})


class updateProject(DatabaseQuery):
    query_str = \
    f"""UPDATE ActiveProjects
    SET ProjectName = COALESCE(%(ProjectName)s, ProjectName)
    WHERE ProjectID = %(ProjectID)s;"""
    def query(self, cursor, project_id, project_name=None):
        return cursor.execute(self.query_str, {"ProjectName":project_name, "ProjectID":project_id})


class archiveProject(DatabaseQuery):
    query_str = \
    f"""INSERT INTO ArchivedProjects
    SELECT * FROM ActiveProjects
    WHERE ProjectID = %s;
    DELETE FROM ActiveProjects WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id, project_id))


class deleteProject(DatabaseQuery):
    query_str = \
    f"""DELETE FROM ArchivedProjects WHERE ProjectID = %s;
    DELETE FROM Data WHERE ProjectID = %s"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,project_id))


class insertData(DatabaseQuery):
    query_str = \
    f"""INSERT INTO Data (DeviceID, ProjectID, Temperature, Humidity, Moisture, LightExposure, IRExposure, pHLevel)
    VALUES (%s, %s, %s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_id, project_id, temperature, humidity, moisture, light_exposure, ir_exposure, ph_level):
        return cursor.execute(self.query_str, (device_id, project_id, temperature, humidity, moisture, light_exposure, ir_exposure, ph_level))


class getDeviceID(DatabaseQuery):
    query_str = \
    f"""SELECT DeviceID FROM RegisteredDevices
    WHERE DeviceName = %s;"""
    def query(self, cursor, device_name):
        return cursor.execute(self.query_str, (device_name,))


class getDevices(DatabaseQuery):
    query_str = \
    """SELECT * FROM RegisteredDevices;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)


class getProjectDevices(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM RegisteredDevices WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))


class getDevice(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM RegisteredDevices
    WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))


class configureDevice(DatabaseQuery):
    query_str = \
    f"""UPDATE RegisteredDevices
    SET DeviceName = COALESCE(%(DeviceName)s, DeviceName), PresetID = COALESCE(%(PresetID)s, PresetID), ProjectID = COALESCE(%(ProjectID)s, ProjectID), DeviceStatus = COALESCE(%(DeviceStatus)s, DeviceStatus)
    WHERE DeviceID = %(DeviceID)s;"""
    def query(self, cursor, device_id, device_name=None, preset_id=None, project_id=None, status=None):
        return cursor.execute(self.query_str, {"DeviceName":device_name, "ProjectID":project_id, "PresetID":preset_id, "DeviceStatus": status, "DeviceID":device_id})


class registerDevice(DatabaseQuery):
    query_str = \
    f"""INSERT INTO RegisteredDevices (DeviceName, DeviceIP, DeviceMAC, PresetID, ProjectID, DeviceStatus)
    VALUES (%s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_name, device_ip, device_mac, preset_id, project_id, device_status):
        return cursor.execute(self.query_str, (device_name, device_ip, device_mac, preset_id, project_id, device_status))


class deleteDevice(DatabaseQuery):
    query_str = \
    f"""DELETE FROM RegisteredDevices WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))


class getPresetID(DatabaseQuery):
    query_str = \
    f"""SELECT PresetID FROM Presets WHERE PresetName = %s"""
    def query(self, cursor, preset_name):
        return cursor.execute(self.query_str, (preset_name,))


class getPresets(DatabaseQuery):
    query_str = \
    """SELECT * FROM Presets;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)


class getPreset(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM Presets WHERE PresetID = %s"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id,))


class createPreset(DatabaseQuery):
    query_str = \
    f"""INSERT INTO Presets (PresetName, Temperature, Humidity, Moisture, LightExposure, IRExposure)
    VALUES (%s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, preset_name, temperature, humidity, moisture, light_exposure, ir_exposure):
        return cursor.execute(self.query_str, (preset_name, temperature, humidity, moisture, light_exposure, ir_exposure))


class updatePreset(DatabaseQuery):
    query_str = \
    f"""UPDATE Presets
    SET PresetName = COALESCE(%(PresetName)s, PresetName), Temperature = COALESCE(%(Temperature)s, Temperature), Humidity = COALESCE(%(Humidity)s, Humidity), Moisture = COALESCE(%(Moisture)s, Moisture), LightExposure = COALESCE(%(LightExposure)s, LightExposure), IRExposure = COALESCE(%(IRExposure)s, IRExposure)
    WHERE PresetID = %(presetID)s"""
    def query(self, cursor, preset_id, preset_name=None, temperature=None, humidity=None, moisture=None, light_exposure=None, ir_exposure=None):
        return cursor.execute(self.query_str, {"PresetName":preset_name, "Temperature":temperature, "Humidity":humidity, "Moisture":moisture, "LightExposure":light_exposure, "IRExposure":ir_exposure, "presetID":preset_id})


class deletePreset(DatabaseQuery):
    query_str = \
    f"""DELETE FROM Presets WHERE PresetID = %s"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id,))
    
    
class getPresetAssociatedDevices(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM RegisteredDevices WHERE PresetID = %s"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id,))
    

# configure db to only connect via localhost so that no password needed
class DatabaseInterface:
    def __init__(self):
        with open(sql_config_path, 'r') as f:
            config = json.loads(f.read()) # intiialization scripts shoild be run by dockerfile
            
        self.connection_pool = mysql.connector.connect(
            **config,
        )
                
        self.queries = {
            "getActiveProjects":getActiveProjects(),
            "getProjectID":getProjectID(),
            "getArchiveProjectID":getArchiveProjectID(),
            "getProject":getProject(),
            "getArchivedProjects":getArchivedProjects(),
            "getArchivedProject":getArchivedProject(),
            "getProjectData":getProjectData(),
            "getDeviceData":getDeviceData(),
            "addProject":addProject(),
            "updateProject":updateProject(),
            "archiveProject":archiveProject(),
            "deleteProject":deleteProject(),
            "insertData":insertData(),
            "getDeviceID":getDeviceID(),
            "getDevices":getDevices(),
            "getProjectDevices":getProjectDevices(),
            "getDevice":getDevice(),
            "configureDevice":configureDevice(),
            "registerDevice":registerDevice(),
            "deleteDevice":deleteDevice(),
            "getPresetID":getPresetID(),
            "getPresets":getPresets(),
            "getPreset":getPreset(),
            "createPreset":createPreset(),
            "updatePreset":updatePreset(),
            "deletePreset":deletePreset(),
            "getPresetAssociatedDevices":getPresetAssociatedDevices()
        }
        
    def execute(self, query, *query_parameters, **keyword_parameters)-> list[tuple | None]:
        connection = mysql.connector.connect(pool_name="db_pool")
        cursor = connection.cursor(prepared=True)
        self.queries[query](cursor, *query_parameters, **keyword_parameters)
        result = cursor.fetchall()
        connection.commit()
        cursor.close()
        connection.close()
        return result

    def shutdown(self):
        self.connection_pool.close()