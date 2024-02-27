from typing import Annotated
from fastapi import Depends, FastAPI

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")

from model.data_interface import DatabaseInterface
from model.device_initialization import DeviceManager, Device
from model.data_retrieving import DataRetriever


database_interface: DatabaseInterface = DatabaseInterface()


async def database_dependency() -> DatabaseInterface:
    return database_interface


device_manager = DeviceManager("huehue", database_interface)
data_visualizer = DataRetriever()

