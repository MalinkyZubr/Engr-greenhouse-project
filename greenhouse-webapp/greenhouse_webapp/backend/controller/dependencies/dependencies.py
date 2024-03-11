from typing import Annotated
from fastapi import Depends, FastAPI

from model.database.database_manager import DatabaseInterface


from utilities.config_reader import get_config



database_interface: DatabaseInterface = DatabaseInterface(**get_config("database"))


async def database_dependency() -> DatabaseInterface:
    return database_interface







