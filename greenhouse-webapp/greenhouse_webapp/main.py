from fastapi import Depends, FastAPI
from typing import Annotated
import asyncio

from controller.device_interface import router as device_interface_router
from controller.device_management import router as device_management_router
from controller.presets import router as preset_router
from controller.projects import router as project_router
from model.data_interface import DatabaseInterface
from model.device_initialization import DeviceManager

loop = asyncio.get_running_loop()

database_connector = DatabaseInterface()
device_manager = DeviceManager()

routers = {'device_interface':device_interface_router,
           'device_manager':device_management_router,
           'presets':preset_router,
           'projects':project_router}

for router in routers.values():
    router.database_connector = database_connector
    
routers['device_manager'] = device_manager

app = FastAPI()

app.include_router(device_interface_router)
app.include_router(device_management_router)
app.include_router(preset_router)
app.include_router(project_router)


manager_task = asyncio.create_task(device_manager)