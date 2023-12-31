import typing

from fastapi import APIRouter
from fastapi.templating import Jinja2Templates
from model.data_interface import DatabaseInterface
from model.device_initialization import DeviceManager
from model.data_retrieving import DataRetriever
from controller.frontend_paths import TEMPLATES


class APIDRouter(APIRouter):
    """custom router that can support specialized functions more intuitively (to me)

    Args:
        template_paths, Jinja2Templates: template manager object to support template responses
        database_connector, Optional[DatabaseInterface]: connector object to make queries to database through API calls
        device_manager, Optional[DeviceManager]: low level socket based management system for governing device association, timing, and aging of devices
        data_retriever, DataRetriever: utility for fetching data visualizations and CSV files
    """
    template_paths: Jinja2Templates = Jinja2Templates(directory=TEMPLATES)
    database_connector: typing.Optional[DatabaseInterface] = None
    device_manager: typing.Optional[DeviceManager] = None
    data_retriever: DataRetriever = None
    
    def set_db_connector(self, connector: DatabaseInterface):
        self.database_connector = connector
    
    def set_device_manager(self, device_manager: DeviceManager):
        self.device_manager = device_manager
        
    def set_data_retriever(self, data_retriever: DataRetriever):
        self.data_retriever = data_retriever