import typing

from fastapi import APIRouter
from fastapi.templating import Jinja2Templates
from model.data_interface import DatabaseInterface
from model.device_initialization import DeviceManager
from controller.frontend_paths import TEMPLATES


class APIDRouter(APIRouter):
    template_paths: Jinja2Templates = Jinja2Templates(directory=TEMPLATES)
    database_connector: typing.Optional[DatabaseInterface] = None
    device_manager: typing.Optional[DeviceManager] = None
    
    def set_db_connector(self, connector: DatabaseInterface):
        self.database_connector = connector
    
    def set_device_manager(self, device_manager: DeviceManager):
        self.device_manager = device_manager