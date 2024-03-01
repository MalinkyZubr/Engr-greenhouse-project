import json
import sys
from contextlib import closing
import mysql.connector
from pydantic import BaseModel

sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from utilities.config_reader import get_config
from model.database.input_sanitization import check_keyword_query, check_query
from model.database.queries.base_query import DatabaseQuery


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
                result = query.execute(cursor, query_parameters)
                conn.commit()
                
        return result

    def shutdown(self) -> None:
        self.connection_pool.close()