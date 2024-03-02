from typing import Type, Union, TypeVar
from abc import ABC, abstractmethod
from pydantic import BaseModel
import sys


from mysql.connector import cursor
from mysql.connector.cursor import MySQLCursorPrepared
from model.database.database_manager import DatabaseInterface


METADATA_OBJECT = dict[str, str]
METADATA_LIST = list[dict[str, str]]
DATA_SCHEMA = dict[str, list[str | int | float]]


class DatabaseQuery[
        parameter_schema: BaseModel | None, 
        return_schema: Union[
            METADATA_LIST, 
            METADATA_OBJECT, 
            DATA_SCHEMA,
            None
        ]](ABC):
    
    query_str = """"""
    
    def __init__(self, database_interface: DatabaseInterface):
        self.database_interface: DatabaseInterface = database_interface
        
    def submit_query(self, cursor: MySQLCursorPrepared, query_parameters: parameter_schema) -> list[tuple]:
        """cursor.execute should go in here, along with query parameters"""
        if query_parameters:
            field_parameters_dict = query_parameters.model_dump()
            cursor.execute(self.query_str, field_parameters_dict)
        else:
            cursor.execute(self.query_str)
        
        result: list[tuple] = cursor.fetchall()
        
        return result
        
    def get_column_headers(self, cursor: MySQLCursorPrepared) -> tuple[str]:
        return tuple([descriptor[0] for descriptor in cursor.description])
    
    @abstractmethod
    def convert_to_schema(self, unformatted_data: list[tuple], column_headers: tuple[str]) -> return_schema:
        pass

    def full_query(self, cursor: MySQLCursorPrepared, query_parameters: parameter_schema) -> return_schema:
        unformatted_data: list[tuple] = self.query(cursor, query_parameters)
        column_headers: tuple[str] = self.get_column_headers(cursor)
        
        formatted_result: return_schema = self.convert_to_schema(unformatted_data, column_headers)
        return formatted_result
    
    def execute(self, query_parameters: parameter_schema) -> return_schema:
        return self.database_interface.execute(self, query_parameters)
        
        
class MetadataObjectQuery[parameter_schema: BaseModel](DatabaseQuery[parameter_schema, METADATA_OBJECT]):
    def convert_to_schema(self, unformatted_data: list[tuple], column_headers: tuple[str]) -> METADATA_OBJECT:
        if(len(unformatted_data) != 1):
            raise IndexError("Metadata object query received unformatted data of length greater than 1")
        return dict(zip(column_headers, unformatted_data[0]))


class MetadataListQuery[parameter_schema: BaseModel](DatabaseQuery[parameter_schema, METADATA_LIST]):
    def convert_to_schema(self, unformatted_data: list[tuple], column_headers: tuple[str]) -> METADATA_LIST:
        formatted_data: list[dict] = list()
        
        for metadata_row in unformatted_data:
            formatted_data.append(dict(zip(column_headers, metadata_row)))
            
        return formatted_data


class DataQuery[parameter_schema: BaseModel](DatabaseQuery[parameter_schema, DATA_SCHEMA]):
    def invert_indicies(self, unformatted_data: list[tuple]) -> tuple[tuple]:
        return tuple(zip(*unformatted_data))
    
    def convert_to_schema(self, unformatted_data: list[tuple], column_headers: tuple[str]) -> DATA_SCHEMA:
        formatted_data: dict[list] = dict()
        inverted_data: tuple[tuple] = self.invert_indicies(unformatted_data)
        
        for index, column_header in enumerate(column_headers):
            formatted_data.set(column_header, inverted_data[index])
            
        return formatted_data
    

class NoReturnQuery[parameter_schema: BaseModel](DatabaseQuery[parameter_schema, None]):
    def invert_indicies(self, unformatted_data: list[tuple]) -> tuple[tuple]:
        return tuple(zip(*unformatted_data))
    
    def convert_to_schema(self, unformatted_data: list[tuple], column_headers: tuple[str]) -> None:
        return None
        
            