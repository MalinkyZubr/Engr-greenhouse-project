import typing

from fastapi import APIRouter
from greenhouse_webapp.model.data_interface import DatabaseInterface
from greenhouse_webapp.model.device_initialization import DeviceManager


class APIDRouter(APIRouter):
    database_connector: typing.Optional[DatabaseInterface] = None
    device_manager: typing.Optional[DeviceManager] = None
    
    def set_db_connector(self, connector: DatabaseInterface):
        self.database_connector = connector
    
    def set_device_manager(self, device_manager: DeviceManager):
        self.device_manager = device_manager