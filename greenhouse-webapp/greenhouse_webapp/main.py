from fastapi import Depends, FastAPI
from typing import Annotated

from controller.device_interface import router as device_interface_router
from controller.device_management import router as device_management_router
from controller.presets import router as preset_router
from controller.projects import router as project_router
from model.data_interface import DatabaseInterface

database_connector = DatabaseInterface()

routers = [device_interface_router,
           device_management_router,
           preset_router,
           project_router]

for router in routers:
    router.database_connector = database_connector

app = FastAPI()

app.include_router(device_interface_router)
app.include_router(device_management_router)
app.include_router(preset_router)
app.include_router(project_router)

