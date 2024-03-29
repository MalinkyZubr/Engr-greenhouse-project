from pydantic import BaseModel


class QueryByName(BaseModel):
    name: str
    

class QueryByID(BaseModel):
    id: int
    
    
class QueryNameID(QueryByID, QueryByName):
    pass