import typing

from fastapi import APIRouter
from greenhouse_webapp.model.data_interface import DatabaseInterface


class APIDRouter(APIRouter):
    database_connector: typing.Optional[DatabaseInterface] = None
    
    def set_db_connector(self, connector: DatabaseInterface):
        self.database_connector = connector