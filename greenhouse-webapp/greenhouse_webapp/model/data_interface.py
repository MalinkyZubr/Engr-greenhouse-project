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

    def __init__(self, database_connector):
        self.connector = database_connector

    @absractmethod
    def query(self):
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
        return cursor.execute(self.query)


class getActiveProjects(DatabaseQuery):
    query = \
    f"""SELECT * FROM ActiveProjects"""

class getProject(DatabaseQuery):
    query = \ 
    f"""SELECT * FROM ActiveProjects WHERE ProjectName = %s"""

class getArchivedProjects(DatabaseQuery):
    query = \ 
    f"""SELECT * FROM ArchivedProjects"""

class getProjectData(DatabaseQuery):
    query = \
    f"""SELECT * FROM ArchivedProjects WHERE ProjectName = %s"""

class addProject(DatabaseQuery):
    query = \
    f"""INSERT INTO ActiveProjects (ProjectName)

class updateProject(DatabaseQuery):

class archiveProject(DatabaseQuery):

class insertData(DatabaseQuery):

class deleteProject(DatabaseQuery):

class getDevices(DatabaseQuery):

class getDevice(DatabaseQuery):

class configureDevice(DatabaseQuery):

class registerDevice(DatabaseQuery):

class deleteDevice(DatabaseQuery):

class getPresets(DatabaseQuery):

class getPreset(DatabaseQuery):

class updatePreset(DatabaseQuery):

class deletePreset(DatabaseQuery):

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


        