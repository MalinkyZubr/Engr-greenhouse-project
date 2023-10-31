from pydantic import BaseModel


class BaseSchema(BaseModel):
    device_name: str
    src_ip: str
    src_mac: str
    
class ServerSchema(BaseSchema):