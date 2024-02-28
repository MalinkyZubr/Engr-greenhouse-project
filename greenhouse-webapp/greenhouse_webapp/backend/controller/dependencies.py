from typing import Annotated
from fastapi import Depends, FastAPI

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")

from greenhouse_webapp.model.database.data_interface import DatabaseInterface
from greenhouse_webapp.model.device_manager.device_initialization import DeviceManager, Device
from greenhouse_webapp.model.data_retrieval.data_retrieving import DataRetriever

from utilities.config_reader import get_config



database_interface: DatabaseInterface = DatabaseInterface()


async def database_dependency() -> DatabaseInterface:
    return database_interface


device_manager = DeviceManager("huehue", database_interface)
data_visualizer = DataRetriever()

