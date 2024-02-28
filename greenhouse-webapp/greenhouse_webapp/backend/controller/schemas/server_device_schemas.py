from pydantic import BaseModel
from typing import Type, Literal


class BaseSchema(BaseModel):
    command: Literal['unregister', 'configure', 'deactivate', 'activate', 'time_sync'] # time sync should be every 30 minutes
    request_content: Type[BaseModel]