from typing import Annotated
from fastapi import Depends, FastAPI

from model.database.database_manager import DatabaseInterface
from model.task_manager.task_manager import TaskManager, Task
from model.devices.device_manager import DeviceContainer, CheckDeadScans, ScanListener, ScanUDPSocket


from utilities.config_reader import get_config



database_interface: DatabaseInterface = DatabaseInterface(**get_config("database"))

device_manager: TaskManager[DeviceContainer] = TaskManager[DeviceContainer](dict(), "device_manager") \
    .attach_task(CheckDeadScans()) \
    .attach_task(ScanListener()) \
    .attach_task(ScanUDPSocket())# make sure to integrate the logger AND exception handlers


async def database_dependency() -> DatabaseInterface:
    return database_interface


async def device_manager_dependency() -> TaskManager[DeviceContainer]:
    return device_manager