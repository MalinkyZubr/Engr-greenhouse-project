from pydantic import BaseModel


class ConnectRequestSchema(BaseModel):
    ip: str
    name: str
    id: str
    expidited: bool
    
    
class RegistrationSchema(BaseModel):
    server_ip: str
    server_port: int