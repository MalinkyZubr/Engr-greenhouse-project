import os
import sys

import mysql.connector
from mysql.connector.cursor import MySQLCursorPrepared
from abc import ABC, abstractmethod


script_dir = os.path.dirname(os.path.abspath(__file__))
sql_creation_path = os.path.join(script_dir, "create_db.sql")


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

    query = """"""

    @absractmethod
    def query(self, cursor):
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
    query = \
    f"""SELECT ProjectID FROM ActiveProjects WHERE ProjectName = %s;"""
    def query(self, cursor, project_name):
        return cursor.execute(self.query, {"ProjectName":project_name})


class getArchiveProjectID(DatabaseQuery):
    query = \
    f"""SELECT ProjectID FROM ArchivedProjects WHERE ProjectName = %s;"""
    def query(self, cursor, project_name):
        return cursor.execute(self.query, {"ProjectName":project_name})


class getActiveProjects(DatabaseQuery):
    query = \
    f"""SELECT * FROM ActiveProjects;"""
    def query(self, cursor):
        return cursor.execute(self.query)


class getProject(DatabaseQuery):
    query = \ 
    f"""SELECT * FROM ActiveProjects WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, {"ProjectID":project_id})


class getArchivedProjects(DatabaseQuery):
    query = \ 
    f"""SELECT * FROM ArchivedProjects;"""
    def query(self, cursor):
        return cursor.execute(self.query)


class getArchivedProject(DatabaseQuery):
    query = \ 
    f"""SELECT * FROM ArchivedProjects
    WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, {"ProjectID":project_id})
    

class getProjectData(DatabaseQuery):
    query = \
    f"""SELECT * FROM DATA WHERE ProjectID = %s"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, {"ProjectID":project_id})


class getDeviceData(DatabaseQuery):
    query = \
    f"""SELECT * FROM DATA WHERE DeviceID = %s"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, {"DeviceID":project_id})


class addProject(DatabaseQuery):
    query = \
    f"""@name = %s;
    INSERT INTO ActiveProjects (ProjectName)
    VALUES (@name) WHERE NOT EXISTS (SELECT 1 FROM ArchivedProjects WHERE ProjectName = @name);"""
    def query(self, cursor, project_name):
        return cursor.execute(self.query, {"name":project_name})


class updateProject(DatabaseQuery):
    query = \
    f"""UPDATE ActiveProjects
    SET ProjectName = %s
    WHERE ProjectID = %s;"""
    def query(self, cursor, project_name, project_id):
        return cursor.execute(self.query, {"ProjectName":project_name, "ProjectID":project_id})


class archiveProject(DatabaseQuery):
    query = \
    f"""SET @id = %s
SET @requested_name = (SELECT ProjectName FROM ActiveProjects WHERE ProjectID = @id);
SET @date = (SELECT DateStarted FROM ActiveProjects WHERE ProjectID = @id);
IF EXISTS (SELECT 1 FROM ArchivedProjects WHERE ProjectName = @requested_name) THEN
    SET @new_name = @desired_name;
    SET @increment = 0;
    WHILE EXISTS (SELECT 1 FROM ArchivedProjects WHERE ProjectName = @new_name) DO
        SET @new_name = CONCAT(@new_name, @increment);
        @increment = @increment + 1;
    END WHILE;
INSERT INTO ArchivedProjects (ProjectID, ProjectName, DateStarted)
VALUES (@id, @requested_name, @date);
DELETE FROM ActiveProjects WHERE ProjectID = @id;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, (project_id,))


class deleteProject(DatabaseQuery):
    query = \
    f"""DELETE FROM ArchivedProjects WHERE ProjectID = %s"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, {"ProjectID":project_id})


