from pydantic import BaseModel


class ClientSchema(BaseModel):
    device_name: str
    src_ip: str
    src_mac: str
    
    
class RegistrationSchema(BaseModel):
    server_ip: str
    server_port: int
    