class insertData(DatabaseQuery):
    query = \ 
    f"""INSERT INTO Data (DeviceID, ProjectID, Temperature, Humidity, Moisture, LightExposure, IRExposure, pHLevel)
    VALUES (%s, %s, %s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_id, project_id, temperature, humidity, moisture, light_exposure, ir_exposure, ph_level):
        return cursor.execute(self.query, (device_id, project_id, temperature, humidity, moisturem light_exposure, ir_exposure, ph_level))


class getDeviceID(DatabaseQuery):
    query = \
    f"""SELECT DeviceID FROM RegisteredDevices
    WHERE DeviceName = %s;"""
    def query(self, cursor, device_name):
        return cursor.execute(self.query, {"DeviceName": device_name})


class getDevices(DatabaseQuery):
    query = \
    """SELECT * FROM RegisteredDevices;"""
    def query(self, cursor):
        return cursor.execute(self.query)


class getProjectDevices(DatabaseQuery):
    query = \
    f"""SELECT * FROM RegisteredDevices WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query, {"ProjectID":project_id})


class getDevice(DatabaseQuery):
    query = \ 
    f"""SELECT * FROM RegisteredDevices
    WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query, {"DeviceID":device_id})


class configureDevice(DatabaseQuery):
    query = \
    f"""UPDATE RegisteredDevices
    SET DeviceName = %s, PresetID = %s, ProjectID = %s, DeviceStatus = %s
    WHERE DeviceID = %s;"""
    def query(self, cursor, device_name, preset_id, project_id, status, device_id):
        return cursor.execute(self.query, {"DeviceName":device_name, "ProjectID":project_id, "PresetID":preset_it, "DeviceStatus": status, "DeviceID":device_id})


class registerDevice(DatabaseQuery):
    query = \
    f"""INSERT INTO RegisteredDevices (DeviceName, DeviceIP, DeviceMAC, PresetID, ProjectID, DeviceStatus)
    VALUES (%s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_name, device_ip, device_map, preset_id, project_id, device_status):
        return cursor.execute(self.query, (device_name, device_ip, device_mac, preset_id, project_id, device_status))


class deleteDevice(DatabaseQuery):
    query = \
    f"""DELETE FROM RegisteredDevices WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query, {"DeviceID":device_id})


class getPresetID(DatabaseQuery):
    query = \
    f"""SELECT PresetID FROM Presets WHERE PresetName = %s"""
    def query(self, cursor, preset_name):
        return cursor.execute(self.query, {"PresetName":preset_name})


class getPresets(DatabaseQuery):
    query = \
    """SELECT * FROM Presets;"""
    def query(self, cursor):
        return cursor.execute(self.query)


class getPreset(DatabaseQuery):
    query = \
    f"""SELECT * FROM Presets WHERE PresetID = %s"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query, {"PresetID":preset_id})


class createPreset(DatabaseQuery):
    query = \
    f"""INSERT INTO Presets (PresetName, Temperature, Humidity, Moisture, LightExposure, IRExposure)
    VALUES (%s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, preset_name, temperature, humidity, moisture, light_exposure, ir_exposure):
        return cursor.execute(self.query, (preset_name, temperature, humdidity, moisture, light_exposure, ir_exposure))


class updatePreset(DatabaseQuery):
    query = \
    f"""UPDATE Presets
    SET PresetName = %s, Temperature = %s, Humidity = %s, Moisture = %s, LightExposure = %s, IRExposure = %s
    WHERE PresetID = %s"""
    def query(self, cursor, preset_name, temperature, humidity, moisture, light_exposure, ir_exposure, preset_id):
        return cursor.execute(self.query, {"PresetName":preset_name, "Temperature":temperature, "Humidity":humidity, "Moisture":moisture, "LightExposure":light_exposure, "IRExposure":ir_exposure})


class deletePreset(DatabaseQuery):
    query = \
    f"""DELETE FROM Presets WHERE PresetID = %s"""
    def query(self, cursort, preset_id):
        return cursor.execute(self.query, {"PresetID":preset_id})


# configure db to only connect via localhost so that no password needed
class DatabaseInterface:
    def __init__(self, user, password, host, database):
        self.connector = mysql.connector.connect(
            user="greenhouse_api",
            host="127.0.0.1",
            database="greenhouse_db"
        )
        self.cursor = self.connector.cursor(cursor_class=MySQLCursorPrepared)

        with open(sql_creation_path, 'r') as f:
            sql_creation_script = f.read()
            self.cursor.execute(sql_creation_script)

        self.queries = {
            "getActiveProjects":
        }


        